//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Start_win.h"
#include "New_game.h"
#include "Rating.h"
#include "Avtor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm4 *Form4;
//---------------------------------------------------------------------------
__fastcall TForm4::TForm4(TComponent* Owner)
    : TForm(Owner)
{
	SpeedButton1->Font->Color = (TColor)RGB(0, 240, 255);
	SpeedButton2->Font->Color = (TColor)RGB(0, 240, 255);
	SpeedButton3->Font->Color = (TColor)RGB(0, 240, 255);
	SpeedButton4->Font->Color = (TColor)RGB(0, 240, 255);
}

void __fastcall TForm4::FormClose(TObject *Sender, TCloseAction &Action)
{
	Application->Terminate();
}
void __fastcall TForm4::Button2Click(TObject *Sender)
{
	Close();
}

void __fastcall TForm4::Button1Click(TObject *Sender)
{
	Form3->Show();
	Form4->Hide();
}
void __fastcall TForm4::CommonMouseEnter(TObject *Sender)
{
	TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
	if (btn) {
		btn->Font->Color = (TColor)RGB(0, 0, 200);
	}
}

void __fastcall TForm4::CommonMouseLeave(TObject *Sender)
{
	TSpeedButton *btn = dynamic_cast<TSpeedButton*>(Sender);
	if (btn) {
		btn->Font->Color = (TColor)RGB(0, 240, 255);
	}
}
void __fastcall TForm4::SpeedButton2Click(TObject *Sender)
{
	Form7->Show();
	this->Hide();
}
//---------------------------------------------------------------------------

void __fastcall TForm4::SpeedButton3Click(TObject *Sender)
{
	Form8->Show();
	this->Hide();
}
//---------------------------------------------------------------------------

