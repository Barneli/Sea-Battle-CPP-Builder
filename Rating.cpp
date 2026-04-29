#include <vcl.h>
#pragma hdrstop

#include "Rating.h"
#include "Start_win.h"
#include <Data.Win.ADODB.hpp>

#pragma package(smart_init)
#pragma resource "*.dfm"
TForm7 *Form7;

__fastcall TForm7::TForm7(TComponent* Owner)
    : TForm(Owner)
{
}

__fastcall TForm7::~TForm7()
{
}

//---------------------------------------------------------------------------
void TForm7::LoadRating()
{
	String dbPath = ExtractFilePath(Application->ExeName) + "Rating.mdb";

	TADOConnection *conn = new TADOConnection(NULL);
	TADOQuery      *qry  = new TADOQuery(NULL);

	// Масив копок для зручності
	TSpeedButton* buttons[] = {
		SpeedButton1,
		SpeedButton2,
		SpeedButton3
    };

    // Очищаємо перед завантаженням
    for (int i = 0; i < 3; i++) {
        buttons[i]->Caption = IntToStr(i+1) + ". —";
        buttons[i]->Tag = -1; // Скидаємо ID
	}

    try {
		conn->ConnectionString =
			"Provider=Microsoft.Jet.OLEDB.4.0;"
			"Data Source=" + dbPath + ";"
			"Persist Security Info=False;";
		conn->LoginPrompt = false;
		conn->Open();

        qry->Connection = conn;

        // Сортуємо залежно від вибраного критерію
        String sortField = "GamesWon"; // за замовчуванням
        if (ComboBox1->ItemIndex == 1)
            sortField = "GamesPlayed";
        else if (ComboBox1->ItemIndex == 2)
            sortField = "MinShotsToWin";  // менше = краще, тому ASC

        String orderDir = (ComboBox1->ItemIndex == 2) ? "ASC" : "DESC";

        // Фільтруємо тих хто взагалі грав
        qry->SQL->Text =
			"SELECT TOP 3 PlayerName, GamesPlayed, GamesWon, GamesLost, MinShotsToWin "
			"FROM BotRatings "
			"WHERE GamesPlayed > 0 AND MinShotsToWin > 0 "
			"ORDER BY " + sortField + " " + orderDir;
        qry->Open();

        int rank = 0;
		while (!qry->Eof && rank < 3) {
            String name    = qry->FieldByName("PlayerName")->AsString;
            int played     = qry->FieldByName("GamesPlayed")->AsInteger;
            int won        = qry->FieldByName("GamesWon")->AsInteger;
            int lost       = qry->FieldByName("GamesLost")->AsInteger;
            int minShots   = qry->FieldByName("MinShotsToWin")->AsInteger;

            // Відсоток перемог
            double winPct = (played > 0) ? (won * 100.0 / played) : 0.0;
            String minStr = (minShots > 0) ? IntToStr(minShots) : "—";

			buttons[rank]->Caption =name;

            qry->Next();
            rank++;
        }
        qry->Close();

    } catch (const Exception &e) {
        ShowMessage("Помилка завантаження рейтингу:\n" + e.Message);
    }
    delete qry;
    delete conn;
}

//---------------------------------------------------------------------------
void __fastcall TForm7::FormShow(TObject *Sender)
{
    // При відкритті форми — завантажуємо рейтинг з сортуванням за замовч.
    if (ComboBox1->Items->Count == 0) {
        ComboBox1->Items->Add("За кількістю перемог");
        ComboBox1->Items->Add("За кількістю ігор");
        ComboBox1->Items->Add("За мін. кількістю ходів");
        ComboBox1->ItemIndex = 0;
    }
    LoadRating();
}

void __fastcall TForm7::ComboBox1Change(TObject *Sender)
{
    LoadRating();
}

void __fastcall TForm7::FormClose(TObject *Sender, TCloseAction &Action)
{
    Application->Terminate();
}

void __fastcall TForm7::Image3Click(TObject *Sender)
{
    Form4->Show();
    this->Hide();
}
