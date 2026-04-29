#ifndef Arrangment_shipH
#define Arrangment_shipH

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <map>
#include "Game_logic.h"

class TForm2 : public TForm
{
__published:
	TDrawGrid  *Arrangement_field;
	TDrawGrid  *Ship_4_1;
	TDrawGrid  *Ship_3_1;
	TDrawGrid  *Ship_3_2;
	TDrawGrid  *Ship_2_1;
	TDrawGrid  *Ship_2_2;
	TDrawGrid  *Ship_2_3;
	TDrawGrid  *Ship_1_1;
	TDrawGrid  *Ship_1_2;
	TDrawGrid  *Ship_1_3;
	TDrawGrid  *Ship_1_4;
	TImageList *ShipImages;
	TLabel     *Label_stat;
	TImage *Image5;
	TSpeedButton *Btn_start;
	TImage *Image1;
	TSpeedButton *SpeedButton1;
	TImage *Image2;
	TImage *Image3;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Btn_random_shipClick(TObject *Sender);
	void __fastcall Btn_startClick(TObject *Sender);

	void __fastcall ShipMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall ShipMouseDown(TObject *Sender, TMouseButton Button,
								  TShiftState Shift, int X, int Y);
	void __fastcall ShipMouseUp(TObject *Sender, TMouseButton Button,
								TShiftState Shift, int X, int Y);
	void __fastcall Arrangement_fieldMouseDown(TObject *Sender,
											   TMouseButton Button,
											   TShiftState Shift, int X, int Y);
/*	void __fastcall Arrangement_fieldDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State);
	void __fastcall AllShipDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow, TRect &Rect, TGridDrawState State);   */
	void __fastcall Image3Click(TObject *Sender);

	void __fastcall Arrangement_fieldDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State);
	void __fastcall AllShipDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect, TGridDrawState State);

private:
	int last_c, last_r;
	int current_dir;
	int current_len  = 0;
	int ships_placed = 0;
	bool FIsDragging  = false;
	TDrawGrid *FDraggedShip = nullptr;
	TPoint offset;
	std::map<TObject*, TPoint> FHomePositions;
	bool waiting_for_opponent = false; // чекаємо на суперника після "Старт"
	bool FindShipAt(int c, int r, int &outLen, int &outDir, int &outHeadC, int &outHeadR);
	void __fastcall AppMessage(tagMSG &Msg, bool &Handled);
	void RotateDraggedShip();

public:
	TBitmap *BmpBamSea;
	TBitmap *BmpBamShip;
	TBitmap *KillShip;
	__fastcall  TForm2(TComponent* Owner);
	__fastcall ~TForm2();

	Game_logic *player1, *player2;

	// Викликається з dmNetwork коли суперник натиснув "Старт"
	void OnOpponentReady();
	// Публічні методи малювання — використовуються дочірніми формами
	void DrawSeaCell(TCanvas* Canvas, const TRect& Rect);
	void DrawShipSprite(TCanvas* cvs, const TRect& Rect, int spriteIndex);

};

extern PACKAGE TForm2 *Form2;
#endif
