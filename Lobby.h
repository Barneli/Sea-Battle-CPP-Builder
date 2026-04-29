//---------------------------------------------------------------------------
#ifndef LobbyH
#define LobbyH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>

#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Graphics.hpp>/
#include <Vcl.Graphics.hpp>/---------------------------------------------------------------------------
class TForm5 : public TForm
{
__published:    // "Створити гру" (хост)    // (залишаємо для сумісності з .dfm, можна сховати)
	TEdit   *edCode;     // поле введення 6-значного коду
    TLabel  *lblCode;    // показує згенерований код (для хоста)
    TLabel  *lblStatus;
	TImage *Image1;
	TImage *Image2;
	TSpeedButton *Button1;
	TImage *Image3;
	TSpeedButton *Button2;
	TImage *Image4;
	TSpeedButton *BtnJoin;
	TImage *Image5;  // поточний статус з'єднання

    void __fastcall Button1Click(TObject *Sender);
    void __fastcall BtnJoinClick(TObject *Sender);   // підключення по коду
    void __fastcall edCodeChange(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall CommonMouseEnter(TObject *Sender);
    void __fastcall CommonMouseLeave(TObject *Sender);
	void __fastcall Image5Click(TObject *Sender);
private:
    void TryConnect(const String& code);  // спільна логіка підключення

public:
    __fastcall TForm5(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm5 *Form5;
//---------------------------------------------------------------------------
#endif
