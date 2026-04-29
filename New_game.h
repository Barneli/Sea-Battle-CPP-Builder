//---------------------------------------------------------------------------
#ifndef New_gameH
#define New_gameH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Data.Win.ADODB.hpp>
#include <Data.DB.hpp>
#include <Vcl.Graphics.hpp>

// Глобальний прапор: true = грати з другом, false = з ботом
extern bool with_friend;
extern int    CurrentPlayerId;
extern String CurrentPlayerName;

void UpdatePlayerStats(bool won, int shots);

class TForm3 : public TForm
{
__published:
    TLabel  *Label1;
    TEdit   *Edit1;
	TImage *Image1;
	TImage *Image5;
	TSpeedButton *Button1;
	TSpeedButton *Button2;
	TImage *Image2;
	TImage *Image3;
	TSpeedButton *Button3;
	TImage *Image4;
	TADOConnection *conn;
	TADOQuery *qry;

	  // Гра з ботом

    void __fastcall Edit1Change(TObject *Sender);
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Button3Click(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall CommonMouseEnter(TObject *Sender);
	void __fastcall CommonMouseLeave(TObject *Sender);
	void __fastcall Image4Click(TObject *Sender);
private:

    void InitDB();
	void FindOrCreatePlayer(const String& name);
public:
	__fastcall TForm3(TComponent* Owner);
	__fastcall TForm3::~TForm3();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm3 *Form3;
//---------------------------------------------------------------------------
#endif
