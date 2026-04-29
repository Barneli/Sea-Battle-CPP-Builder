#include <vcl.h>
#pragma hdrstop

#include "New_game.h"
#include "Arrangment_ship.h"
#include "Start_win.h"
#include "Lobby.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm3 *Form3;

// Глобальні змінні
bool   with_friend       = false;
int    CurrentPlayerId   = -1;
String CurrentPlayerName = "";

//---------------------------------------------------------------------------
// Оновлення статистики гравця — викликається з Play_with_bot
// won = true якщо переміг, shots = кількість пострілів (0 якщо здався)
void UpdatePlayerStats(bool won, int shots)
{
    if (CurrentPlayerId < 0) return;

    // Шлях до БД — поруч з .exe
	String dbPath = ExtractFilePath(Application->ExeName) + "Rating.mdb";

    TADOConnection *conn = new TADOConnection(NULL);
    TADOQuery      *qry  = new TADOQuery(NULL);
    try {
        conn->ConnectionString =
            "Provider=Microsoft.Jet.OLEDB.4.0;"
            "Data Source=" + dbPath + ";"
            "Persist Security Info=False;";
        conn->LoginPrompt = false;
        conn->Open();

        qry->Connection = conn;

        // Спочатку читаємо поточні значення
        qry->SQL->Text =
            "SELECT GamesPlayed, GamesWon, GamesLost, MinShotsToWin "
			"FROM BotRatings WHERE IdPlayer = :id";
        qry->Parameters->ParamByName("id")->Value = CurrentPlayerId;
        qry->Open();

        int played = qry->FieldByName("GamesPlayed")->AsInteger;
        int gWon   = qry->FieldByName("GamesWon")->AsInteger;
        int gLost  = qry->FieldByName("GamesLost")->AsInteger;
        int minShots = qry->FieldByName("MinShotsToWin")->AsInteger;
        qry->Close();

        played++;
        if (won) {
            gWon++;
            // Оновлюємо мінімум пострілів тільки при перемозі
            if (minShots == 0 || shots < minShots)
                minShots = shots;
        } else {
            gLost++;
        }

        // Записуємо оновлені значення
        qry->SQL->Text =
			"UPDATE BotRatings SET "
            "GamesPlayed = :played, "
            "GamesWon    = :won, "
            "GamesLost   = :lost, "
            "MinShotsToWin = :mins "
            "WHERE IdPlayer = :id";
        qry->Parameters->ParamByName("played")->Value = played;
        qry->Parameters->ParamByName("won")->Value    = gWon;
        qry->Parameters->ParamByName("lost")->Value   = gLost;
        qry->Parameters->ParamByName("mins")->Value   = minShots;
        qry->Parameters->ParamByName("id")->Value     = CurrentPlayerId;
        qry->ExecSQL();

    } catch (const Exception &e) {
        ShowMessage("Помилка оновлення БД: " + e.Message);
    }
    delete qry;
    delete conn;
}

//---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner)
    : TForm(Owner)
{
    // DBConn НЕ створюємо тут — щоб уникнути EClassNotFound
    // ADO об'єкти створюємо локально тільки при потребі
    Button1->Font->Color = (TColor)RGB(0, 240, 255);
    Button2->Font->Color = (TColor)RGB(0, 240, 255);
    Button3->Font->Color = (TColor)RGB(0, 240, 255);
    Button2->Enabled = false;
    Button3->Enabled = false;
}

__fastcall TForm3::~TForm3()
{
}

//---------------------------------------------------------------------------
// InitDB і FindOrCreatePlayer об'єднані в одну функцію
// Всі ADO об'єкти — локальні, щоб уникнути EClassNotFound
void TForm3::FindOrCreatePlayer(const String& name)
{
	String dbPath = ExtractFilePath(Application->ExeName) + "Rating.mdb";

    TADOConnection *conn = new TADOConnection(NULL);
    TADOQuery      *qry  = new TADOQuery(NULL);
    try {
        conn->ConnectionString =
            "Provider=Microsoft.Jet.OLEDB.4.0;"
            "Data Source=" + dbPath + ";"
            "Persist Security Info=False;";
        conn->LoginPrompt = false;
        conn->Open();

        qry->Connection = conn;

        // Пошук гравця
        qry->SQL->Text =
			"SELECT IdPlayer FROM BotRatings "
			"WHERE PlayerName = :nm";
        qry->Parameters->ParamByName("nm")->Value = name;
        qry->Open();

        if (!qry->Eof) {
            CurrentPlayerId = qry->FieldByName("IdPlayer")->AsInteger;
            qry->Close();
        } else {
            qry->Close();
            // Новий гравець
            qry->SQL->Text =
				"INSERT INTO BotRatings "
                "(PlayerName, GamesPlayed, GamesWon, GamesLost, MinShotsToWin) "
                "VALUES (:nm, 0, 0, 0, 0)";
            qry->Parameters->ParamByName("nm")->Value = name;
            qry->ExecSQL();

            // Читаємо ID
            qry->SQL->Text =
                "SELECT IdPlayer FROM BotRatings WHERE PlayerName = :nm";
            qry->Parameters->ParamByName("nm")->Value = name;
            qry->Open();
            CurrentPlayerId = qry->FieldByName("IdPlayer")->AsInteger;
            qry->Close();
        }

        CurrentPlayerName = name;

    } catch (const Exception &e) {
        ShowMessage("Помилка БД: " + e.Message);
        CurrentPlayerId = -1;
    }
    delete qry;
    delete conn;
}

//---------------------------------------------------------------------------
void __fastcall TForm3::Edit1Change(TObject *Sender)
{
    Button1->Enabled = (Edit1->Text.Trim().Length() > 0);
    // Якщо змінили ім'я — скидаємо підтвердження
    Button2->Enabled = false;
    Button3->Enabled = false;
    CurrentPlayerId = -1;
}

void __fastcall TForm3::Button1Click(TObject *Sender)
{
    if (Edit1->Text.Trim().IsEmpty()) {
        ShowMessage("Введіть ваше ім'я!");
        return;
    }

    // Підключаємось до БД і знаходимо/створюємо гравця
    FindOrCreatePlayer(Edit1->Text.Trim());

    if (CurrentPlayerId >= 0) {
        Button2->Enabled = true;
        Button3->Enabled = true;
		//ShowMessage("Привіт, " + CurrentPlayerName + "! Ваш ID: " + IntToStr(CurrentPlayerId));
    }
}

void __fastcall TForm3::Button2Click(TObject *Sender)
{
    with_friend = true;
    Form5->Show();
    Form3->Hide();
}

void __fastcall TForm3::Button3Click(TObject *Sender)
{
    with_friend = false;
    Form2->Show();
    Form3->Hide();
}

void __fastcall TForm3::CommonMouseEnter(TObject *Sender)
{
    TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
    if (btn) btn->Font->Color = (TColor)RGB(0, 0, 200);
}

void __fastcall TForm3::CommonMouseLeave(TObject *Sender)
{
    TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
    if (btn) btn->Font->Color = (TColor)RGB(0, 240, 255);
}

void __fastcall TForm3::FormClose(TObject *Sender, TCloseAction &Action)
{
    Application->Terminate();
}

void __fastcall TForm3::Image4Click(TObject *Sender)
{
    Form4->Show();
    this->Hide();
}

