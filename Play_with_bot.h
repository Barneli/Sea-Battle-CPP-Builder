#ifndef Play_with_botH
#define Play_with_botH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include "Game_logic.h"
#include "Arrangment_ship.h"
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Imaging.jpeg.hpp>

class TForm1 : public TForm
{
__published:
	TDrawGrid *Arrangement_field;
	TDrawGrid *Attack_field;
	TImage *Image1;
	TImage *Image2;
	TSpeedButton *Btn_return;
	TLabel *Label_op_count;
	TLabel *Label_who;
	TLabel *Label_your_count;
	//			 äëÿ âåðñ³¿ 12
	/*void __fastcall Attack_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State);
	void __fastcall Arrangement_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State);
	void __fastcall Attack_fieldSelectCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, bool &CanSelect);  */

	//                ÄËß ÂÅÐÑ²¯ 10
	void __fastcall Attack_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State);
	void __fastcall Arrangement_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State);
	void __fastcall Attack_fieldSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);

	void __fastcall CommonMouseEnter(TObject *Sender);
	void __fastcall CommonMouseLeave(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Btn_returnClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Attack_fieldMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall Attack_fieldMouseLeave(TObject *Sender);
private:
	int  you_count;
	int  op_count;
	bool game_over;
    bool stats_saved;
	int  hover_c;
	int  hover_r;
	void SyncAttackField();
	void DrawSeaCell(TCanvas* Canvas, const TRect& Rect);
	void DrawShipSprite(TCanvas* cvs, const TRect& Rect, int spriteIndex);
	void DrawMiss(TCanvas* cvs, const TRect& Rect);
	void DrawHit(TCanvas* cvs, const TRect& Rect);
	void DrawKill(TCanvas* cvs, const TRect& Rect);
public:
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
