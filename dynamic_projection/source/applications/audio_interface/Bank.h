#ifndef _BANK_H_
#define _BANK_H_

#include <string>
#include <vector>
#include <list>
#include <iostream>
//#include "../../multi_mouse/interaction.h"
#include "../paint/button.h"
#include "FadeLabel.h"

class Bank {
public:

	Bank(double width, double height, Pt pos){
		BW = width;
		BH = height;
		BPos = pos;

		//Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
		//										int HEIGHT, int WIDTH, bool FADES)
		titleW = BW/10;
		titleH = BH/5;
		bankTitle = new FadeLabel(Pt(BPos.x + titleW/2, BPos.y + BH), Vec3f(1.0,1.0,1.0), "BANK", titleH, titleW, false);
		canAddTrack = true;
	}

	void draw(){
		drawBorder();
		drawElements();
	}

	void* tryToPressObj(Cursor *c);
	void tryToReleaseObj(Cursor *c, void *objPtr);
	void tryToMoveObj(Cursor *c, void *objPtr);
	bool getCanAddTrack(){ return canAddTrack; }
	void setCanAddTrack(bool c){ canAddTrack = c; }

	void moveAndResize(double w, double h, Pt pos);

	void initializeElements();

	std::string getButtonName(void *objPtr);
	double getButtonHeight(void *objPtr);


protected:

	//protected methods
	void drawBorder();
	void drawElements();

	double BW;
	double BH;
	double titleW;
	double titleH;
	Pt BPos;
	std::vector<Button*> elements;
	std::list<Button*> ghostElements;
	FadeLabel* bankTitle;
	bool canAddTrack;

};

#endif
