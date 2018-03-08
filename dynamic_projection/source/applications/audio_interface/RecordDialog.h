#ifndef _RECORD_H_
#define _RECORD_H_

#include <string>
#include <vector>
#include <list>
#include <iostream>
//#include "../../multi_mouse/interaction.h"
#include "../paint/button.h"
#include "FadeLabel.h"

class RecordDialog {
public:

	RecordDialog(double width, double height, Pt pos){
		RW = width;
		RH = height;
		RPos = pos;

		//Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
		//										int HEIGHT, int WIDTH, bool FADES)
		titleW = RW/4;
		titleH = RH/7;
		RecordDialogTitle = new FadeLabel(Pt(RPos.x + titleW/2, RPos.y + RH), Vec3f(1.0,1.0,1.0), "RECORD", titleH, titleW, false);
		currentTime = new FadeLabel(Pt(RPos.x + RW/2, RPos.y + (RH*2)/3 + titleH), Vec3f(1.0,1.0,1.0), "00:00", titleH, titleW, false);
		recording = false;
		inDialog = false;
		totalRecTime = 15.0;
	}

	void draw(){
		drawBorder();
		drawElements();
	}

	void drawRecordBar(double percent, double seconds);

	void* tryToPressObj(Cursor *c);
	bool tryToReleaseObj(Cursor* c, void* objPtr, std::string& objType);

	void setRecording(bool r){ recording = r; }
	bool isRecording(){ return recording; }
	void setInDialog(bool d){ inDialog = d; }
	bool isInDialog(){ return inDialog; }
	double getRecTime(){ return totalRecTime; }

	void moveAndResize(double w, double h, Pt pos);

	void initializeElements();


protected:

	//protected methods
	void drawBorder();
	void drawElements();

	double RW;
	double RH;
	double titleW;
	double titleH;
	Pt RPos;
	bool recording, inDialog;
	double totalRecTime;
	//double percentRecorded;
	//std::vector<Button*> elements;
	Button* RecButton;
	Button* CancelButton;
	FadeLabel* RecordDialogTitle;
	FadeLabel* currentTime;

};

#endif
