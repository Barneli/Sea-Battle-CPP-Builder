#include <vcl.h>
#pragma hdrstop
#include "Arrangment_ship.h"
#include "Play_with_bot.h"
#include "Play_with_friend.h"
#include "dmNetwork.h"
#include "New_game.h"
//#include "Lobby.h"
#include <winuser.h>   // WM_RBUTTONDOWN
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm2 *Form2;
// ─── Кольори ────────────────────────────────────────────────────────────────
static const TColor GRID_COLOR = (TColor)0x6B4900;
// ─── Малює спрайт з ImageList (розтягує до розміру клітинки) ────────────────
static void DrawImgCell(TImageList* il, TCanvas* canvas,
                        const TRect& rect, int imgIndex)
{
    TBitmap* bmp = new TBitmap();
    try {
        bmp->PixelFormat = pf32bit;
        bmp->Width  = il->Width;
        bmp->Height = il->Height;
        bmp->Canvas->Brush->Color = clBlack;
        bmp->Canvas->FillRect(TRect(0, 0, bmp->Width, bmp->Height));
        il->Draw(bmp->Canvas, 0, 0, imgIndex, true);
        canvas->StretchDraw(rect, bmp);
    } __finally { delete bmp; }
}
// ─── Конструктор / деструктор ────────────────────────────────────────────────
__fastcall TForm2::TForm2(TComponent* Owner) : TForm(Owner)
{
    player1 = new Game_logic();
    player2 = new Game_logic();
	BmpBamSea  = new TBitmap();
	BmpBamShip = new TBitmap();
	KillShip = new TBitmap();

    with_friend          = false;
    waiting_for_opponent = false;
    FIsDragging          = false;
    FDraggedShip         = NULL;
    last_c = last_r      = -1;
    current_dir          = 0;
    current_len          = 0;
    ships_placed         = 0;
    TDrawGrid* all_ships[] = {
        Ship_4_1,
        Ship_3_1, Ship_3_2,
        Ship_2_1, Ship_2_2, Ship_2_3,
        Ship_1_1, Ship_1_2, Ship_1_3, Ship_1_4
    };
    for (int i = 0; i < 10; i++)
        if (all_ships[i])
            FHomePositions[all_ships[i]] =
                TPoint(all_ships[i]->Left, all_ships[i]->Top);
    Application->OnMessage = AppMessage;
}
__fastcall TForm2::~TForm2()
{
    delete player1;
    delete player2;
    delete BmpBamSea;
	delete BmpBamShip;
	delete KillShip;
}
// ─── Текстура моря: спрайт №20 з ShipImages ─────────────────────────────────
void TForm2::DrawSeaCell(TCanvas* Canvas, const TRect& Rect)
{
    DrawImgCell(ShipImages, Canvas, Rect, 20);
}
// ─── Перехоплення WM_RBUTTONDOWN ────────────────────────────────────────────
void __fastcall TForm2::AppMessage(tagMSG &Msg, bool &Handled)
{
    if (Msg.message == WM_RBUTTONDOWN && FIsDragging && FDraggedShip) {
        RotateDraggedShip();
        Handled = true;
    }
}
// ─── Поворот корабля під час перетягування ───────────────────────────────────
void TForm2::RotateDraggedShip()
{
    TDrawGrid* ship = FDraggedShip;
    if (!ship) return;
    current_dir    = (current_dir == 0) ? 1 : 0;
    int tmp        = ship->ColCount;
    ship->ColCount = ship->RowCount;
    ship->RowCount = tmp;
    ship->Width  = ship->ColCount * (ship->DefaultColWidth  + 1);
    ship->Height = ship->RowCount * (ship->DefaultRowHeight + 1);
    TPoint mouseInForm = this->ScreenToClient(Mouse->CursorPos);
    ship->Left = mouseInForm.x;
    ship->Top  = mouseInForm.y;
    ship->Invalidate();
    TPoint p = Arrangement_field->ScreenToClient(
        ship->ClientToScreen(TPoint(0, 0)));
    Arrangement_field->MouseToCell(p.x + 2, p.y + 2, last_c, last_r);
    Arrangement_field->Repaint();
}
// ─── FormShow ────────────────────────────────────────────────────────────────
void __fastcall TForm2::FormShow(TObject *Sender)
{
    player1->all_clean();
    player2->all_clean();
    ships_placed         = 0;
    current_dir          = 0;
    FIsDragging          = false;
    FDraggedShip         = NULL;
    last_c = last_r      = -1;
    waiting_for_opponent = false;
    Btn_start->Enabled   = false;
    // ── Завантажуємо BMP один раз тут — дочірні форми беруть з Form2 ────
    String appDir = ExtractFileDir(Application->ExeName);
	try {
		ShipImages->GetBitmap(23, BmpBamSea);
		BmpBamSea->Transparent      = true;
		BmpBamSea->TransparentColor = BmpBamSea->Canvas->Pixels[0][0];
	} catch (...) {}
	try {
		ShipImages->GetBitmap(21, BmpBamShip);
		BmpBamShip->Transparent      = true;
		BmpBamShip->TransparentColor = BmpBamShip->Canvas->Pixels[0][0];
	} catch (...) {}
	try {
		ShipImages->GetBitmap(22, KillShip);
		KillShip->Transparent      = true;
		KillShip->TransparentColor = KillShip->Canvas->Pixels[0][0];
	} catch (...) {}
    for (std::map<TObject*, TPoint>::iterator it = FHomePositions.begin();
         it != FHomePositions.end(); ++it)
    {
        TDrawGrid* s   = dynamic_cast<TDrawGrid*>(it->first);
        TPoint     pos = it->second;
        if (!s) continue;
        if (s->Tag > 1 && s->RowCount > s->ColCount) {
            int tmp2    = s->ColCount;
            s->ColCount = s->RowCount;
            s->RowCount = tmp2;
            s->Width  = s->ColCount * (s->DefaultColWidth  + 1);
            s->Height = s->RowCount * (s->DefaultRowHeight + 1);
        }
        s->Left    = pos.x;
        s->Top     = pos.y;
        s->Visible = true;
    }
    DataModule1->OpponentReady = false;
    Arrangement_field->Invalidate();
}
// ─── Drag & Drop ─────────────────────────────────────────────────────────────
void __fastcall TForm2::ShipMouseDown(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y)
{
    TDrawGrid *ship = dynamic_cast<TDrawGrid*>(Sender);
    if (!ship) return;
    if (Button == mbLeft) {
        FIsDragging  = true;
        FDraggedShip = ship;
        ship->BringToFront();
        offset      = TPoint(0, 0);
        current_len = ship->Tag;
        current_dir = (ship->ColCount >= ship->RowCount) ? 0 : 1;
    }
}
void __fastcall TForm2::ShipMouseMove(TObject *Sender, TShiftState Shift,
    int X, int Y)
{
    TDrawGrid *ship = dynamic_cast<TDrawGrid*>(Sender);
    if (!FIsDragging || !ship || ship != FDraggedShip) return;
    TPoint mouseInForm = this->ScreenToClient(Mouse->CursorPos);
    ship->Left = mouseInForm.x;
    ship->Top  = mouseInForm.y;
    TPoint p = Arrangement_field->ScreenToClient(
        ship->ClientToScreen(TPoint(0, 0)));
    int c, r;
    Arrangement_field->MouseToCell(p.x + 2, p.y + 2, c, r);
    if (c != last_c || r != last_r) {
        last_c = c;
        last_r = r;
        Arrangement_field->Repaint();
    }
}
void __fastcall TForm2::ShipMouseUp(TObject *Sender, TMouseButton Button,
    TShiftState Shift, int X, int Y)
{
    TDrawGrid *ship = dynamic_cast<TDrawGrid*>(Sender);
    if (!ship || !FIsDragging || ship != FDraggedShip) return;
    FIsDragging  = false;
    FDraggedShip = NULL;
    TPoint p = Arrangement_field->ScreenToClient(
        ship->ClientToScreen(TPoint(0, 0)));
    int c, r;
    Arrangement_field->MouseToCell(p.x + 2, p.y + 2, c, r);
    bool placed = false;
    if (c >= 0 && r >= 0 && c < 10 && r < 10) {
        bool fits = (current_dir == 0 && c + current_len <= 10) ||
                    (current_dir == 1 && r + current_len <= 10);
        if (fits && player1->Cell_is_free(c, r, current_len,
                                          current_dir, player1->my_field)) {
            int baseIndex = (current_len == 1) ? 0 :
                            (current_len == 2) ? 1 :
                            (current_len == 3) ? 3 : 6;
            if (current_dir == 1) baseIndex += 10;
            for (int i = 0; i < current_len; i++) {
                int cx = c + (current_dir == 0 ? i : 0);
                int cy = r + (current_dir == 1 ? i : 0);
                player1->my_field[cx][cy] = baseIndex + i + 1;
            }
            ship->Visible = false;
            ships_placed++;
            if (ships_placed == 10) Btn_start->Enabled = true;
            placed = true;
        }
    }
    if (!placed) {
        if (ship->Tag > 1 && ship->RowCount > ship->ColCount) {
            int tmp2       = ship->ColCount;
            ship->ColCount = ship->RowCount;
            ship->RowCount = tmp2;
            ship->Width  = ship->ColCount * (ship->DefaultColWidth  + 1);
            ship->Height = ship->RowCount * (ship->DefaultRowHeight + 1);
            ship->Invalidate();
        }
        ship->Left = FHomePositions[ship].x;
        ship->Top  = FHomePositions[ship].y;
    }
    last_c = last_r = -1;
    Arrangement_field->Invalidate();
}
// ─── Клік на поле — видалити корабель ───────────────────────────────────────
bool TForm2::FindShipAt(int c, int r, int &outLen, int &outDir,
                         int &outHeadC, int &outHeadR)
{
    if (player1->my_field[c][r] <= 0 || player1->my_field[c][r] >= 100)
        return false;
    bool hasH =
        (c > 0 && player1->my_field[c-1][r] > 0 && player1->my_field[c-1][r] < 100) ||
        (c < 9 && player1->my_field[c+1][r] > 0 && player1->my_field[c+1][r] < 100);
    bool hasV =
        (r > 0 && player1->my_field[c][r-1] > 0 && player1->my_field[c][r-1] < 100) ||
        (r < 9 && player1->my_field[c][r+1] > 0 && player1->my_field[c][r+1] < 100);
    int dir = (hasV && !hasH) ? 1 : 0;
    int hc = c, hr = r;
    if (dir == 0)
        while (hc > 0 &&
               player1->my_field[hc-1][r] > 0 &&
               player1->my_field[hc-1][r] < 100) hc--;
    else
        while (hr > 0 &&
               player1->my_field[c][hr-1] > 0 &&
               player1->my_field[c][hr-1] < 100) hr--;
    int len = 0;
    if (dir == 0)
        while (hc+len < 10 &&
               player1->my_field[hc+len][r] > 0 &&
               player1->my_field[hc+len][r] < 100) len++;
    else
        while (hr+len < 10 &&
               player1->my_field[c][hr+len] > 0 &&
               player1->my_field[c][hr+len] < 100) len++;
    outLen = len; outDir = dir; outHeadC = hc; outHeadR = hr;
    return (len > 0);
}
void __fastcall TForm2::Arrangement_fieldMouseDown(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button != mbLeft || FIsDragging) return;
    int c, r;
    Arrangement_field->MouseToCell(X, Y, c, r);
    if (c < 0 || c >= 10 || r < 0 || r >= 10) return;
    if (player1->my_field[c][r] <= 0) return;
    int len, dir, hc, hr;
    if (!FindShipAt(c, r, len, dir, hc, hr)) return;
    for (int i = 0; i < len; i++) {
        int cx = hc + (dir == 0 ? i : 0);
        int cy = hr + (dir == 1 ? i : 0);
        player1->my_field[cx][cy] = 0;
    }
    if (ships_placed > 0) ships_placed--;
    Btn_start->Enabled = false;
    for (std::map<TObject*, TPoint>::iterator it = FHomePositions.begin();
         it != FHomePositions.end(); ++it)
    {
        TDrawGrid* s   = dynamic_cast<TDrawGrid*>(it->first);
        TPoint     pos = it->second;
		if (!s || s->Visible || s->Tag != len) continue;
		if (s->Tag > 1 && s->RowCount > s->ColCount) {
			int tmp2    = s->ColCount;
			s->ColCount = s->RowCount;
			s->RowCount = tmp2;
			s->Width  = s->ColCount * (s->DefaultColWidth  + 1);
			s->Height = s->RowCount * (s->DefaultRowHeight + 1);
		}
		s->Left    = pos.x;
		s->Top     = pos.y;
		s->Visible = true;
		break;
	}
	Arrangement_field->Invalidate();
}
// ─── Малювання поля розстановки ─────────────────────────────────────────────
// Фон — текстура моря (спрайт №20), кораблі малюються поверх.
//void __fastcall TForm2::Arrangement_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)
void __fastcall TForm2::Arrangement_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
	int val      = player1->my_field[ACol][ARow];
    TCanvas *cvs = Arrangement_field->Canvas;

    // Фон: текстура моря
    DrawSeaCell(cvs, Rect);

	// Корабель
	if (val > 0 && val < 100)
		DrawShipSprite(cvs, Rect, val - 1);

    // Підсвічування під час перетягування (зелений = можна, червоний = не можна)
    if (FIsDragging && last_c >= 0 && last_r >= 0) {
        bool isPh =
            (current_dir == 0 &&
             ARow == last_r &&
             ACol >= last_c && ACol < last_c + current_len) ||
            (current_dir == 1 &&
             ACol == last_c &&
             ARow >= last_r && ARow < last_r + current_len);

        if (isPh) {
            bool canPlace =
                ((current_dir == 0 && last_c + current_len <= 10) ||
                 (current_dir == 1 && last_r + current_len <= 10)) &&
                player1->Cell_is_free(last_c, last_r, current_len,
                                      current_dir, player1->my_field);

            cvs->Brush->Style = bsClear;
            cvs->Pen->Color   = canPlace ? clLime : clRed;
            cvs->Pen->Width   = 2;
            cvs->Rectangle(Rect);
            cvs->Pen->Width   = 1;
            cvs->Brush->Style = bsSolid;
        }
    }

    // Сітка
    cvs->Pen->Color = GRID_COLOR;
    cvs->Pen->Width = 1;
	cvs->FrameRect(Rect);
}
// ─── Малювання мінішипів у панелі вибору ─────────────────────────────────────
// Фон — теж текстура моря.
void __fastcall TForm2::AllShipDrawCell(TObject *Sender, int ACol, int ARow,
		  TRect &Rect, TGridDrawState State)
{
	TDrawGrid *grid = dynamic_cast<TDrawGrid*>(Sender);
	if (!grid) return;
	TCanvas* cvs = grid->Canvas;
    // Фон — текстура моря
	DrawImgCell(ShipImages, cvs, Rect, 20);
    int shipLen   = grid->Tag;
    int baseIndex = (shipLen == 1) ? 0 :
                    (shipLen == 2) ? 1 :
                    (shipLen == 3) ? 3 : 6;
    if (grid->RowCount > grid->ColCount) baseIndex += 10;
    int deckNum = (grid->ColCount > grid->RowCount) ? ACol : ARow;
	//DrawImgCell(ShipImages, grid->Canvas, Rect, baseIndex + deckNum);
	DrawShipSprite(cvs, Rect, baseIndex + deckNum);

    grid->Canvas->Pen->Color = GRID_COLOR;
    grid->Canvas->Pen->Width = 1;
    grid->Canvas->FrameRect(Rect);
}
// Спрайт корабля поверх моря
void TForm2::DrawShipSprite(TCanvas* cvs, const TRect& Rect, int spriteIndex)
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
// ─── Кнопки ──────────────────────────────────────────────────────────────────
void __fastcall TForm2::Btn_random_shipClick(TObject *Sender)
{
    player1->clear(player1->my_field);
    player1->setup(player1->my_field);
    for (std::map<TObject*, TPoint>::iterator it = FHomePositions.begin();
         it != FHomePositions.end(); ++it)
    {
        ((TControl*)it->first)->Visible = false;
    }
    ships_placed = 10;
    Btn_start->Enabled = true;
    Arrangement_field->Invalidate();
}
void __fastcall TForm2::Btn_startClick(TObject *Sender)
{
    if (!with_friend) {
        Form1->Show();
        this->Hide();
        return;
    }
    waiting_for_opponent = true;
    Btn_start->Enabled   = false;
    if (Label_stat)
        Label_stat->Caption = "Чекаємо суперника...";
    DataModule1->SendToOpponent("READY");
    if (DataModule1->OpponentReady)
        OnOpponentReady();
}
void __fastcall TForm2::FormClose(TObject *Sender, TCloseAction &Action)
{
    Application->Terminate();
}
void TForm2::OnOpponentReady()
{
    if (!waiting_for_opponent) {
        DataModule1->OpponentReady = true;
        if (Label_stat)
            Label_stat->Caption = "Суперник готовий! Натисніть Старт";
        return;
    }
    waiting_for_opponent       = false;
    DataModule1->OpponentReady = false;
    Form6->Show();
    this->Hide();
}

void __fastcall TForm2::Image3Click(TObject *Sender)
{
	Form3->Show();
	this->Hide();
}
//---------------------------------------------------------------------------

