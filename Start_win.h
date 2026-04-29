//---------------------------------------------------------------------------
#ifndef Start_winH
#define Start_winH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>

#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Graphics.hpp>/
#include <Vcl.Imaging.pngimage.hpp>/---------------------------------------------------------------------------
class TForm4 : public TForm
{
__published:  // (резерв)
	TImage *Image1;
	TSpeedButton *SpeedButton1;
	TSpeedButton *SpeedButton2;
	TSpeedButton *SpeedButton3;
	TSpeedButton *SpeedButton4;
	TImage *Image2;
	TImage *Image3;
	TImage *Image4;
	TImage *Image5;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall CommonMouseEnter(TObject *Sender);
    void __fastcall CommonMouseLeave(TObject *Sender);
	void __fastcall SpeedButton2Click(TObject *Sender);
	void __fastcall SpeedButton3Click(TObject *Sender);
private:
public:
    __fastcall TForm4(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm4 *Form4;
//---------------------------------------------------------------------------
#endif
