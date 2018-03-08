#ifndef _TRASHCAN_H_
#define _TRASHCAN_H_

#include <string>
#include <vector>
#include <list>
#include <iostream>
//#include "../../multi_mouse/interaction.h"
#include "../paint/button.h"
#include "FadeLabel.h"
#include "TrackRegion.h"
#include "Bank.h"
#include "Controls.h"

class TrashCan {
public:

	TrashCan(double bwidth, double bheight, Pt bpos,
			 double rwidth, double rheight, Pt rpos){
		buttonWidth = bwidth;
		buttonHeight = bheight;
		buttonPos = bpos;

		regionWidth = rwidth;
		regionHeight = rheight;
		regionPos = rpos;

		collapsed = true;

		//Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
		//										int HEIGHT, int WIDTH, bool FADES)
		//titleW = buttonWidth/10;
		//titleH = buttonHeight/5;
		//trashCanTitle = new FadeLabel(Pt(buttonPos.x + titleW/2, buttonPos.y + buttonHeight), Vec3f(1.0,1.0,1.0), "Trash", titleH, titleW, false);

	}

	void draw(){
		drawBorder();
		if(!collapsed){
			drawTracks();
		}
	}

	void verticallyArrangeTracks();
	void* tryToPressObj(Cursor *c, TrackRegion *TR, Bank *bank, Controls *ctrl);
	void tryToReleaseObj(Cursor *c, void *objPtr);
	void tryToMoveObj(Cursor *c, void *objPtr, std::string &objType);
	void slideBack(Cursor* c, void* objPtr, std::string &objType);


	void addTrack(Track* t){ 
		t->setButtonCollapsed(true);
		t->setConnected(true);
		t->collapseEffects();
		tracks.push_back(t); 

		verticallyArrangeTracks();
	}
	bool pointIside(Pt pos);
	void moveAndResize(double w, double h, Pt pos);
	void initializeElements();

	// std::string getButtonName(void *objPtr);
	// double getButtonHeight(void *objPtr);


protected:

	//protected methods
	void drawBorder();
	void drawTracks();


	bool collapsed;
	double buttonWidth;
	double buttonHeight;
	double regionWidth;
	double regionHeight;
	Pt buttonPos;
	Pt regionPos;

	Button *trashCanOpen;
	Button *trashCanClosed;
	//std::vector<Button*> elements;
	std::list<Track*> tracks;
	//FadeLabel* trashCanTitle;

};

#endif
