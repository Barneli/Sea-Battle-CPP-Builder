//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Lobby.h"
#include "dmNetwork.h"
#include "Arrangment_ship.h"
#include "New_game.h"
#include <System.Math.hpp>
#include <IdGlobal.hpp>
#include <IdUDPClient.hpp>

#pragma package(smart_init)
#pragma resource "*.dfm"
TForm5 *Form5;

static TIdBytes AnsiToIdBytes(const AnsiString& s)
{
	TIdBytes result;
	result.Length = s.Length();
	for (int i = 0; i < s.Length(); i++)
		result[i] = (Byte)s[i + 1];
	return result;
}

static AnsiString IdBytesToAnsi(const TIdBytes& b)
{
	AnsiString result;
	result.SetLength(b.Length);
	for (int i = 0; i < b.Length; i++)
		result[i + 1] = (char)b[i];
	return result;
}

//---------------------------------------------------------------------------
__fastcall TForm5::TForm5(TComponent* Owner)
	: TForm(Owner)
{
	Button1->Font->Color = (TColor)RGB(0, 240, 255);
	Button2->Font->Color = (TColor)RGB(0, 240, 255);
	BtnJoin->Font->Color = (TColor)RGB(0, 240, 255);
	BtnJoin->Enabled = false;
}

//---------------------------------------------------------------------------
void __fastcall TForm5::Button1Click(TObject *Sender)
{
	lblCode->Visible = true;
	Button1->Enabled = false;
	Button2->Enabled = false;

	Randomize();
	DataModule1->GameCode = IntToStr(RandomRange(100000, 999999));
	DataModule1->IsHost   = true;

	lblCode->Caption = "Ваш код: " + DataModule1->GameCode;

	DataModule1->UDPServer->Active = false;
	DataModule1->UDPServer->DefaultPort = 8888;
	DataModule1->UDPServer->Active = true;

	DataModule1->TCPServer->Active = false;
	DataModule1->TCPServer->DefaultPort = 7777;
	DataModule1->TCPServer->Active = true;

	lblStatus->Caption = "Очікування друга...";
}

//---------------------------------------------------------------------------
void __fastcall TForm5::edCodeChange(TObject *Sender)
{
	String code = edCode->Text.Trim();
	String filtered = "";

	for (int i = 1; i <= code.Length(); i++) {
		if (code[i] >= '0' && code[i] <= '9')
			filtered += code[i];
	}

	if (filtered.Length() > 6)
		filtered = filtered.SubString(1, 6);

	if (filtered != edCode->Text) {
		edCode->Text = filtered;
		edCode->SelStart = filtered.Length();
		return;
	}

	bool ready = (filtered.Length() == 6);
	BtnJoin->Enabled = ready;

	if (ready)
		lblStatus->Caption = "Код введено. Натисніть 'Приєднатися'";
	else
		lblStatus->Caption = (filtered.Length() > 0)
			? "Введіть ще " + IntToStr(6 - filtered.Length()) + " цифр..."
			: "";
}

//---------------------------------------------------------------------------
void __fastcall TForm5::BtnJoinClick(TObject *Sender)
{
	TryConnect(edCode->Text.Trim());
}

//---------------------------------------------------------------------------
void TForm5::TryConnect(const String& code)
{
	BtnJoin->Enabled   = false;
	lblStatus->Caption = "Пошук гри...";
	Application->ProcessMessages();

	String foundIP = "";

	// Створюємо окремий UDP клієнт з фіксованим портом відправника 8890
	// Сервер відповідає на той самий порт звідки прийшов запит
	TIdUDPClient *udp = new TIdUDPClient(NULL);
	try {
		udp->BroadcastEnabled = true;
		udp->BoundPort = 8890;  // фіксований порт — сервер відповідає сюди
		udp->Active    = true;

		// Надсилаємо без вказання reply_port — сервер відповідає на PeerPort (8890)
		String fullRequest = "FIND_GAME:" + code;
		TIdBytes RequestData = AnsiToIdBytes(AnsiString(fullRequest.c_str()));

		TStringList *hosts = new TStringList();
		// Спочатку broadcast (для двох ПК), потім localhost (для тесту на одному)
		// Так реальна мережа матиме пріоритет
		hosts->Add("255.255.255.255");
		hosts->Add("127.0.0.1");

		try {
			for (int i = 0; i < hosts->Count && foundIP.IsEmpty(); i++) {
				String host = hosts->Strings[i];
				lblStatus->Caption = "Перевіряємо " + host + "...";
				Application->ProcessMessages();

				try {
					udp->Host = host;
					udp->Port = 8888;
					udp->SendBuffer(RequestData);

					// Відповідь прийде на порт 8890 (наш BoundPort)
					TIdBytes ResponseData;
					String PeerIP    = "";
					uint16_t PeerPort = 0;
					ResponseData.Length = 1024;
					udp->ReceiveBuffer(ResponseData, PeerIP, PeerPort, 3000);

					AnsiString respAnsi = IdBytesToAnsi(ResponseData);
					String Response = String(respAnsi).Trim();

					//ShowMessage("Відповідь від " + PeerIP + ": [" + Response + "]");

					if (Response == "I_AM_SERVER") {
						// Якщо відповідь від broadcast — беремо реальний IP сервера
						// Якщо localhost — підключаємось до 127.0.0.1
						foundIP = (host == "127.0.0.1") ? "127.0.0.1" : PeerIP;
						//ShowMessage("Знайдено сервер: " + foundIP); // діагностика
					}
				} catch (const Exception &e) {
					ShowMessage("Тайм-аут або помилка для " + host + ": " + e.Message);
				}
			}
		} __finally {
			delete hosts;
		}

	} __finally {
		udp->Active = false;
		delete udp;
	}

	if (foundIP.IsEmpty()) {
		lblStatus->Caption = "Сервер не знайдено. Перевір код або чи запущено хост.";
		BtnJoin->Enabled   = true;
		return;
	}

	// TCP підключення
	try {
		DataModule1->ServerIP = foundIP;
		DataModule1->IsHost   = false;

		DataModule1->TCPClient->Host = foundIP;
		DataModule1->TCPClient->Port = 7777;

		lblStatus->Caption = "З'єднуємось по TCP...";
		Application->ProcessMessages();

		DataModule1->TCPClient->Connect();

		lblStatus->Caption = "З'єднано!";
		Application->ProcessMessages();
		Sleep(300);

		this->Hide();
		if (Form2) Form2->Show();

	} catch (const Exception &e) {
		lblStatus->Caption = "Помилка TCP: " + e.Message;
		BtnJoin->Enabled   = true;
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm5::Button2Click(TObject *Sender)
{
	Button1->Enabled = false;
	Button2->Enabled = false;
	edCode->Visible  = true;
	edCode->SetFocus();
}
void __fastcall TForm5::CommonMouseEnter(TObject *Sender)
{
	TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
	if (btn) {
		btn->Font->Color = (TColor)RGB(0, 0, 200);
	}
}

void __fastcall TForm5::CommonMouseLeave(TObject *Sender)
{
	TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
	if (btn) {
		btn->Font->Color = (TColor)RGB(0, 240, 255);
	}
}

void __fastcall TForm5::FormClose(TObject *Sender, TCloseAction &Action)
{
	Application->Terminate();
}
void __fastcall TForm5::Image5Click(TObject *Sender)
{
	Form3->Show();
	this->Hide();
}
//---------------------------------------------------------------------------

