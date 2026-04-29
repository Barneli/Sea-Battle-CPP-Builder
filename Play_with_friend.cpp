#include <vcl.h>
#pragma hdrstop
#include "Play_with_friend.h"
#include "Arrangment_ship.h"
#include "New_game.h"
#include "dmNetwork.h"
#include <algorithm>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm6 *Form6;
static const TColor GRID_COLOR = (TColor)0x6B4900;

//---------------------------------------------------------------------------
// Малює спрайт з ShipImages (море, корабель) — перший шар, прозорість не потрібна
static void DrawSprite(TImageList* il, TCanvas* canvas,
                       const TRect& rect, int idx)
{
    TBitmap* bmp = new TBitmap();
    try {
        bmp->PixelFormat = pf32bit;
        bmp->Width       = il->Width;
        bmp->Height      = il->Height;
        bmp->Canvas->Brush->Color = clBlack;
        bmp->Canvas->FillRect(TRect(0, 0, bmp->Width, bmp->Height));
        il->Draw(bmp->Canvas, 0, 0, idx, true);
        canvas->StretchDraw(rect, bmp);
    } __finally { delete bmp; }
}

//---------------------------------------------------------------------------
// Накладає TBitmap прозоро поверх вже намальованого шару через WinAPI TransparentBlt.
// Прозорий колір беремо з пікселя [0,0] оригінального BMP.
static void DrawBmpTransparent(TCanvas* destCanvas, const TRect& destRect,
                                TBitmap* src)
{
    if (!src || src->Width == 0 || src->Height == 0) return;

    TBitmap* scaled = new TBitmap();
    try {
        scaled->PixelFormat = pf24bit;
        scaled->Width  = destRect.Width();
        scaled->Height = destRect.Height();
        scaled->Canvas->StretchDraw(
            TRect(0, 0, scaled->Width, scaled->Height), src);

        TColor transpColor = src->Canvas->Pixels[0][0];

        ::TransparentBlt(
            destCanvas->Handle,
            destRect.Left, destRect.Top,
            destRect.Width(), destRect.Height(),
            scaled->Canvas->Handle,
            0, 0,
            scaled->Width, scaled->Height,
            (UINT)transpColor
        );
    } __finally { delete scaled; }
}

//---------------------------------------------------------------------------
__fastcall TForm6::TForm6(TComponent* Owner) : TForm(Owner)
{
    you_count = 0;
    op_count  = 0;
    game_over = false;
    my_turn   = false;
    hover_c   = -1;
    hover_r   = -1;
    Btn_return->Caption     = "ЗДАТИСЬ";
    Btn_return->Font->Color = (TColor)RGB(0, 240, 255);
}

//---------------------------------------------------------------------------
// Шар 1: текстура моря
void TForm6::DrawSeaCell(TCanvas* Canvas, const TRect& Rect)
{
    DrawSprite(Form2->ShipImages, Canvas, Rect, 20);
}

// Шар 2: корабель прозоро поверх моря
void TForm6::DrawShipSprite(TCanvas* cvs, const TRect& Rect, int spriteIndex)
{
    if (spriteIndex < 0 || spriteIndex >= Form2->ShipImages->Count) return;
	TBitmap *bitmap = new TBitmap(); // Створюємо об'єкт у пам'яті
	try {
		Form2->ShipImages->GetBitmap(spriteIndex, bitmap);
		bitmap->Transparent = true;
		// Встановлюємо колір прозорості (зазвичай лівий нижній або верхній кут)
		bitmap->TransparentColor = bitmap->Canvas->Pixels[0][0];
		cvs->StretchDraw(Rect, bitmap); // Малюємо
	}
	__finally {
		delete bitmap; // Обов'язково видаляємо, щоб не було витоку пам'яті
	}
}

// Шар 3: промах — BmpBamSea прозоро поверх моря
void TForm6::DrawMiss(TCanvas* cvs, const TRect& Rect)
{
    if (Form2->BmpBamSea && Form2->BmpBamSea->Width > 0)
        DrawBmpTransparent(cvs, Rect, Form2->BmpBamSea);
    else {
        int cx = (Rect.Left + Rect.Right)  / 2;
        int cy = (Rect.Top  + Rect.Bottom) / 2;
        int r  = 4;
        cvs->Brush->Color = clWhite;
        cvs->Pen->Color   = clSilver;
        cvs->Ellipse(cx-r, cy-r, cx+r, cy+r);
    }
}

