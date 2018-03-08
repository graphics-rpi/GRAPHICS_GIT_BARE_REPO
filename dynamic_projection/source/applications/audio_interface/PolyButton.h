#ifndef _POLYBUTTON_H_
#define _POLYBUTTON_H_

#include <string>
#include <vector>
#include <iostream>
#include <pthread.h>
#include "PolyInterface.h"


class PolyButton {
//: public Button {
public:

	//Constructor PolyButton(PolyInterface ARGS, Vec3f BUTTONCOLOR)
	PolyButton(Pt pos, std::string bText, double w, double h, double f, std::vector<std::string> &vn, Pt r, Vec3f color, std::string texture)
	     {
	     //: Button ( pos, w, h, color, texture ) {

		position = pos;
		buttonColor = color;
		buttonText = bText;
		buttonTexture = texture;
		polyWidth = w;
		polyHeight = h;
		buttonWidth = w/4;
		buttonHeight = w/4;
		collapsed = true;
		connected = true;
		track = NULL;

		if(vn.size() == 2){
			//myPolyInterface = new PolyInterface(position, buttonText, buttonWidth/3, buttonHeight/3, f, vn, r);
			polyWidth /= 3;
			polyHeight /= 3;
		}
		
		myPolyInterface = new PolyInterface(position, buttonText, polyWidth, polyHeight, f, vn, r);
		collapsedButton = new Button(position, buttonWidth, buttonHeight, buttonColor, buttonTexture.c_str(), buttonTexture.c_str());
		collapsedButton->MoveNoDamping(position);
		collapsedButton->addText(buttonText);
		collapsedButton->SetRenderText(false);
		collapsedButton->enable_texture();
		//collapsedButton->SetRenderText(true);
		
	}

	void tryToExpandPolyButton(Cursor* c);
	void slideBackToPosition();
	PolyButton* tryToPressPolyButton(Cursor* c);
	void tryToMovePolyButton(Pt pos);

	
	//Pt getCenter(){return center;}
	
	//std::string getId(){return id;}
	//int getWidth(){return width;}
	//int getHeight(){return height;}
	//Button* getButton(){return button;}
	//std::vector<int> getVolumeVec(){return volumes;}
	//void assignPentVerts();
	Button* getCollapsedButton(){return collapsedButton;}
	PolyInterface* getPolyInterface(){return myPolyInterface;}
	Pt getPosition(){ return position; }
	std::string getText(){ return buttonText; }
	void* getTrack(){ return track; }
	bool getConnected(){ return connected; }

	
	bool isCollapsed(){ return collapsed; }
	void setCollapsed(bool c){ collapsed = c; }
	void setTrack(void *t){ track = t; }
	void setConnected(bool c){ connected = c; }

	void moveAndResize(Pt pos, double w, double h);

	void changePositions(Pt &pos){
		position = pos;
		collapsedButton->MoveNoDamping(pos);
		myPolyInterface->Move(pos);
		//myPolyInterface->assignPentVerts();
	}

	void moveCollapsedToOriginalPos(){
		collapsedButton->MoveNoDamping(position);
	}

	void drawPB(){
		if(collapsed){
			collapsedButton->paint();
		}else{
			myPolyInterface->draw();
		}
	}
	
	//bool isLegalMove(Pt m);
	

protected:
	
	bool collapsed;
	bool connected;
	Pt position;
	double buttonWidth;
	double buttonHeight;
	double polyWidth, polyHeight;


	std::string buttonText;
	std::string buttonTexture;
	Vec3f buttonColor;
	
	Button *collapsedButton;
	PolyInterface *myPolyInterface;
	//NULL unless there is a track associated with this polyButton
	void *track;
};

//void * gradualMovement(void * arg);
//must be implemented by app (should be the main drawing function)
void draw();


#endif
