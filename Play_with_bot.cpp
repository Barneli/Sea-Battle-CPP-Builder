#include <vcl.h>
#pragma hdrstop
#include "Play_with_bot.h"
#include "Arrangment_ship.h"
#include "New_game.h"
#include <algorithm>
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
static const TColor GRID_COLOR = (TColor)0x6B4900;
//---------------------------------------------------------------------------
// Малює спрайт з ShipImages розтягуючи до розміру клітинки.
// Фон спрайта — clBlack, тому ImageList::Draw з dsTransparent
// коректно прибирає чорні пікселі.
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
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)
{
    you_count = 0;
    op_count  = 0;
    game_over = false;
    hover_c   = -1;
    hover_r   = -1;
    Btn_return->Font->Color = (TColor)RGB(0, 240, 255);
    Btn_return->Caption     = "ЗДАТИСЬ";
}
//---------------------------------------------------------------------------
// Текстура моря — спрайт №20 зі ShipImages
void TForm1::DrawSeaCell(TCanvas* Canvas, const TRect& Rect)
{
	DrawSprite(Form2->ShipImages, Canvas, Rect, 20);
}
// Спрайт корабля поверх моря
void TForm1::DrawShipSprite(TCanvas* cvs, const TRect& Rect, int spriteIndex)
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
//---------------------------------------------------------------------------
// Синхронізує attacking_field гравця з реальним станом поля бота.
// attacking_field:
//   0          — не стріляли (нічого не малюємо)
//   100        — промах
//  -1          — влучили, корабель ще живий
//  > 0 && <100 — потоплений (значення = індекс спрайта + 1)
void TForm1::SyncAttackField()
{
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			int val = Form2->player2->my_field[i][j];
			if (val == 100) {
				// Промах записаний ботом-полем
                Form2->player1->attacking_field[i][j] = 100;
            } else if (val < 0 && val > -100) {
                // Підбита клітина
                if (Form2->player2->is_destroyed(i, j, Form2->player2->my_field))
                    Form2->player1->attacking_field[i][j] = abs(val); // потоплено
				else
                    Form2->player1->attacking_field[i][j] = -1;       // просто влучили
            }
            // val > 0 (жива частина) або val == 0 — залишаємо attacking_field без змін
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
    Application->Terminate();
}
void __fastcall TForm1::Btn_returnClick(TObject *Sender)
{
    if (!game_over && !stats_saved) {
        // Гравець здався під час гри — рахуємо як поразку
        stats_saved = true;
        game_over   = true;
        UpdatePlayerStats(false, 0);
    }
    // Скидаємо прапори для наступної гри
    game_over   = false;
    stats_saved = false;
    Btn_return->Caption = "ЗДАТИСЬ";
    Form2->Show();
    this->Hide();
}
void __fastcall TForm1::FormShow(TObject *Sender)
{
    you_count   = 0;
    op_count    = 0;
    game_over   = false;
    stats_saved = false;
    hover_c     = -1;
    hover_r     = -1;
    Label_your_count->Caption = "Ви зробили 0 пострілів";
    Label_op_count->Caption   = "Бот зробив 0 пострілів";
    Label_who->Caption        = "Ваш хід";
    Attack_field->Enabled      = true;
    Arrangement_field->Enabled = true;
    Form2->player1->reset_hunt();
    Form2->player2->all_clean();
    Form2->player2->setup(Form2->player2->my_field);
    Attack_field->Invalidate();
    Arrangement_field->Invalidate();
}
//---------------------------------------------------------------------------
// СВОЄ ПОЛЕ (Arrangement_field)
// Відображає: кораблі, промахи бота (val==100), влучання бота (val<0)
//void __fastcall TForm1::Arrangement_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State)
void __fastcall TForm1::Arrangement_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
	int      val = Form2->player1->my_field[ACol][ARow];
    TCanvas* cvs = Arrangement_field->Canvas;

    // Фон — текстура моря
    DrawSeaCell(cvs, Rect);

    if (val > 0 && val < 100) {
        // Живий корабель — просто спрайт
        DrawShipSprite(cvs, Rect, val - 1);

    } else if (val < 0 && val > -100) {
        // Підбита клітина: спрайт корабля
		DrawShipSprite(cvs, Rect, abs(val) - 1);

        // Перевіряємо чи весь корабель потоплено
        if (Form2->player1->is_destroyed(ACol, ARow, Form2->player1->my_field)) {
            // Корабель потоплено — накладаємо KillShip
			if (Form2->KillShip && Form2->KillShip->Width > 0)
				DrawShipSprite(cvs, Rect, 22);
		} else {
            // Просто влучили — накладаємо BmpBamShip
            if (Form2->BmpBamShip && Form2->BmpBamShip->Width > 0)
				DrawShipSprite(cvs, Rect, 21);
		}

    } else if (val == 100) {
        // Промах бота — BmpBamSea
		DrawShipSprite(cvs, Rect, 23);
	}

    cvs->Pen->Color = GRID_COLOR;
    cvs->Pen->Width = 1;
	cvs->FrameRect(Rect);
}
//---------------------------------------------------------------------------
// ПОЛЕ АТАКИ (Attack_field)
// attacking_field: 0=нічого, 100=промах, -1=влучили(не потоплено),
//                  >0 <100 = потоплено (індекс спрайта+1)
//void __fastcall TForm1::Attack_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State)
void __fastcall TForm1::Attack_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
	int      val = Form2->player1->attacking_field[ACol][ARow];
    TCanvas* cvs = Attack_field->Canvas;

    // Фон — текстура моря
    DrawSeaCell(cvs, Rect);

    if (val == 100) {
        // Промах — BmpBamSea
		DrawShipSprite(cvs, Rect, 23);

    } else if (val == -1) {
        // Влучили, корабель ще живий — BmpBamShip
        // (спрайт корабля не показуємо — суперник ще не знає де він)
		if (Form2->BmpBamShip && Form2->BmpBamShip->Width > 0)
			DrawShipSprite(cvs, Rect, 21);

    } else if (val > 0 && val < 100) {
        // Потоплений корабель — спрайт + KillShip поверх
		DrawShipSprite(cvs, Rect, val - 1);
		if (Form2->KillShip && Form2->KillShip->Width > 0)
			cvs->StretchDraw(Rect, Form2->KillShip);
			DrawShipSprite(cvs, Rect, 22);
	}

    // Hover: червона рішітка на порожній клітинці під курсором
    if (ACol == hover_c && ARow == hover_r &&
        !game_over && Attack_field->Enabled && val == 0)
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
// Рух миші — оновлюємо hover клітинку
void __fastcall TForm1::Attack_fieldMouseMove(TObject *Sender,
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
// Миша вийшла за межі поля — знімаємо hover
void __fastcall TForm1::Attack_fieldMouseLeave(TObject *Sender)
{
    if (hover_c != -1 || hover_r != -1) {
        hover_c = -1;
        hover_r = -1;
        Attack_field->Invalidate();
    }
}
//---------------------------------------------------------------------------
// ХІД ГРАВЦЯ
//void __fastcall TForm1::Attack_fieldSelectCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, bool &CanSelect)
void __fastcall TForm1::Attack_fieldSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    if (game_over || !Attack_field->Enabled) return;
    // Вже стріляли сюди
    if (Form2->player1->attacking_field[ACol][ARow] != 0) return;
    int result = Form2->player2->make_shot(ACol, ARow, Form2->player2->my_field);
    if (result == -1) return;
    Label_your_count->Caption =
        "Ви зробили " + IntToStr(++you_count) + " пострілів";
    SyncAttackField();
    Attack_field->Invalidate();
    // Перевірка перемоги
    if (!Form2->player2->has_ships(Form2->player2->my_field)) {
        game_over             = true;
        Label_who->Caption    = "Перемога!";
        Attack_field->Enabled = false;
        Btn_return->Caption = "ПОВТОРИТИ";
        if (!stats_saved) {
            stats_saved = true;
            UpdatePlayerStats(true, you_count);
        }
        ShowMessage("Вітаємо! Ви потопили всі кораблі суперника!");
        return;
    }
    if (result > 0) {
        // Влучили — гравець стріляє ще раз
        Label_who->Caption = "Влучили! Стріляйте ще";
        return;
    }
    // Промах — хід бота
    Label_who->Caption    = "Хід бота...";
    Attack_field->Enabled = false;
    int bot_result;
    do {
        Application->ProcessMessages();
        if (game_over) return;
        Sleep(600);
        if (game_over) return;
        bot_result = Form2->player1->bot_move(Form2->player1->my_field);
        if (bot_result == -1) break;
        Label_op_count->Caption =
            "Бот зробив " + IntToStr(++op_count) + " пострілів";
        Arrangement_field->Invalidate();
        Application->ProcessMessages();
        if (game_over) return;
        if (!Form2->player1->has_ships(Form2->player1->my_field)) {
            game_over             = true;
            Label_who->Caption    = "Поразка!";
            Attack_field->Enabled = false;
            Btn_return->Caption = "РЕВАНШ";
            if (!stats_saved) {
                stats_saved = true;
                UpdatePlayerStats(false, 0);
            }
            ShowMessage("Бот потопив усі ваші кораблі. Спробуйте ще раз!");
            return;
        }
    } while (bot_result > 0);
    if (!game_over) {
        Label_who->Caption    = "Ваш хід";
        Attack_field->Enabled = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CommonMouseEnter(TObject *Sender)
{
    TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
    if (btn) btn->Font->Color = (TColor)RGB(0, 0, 200);
}
void __fastcall TForm1::CommonMouseLeave(TObject *Sender)
{
    TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
    if (btn) btn->Font->Color = (TColor)RGB(0, 240, 255);
}
