#ifndef Play_with_friendH
#define Play_with_friendH

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Imaging.jpeg.hpp>
#include <vector>
#include <utility>
#include "Game_logic.h"

struct ShipCell {
	int x, y, value;
};
class TForm6 : public TForm
{
__published:
	TLabel    *Label_who;
	TLabel    *Label_your_count;
	TLabel    *Label_op_count;
	TDrawGrid *Arrangement_field;
	TDrawGrid *Attack_field;
	TImage *Image1;
	TSpeedButton *Btn_return;
	TImage *Image2;
	void __fastcall CommonMouseEnter(TObject *Sender);
	void __fastcall CommonMouseLeave(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Btn_returnClick(TObject *Sender);
	void __fastcall Attack_fieldMouseMove(TObject *Sender, TShiftState Shift,
	int X, int Y);
	void __fastcall Attack_fieldMouseLeave(TObject *Sender);
	void __fastcall Attack_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State);
	void __fastcall Arrangement_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State);
	void __fastcall Attack_fieldSelectCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, bool &CanSelect);
private:
	int  you_count;
    int  op_count;
    bool game_over;
	bool my_turn;
    int  hover_c;
	int  hover_r;
	void DrawKill(TCanvas* cvs, const TRect& Rect);
	void DrawSeaCell(TCanvas* Canvas, const TRect& Rect);
	void DrawShipSprite(TCanvas* cvs, const TRect& Rect, int spriteIndex);
	void DrawMiss(TCanvas* cvs, const TRect& Rect);
	void DrawHit(TCanvas* cvs, const TRect& Rect);
public:
	__fastcall TForm6(TComponent* Owner);

	void OnOpponentShot(int x, int y);
	void OnOpponentDisconnected();
    void ApplyShotResult(int x, int y, int result,
                         const std::vector<ShipCell>& shipCells,
						 const std::vector<std::pair<int,int> >& aroundCells);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm6 *Form6;
//---------------------------------------------------------------------------
#endif