// Шар 3: влучили — BmpBamShip прозоро поверх корабля
void TForm6::DrawHit(TCanvas* cvs, const TRect& Rect)
{
    if (Form2->BmpBamShip && Form2->BmpBamShip->Width > 0)
        DrawBmpTransparent(cvs, Rect, Form2->BmpBamShip);
    else {
        int margin = std::max(3, std::min(Rect.Width(), Rect.Height()) / 5);
        cvs->Pen->Color = clRed;
        cvs->Pen->Width = 2;
        cvs->MoveTo(Rect.Left  + margin, Rect.Top    + margin);
        cvs->LineTo(Rect.Right - margin, Rect.Bottom - margin);
        cvs->MoveTo(Rect.Right - margin, Rect.Top    + margin);
        cvs->LineTo(Rect.Left  + margin, Rect.Bottom - margin);
        cvs->Pen->Width = 1;
    }
}

// Шар 3: потоплено — KillShip прозоро поверх корабля
void TForm6::DrawKill(TCanvas* cvs, const TRect& Rect)
{
    if (Form2->KillShip && Form2->KillShip->Width > 0)
        DrawBmpTransparent(cvs, Rect, Form2->KillShip);
    else {
        int margin = std::max(3, std::min(Rect.Width(), Rect.Height()) / 5);
        cvs->Pen->Color = (TColor)RGB(180, 0, 0);
        cvs->Pen->Width = 3;
        cvs->MoveTo(Rect.Left  + margin, Rect.Top    + margin);
        cvs->LineTo(Rect.Right - margin, Rect.Bottom - margin);
        cvs->MoveTo(Rect.Right - margin, Rect.Top    + margin);
        cvs->LineTo(Rect.Left  + margin, Rect.Bottom - margin);
        cvs->Pen->Width = 1;
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm6::FormShow(TObject *Sender)
{
    you_count = 0;
    op_count  = 0;
    game_over = false;
    hover_c   = -1;
    hover_r   = -1;
    my_turn   = DataModule1->IsHost;
    Form2->player1->reset_hunt();
    Label_your_count->Caption = "Ви зробили 0 пострілів";
    Label_op_count->Caption   = "Суперник зробив 0 пострілів";
    Label_who->Caption        = my_turn ? "Ваш хід" : "Хід суперника...";
    Attack_field->Enabled     = my_turn;
    Attack_field->Invalidate();
    Arrangement_field->Invalidate();
}

void __fastcall TForm6::FormClose(TObject *Sender, TCloseAction &Action)
{
    Application->Terminate();
}

void __fastcall TForm6::Btn_returnClick(TObject *Sender)
{
    game_over = true;
    Form2->Show();
    this->Hide();
}

//---------------------------------------------------------------------------
// СВОЄ ПОЛЕ
// Шари: 1)море → 2)спрайт корабля → 3)BmpBamShip або KillShip (прозоро)
void __fastcall TForm6::Arrangement_fieldDrawCell(TObject *Sender,
    System::LongInt ACol, System::LongInt ARow,
    TRect &Rect, TGridDrawState State)
{
    int      val = Form2->player1->my_field[ACol][ARow];
    TCanvas* cvs = Arrangement_field->Canvas;

    // Шар 1: море
    DrawSeaCell(cvs, Rect);

    if (val > 0 && val < 100) {
        // Шар 2: живий корабель прозоро
        DrawShipSprite(cvs, Rect, val - 1);

    } else if (val < 0 && val > -100) {
        // Шар 2: спрайт підбитого корабля прозоро
        DrawShipSprite(cvs, Rect, abs(val) - 1);
        // Шар 3: ефект прозоро поверх корабля
        if (Form2->player1->is_destroyed(ACol, ARow, Form2->player1->my_field))
            DrawKill(cvs, Rect);
        else
            DrawHit(cvs, Rect);

    } else if (val == 100) {
        // Шар 2: промах прозоро поверх моря
        DrawMiss(cvs, Rect);
    }

    cvs->Pen->Color = GRID_COLOR;
    cvs->Pen->Width = 1;
    cvs->FrameRect(Rect);
}

//---------------------------------------------------------------------------
// ПОЛЕ АТАКИ
// Шари: 1)море → 2)спрайт (якщо потоплено) → 3)ефект (прозоро)
void __fastcall TForm6::Attack_fieldDrawCell(TObject *Sender,
    System::LongInt ACol, System::LongInt ARow,
    TRect &Rect, TGridDrawState State)
{
    int      val = Form2->player1->attacking_field[ACol][ARow];
    TCanvas* cvs = Attack_field->Canvas;

    // Шар 1: море
    DrawSeaCell(cvs, Rect);

    if (val == 100) {
        // Промах: шар 2 — BmpBamSea прозоро
        DrawMiss(cvs, Rect);

    } else if (val == -1) {
        // Влучили (не потоплено): шар 2 — BmpBamShip прозоро
        DrawHit(cvs, Rect);

    } else if (val > 0 && val < 100) {
        // Потоплено: шар 2 — спрайт, шар 3 — KillShip прозоро
        DrawShipSprite(cvs, Rect, val - 1);
        DrawKill(cvs, Rect);
    }

    // Hover: червона рішітка
    if (ACol == hover_c && ARow == hover_r &&
        !game_over && my_turn && Attack_field->Enabled && val == 0)
    {
        cvs->Pen->Color = clRed;
        cvs->Pen->Width = 1;
        const int STEP = 4;
        for (int y = Rect.Top; y <= Rect.Bottom; y += STEP) {
            cvs->MoveTo(Rect.Left,  y);
            cvs->LineTo(Rect.Right, y);
        }
        for (int x = Rect.Left; x <= Rect.Right; x += STEP) {
            cvs->MoveTo(x, Rect.Top);
            cvs->LineTo(x, Rect.Bottom);
        }
    }

    cvs->Pen->Color = GRID_COLOR;
    cvs->Pen->Width = 1;
    cvs->FrameRect(Rect);
}

//---------------------------------------------------------------------------
void __fastcall TForm6::Attack_fieldMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y)
{
    int c, r;
    Attack_field->MouseToCell(X, Y, c, r);
    int nc = (c >= 0 && c < 10 && r >= 0 && r < 10) ? c : -1;
    int nr = (c >= 0 && c < 10 && r >= 0 && r < 10) ? r : -1;
    if (nc != hover_c || nr != hover_r) {
        hover_c = nc;
        hover_r = nr;
        Attack_field->Invalidate();
    }
}

void __fastcall TForm6::Attack_fieldMouseLeave(TObject *Sender)
{
    if (hover_c != -1 || hover_r != -1) {
        hover_c = -1;
        hover_r = -1;
        Attack_field->Invalidate();
    }
}

//---------------------------------------------------------------------------
void __fastcall TForm6::Attack_fieldSelectCell(TObject *Sender,
    System::LongInt ACol, System::LongInt ARow, bool &CanSelect)
{
    if (game_over || !my_turn || !Attack_field->Enabled) return;
    if (Form2->player1->attacking_field[ACol][ARow] != 0) return;
    Attack_field->Enabled = false;
    my_turn = false;
    Label_who->Caption = "Чекаємо відповіді...";
    Label_your_count->Caption =
        "Ви зробили " + IntToStr(++you_count) + " пострілів";
    DataModule1->SendToOpponent("SHOT:" + IntToStr(ACol) + ":" + IntToStr(ARow));
}

//---------------------------------------------------------------------------
void TForm6::OnOpponentShot(int x, int y)
{
    if (game_over) return;
    int result = Form2->player1->make_shot(x, y, Form2->player1->my_field);
    String msg = "RESULT:" + IntToStr(x) + ":" + IntToStr(y) + ":"
                           + IntToStr(result);
    if (result == 2) {
        std::vector<ShipCell> ship;
        ShipCell first;
        first.x = x; first.y = y;
        first.value = abs(Form2->player1->my_field[x][y]);
        ship.push_back(first);
        int head = 0;
        while (head < (int)ship.size()) {
            int cx = ship[head].x;
            int cy = ship[head].y;
            head++;
            int dx[] = {0, 0, 1, -1};
            int dy[] = {1, -1, 0, 0};
            for (int d = 0; d < 4; d++) {
                int nx = cx + dx[d];
                int ny = cy + dy[d];
                if (nx < 0 || nx >= 10 || ny < 0 || ny >= 10) continue;
                int v = Form2->player1->my_field[nx][ny];
                if (v >= 0 || v <= -100) continue;
                bool found = false;
                for (int k = 0; k < (int)ship.size(); k++)
                    if (ship[k].x == nx && ship[k].y == ny) { found = true; break; }
                if (!found) {
                    ShipCell sc;
                    sc.x = nx; sc.y = ny; sc.value = abs(v);
                    ship.push_back(sc);
                }
            }
        }
        for (int i = 0; i < (int)ship.size(); i++)
            msg += ":" + IntToStr(ship[i].x) + ":" + IntToStr(ship[i].y)
                       + ":" + IntToStr(ship[i].value);
    }
    DataModule1->SendToOpponent(msg);
    op_count++;
    Label_op_count->Caption =
        "Суперник зробив " + IntToStr(op_count) + " пострілів";
    Arrangement_field->Invalidate();
    if (!Form2->player1->has_ships(Form2->player1->my_field)) {
        game_over           = true;
        Label_who->Caption  = "Поразка!";
        Btn_return->Caption = "РЕВАНШ";
        ShowMessage("Суперник потопив усі ваші кораблі!");
        return;
    }
    if (result > 0) {
        Label_who->Caption    = "Хід суперника...";
        Attack_field->Enabled = false;
    } else {
        my_turn               = true;
        Label_who->Caption    = "Ваш хід";
        Attack_field->Enabled = true;
    }
}

//---------------------------------------------------------------------------
void TForm6::ApplyShotResult(int x, int y, int result,
    const std::vector<ShipCell>& shipCells,
    const std::vector<std::pair<int,int> >& aroundCells)
{
    if (game_over) return;
    if (result == 0) {
        Form2->player1->attacking_field[x][y] = 100;
    } else if (result == 1) {
        Form2->player1->attacking_field[x][y] = -1;
    } else if (result == 2) {
        for (int i = 0; i < (int)shipCells.size(); i++)
            Form2->player1->attacking_field[shipCells[i].x][shipCells[i].y] =
                shipCells[i].value;
        for (int i = 0; i < (int)aroundCells.size(); i++) {
            int ax = aroundCells[i].first;
            int ay = aroundCells[i].second;
            if (ax < 0 || ax >= 10 || ay < 0 || ay >= 10) continue;
            if (Form2->player1->attacking_field[ax][ay] == 0)
                Form2->player1->attacking_field[ax][ay] = 100;
        }
    }
    Attack_field->Invalidate();
    if (result == 2) {
        bool allSunk = true;
        for (int i = 0; i < 10 && allSunk; i++)
            for (int j = 0; j < 10 && allSunk; j++)
                if (Form2->player1->attacking_field[i][j] == 0)
                    allSunk = false;
        if (allSunk) {
            game_over           = true;
            Label_who->Caption  = "Перемога!";
            Btn_return->Caption = "ПОВТОРИТИ";
            ShowMessage("Вітаємо! Ви потопили всі кораблі суперника!");
            return;
        }
    }
    if (result > 0) {
        my_turn               = true;
        Label_who->Caption    = "Влучили! Стріляйте ще";
        Attack_field->Enabled = true;
    } else {
        my_turn               = false;
        Label_who->Caption    = "Хід суперника...";
        Attack_field->Enabled = false;
    }
}

//---------------------------------------------------------------------------
void TForm6::OnOpponentDisconnected()
{
    if (!game_over) {
        game_over = true;
        ShowMessage("Суперник відключився від гри.");
        Attack_field->Enabled = false;
        Form3->Show();
        this->Close();
    }
}

void __fastcall TForm6::CommonMouseEnter(TObject *Sender)
{
    TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
    if (btn) btn->Font->Color = (TColor)RGB(0, 0, 200);
}
void __fastcall TForm6::CommonMouseLeave(TObject *Sender)
{
    TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
    if (btn) btn->Font->Color = (TColor)RGB(0, 240, 255);
}
