#ifndef _POLYBUTTON_H_
#define _POLYBUTTON_H_

#include <string>
#include <vector>
#include <iostream>
#include "drumInterface.h"


class polyButton{
public:

	//Constructor polyButton(drumInterface ARGS, Vec3f BUTTONCOLOR)
	polyButton(Pt pos, std::string i, double w, double h, double f, std::vector<std::string> &vn, Pt r, Vec3f color){

		//drumInterface(Pt POSITION, std::string ID, double WIDTH, 
		//													double HEIGHT, double FDH, vector<string> & VERTNAMES)
		myDrum = new drumInterface(pos, i, w, h, f, vn, r);

		buttonColor = color;
		buttonText = i;
		collapsed = true;
		
		collapsedButton = new Button(Pt((pos.x-w/10),(pos.y-w/10)), w/5, h/5, buttonColor, i.c_str());
		collapsedButton->SetRenderText(true);
		
	}
	
	//Pt getCenter(){return center;}
	//std::string getId(){return id;}
	//int getWidth(){return width;}
	//int getHeight(){return height;}
	//Button* getButton(){return button;}
	//std::vector<int> getVolumeVec(){return volumes;}
	//void assignPentVerts();
	Button* getCollapsedButton(){return collapsedButton;}
	drumInterface* getDrumInterface(){return myDrum;}
	
	bool isCollapsed(){ return collapsed; }
	void setCollapsed(bool c){ collapsed = c; }
	
	void drawPB(){
		if(collapsed){
			collapsedButton->paint();
		}else{
			myDrum->draw();
		}
	}
	
	//bool isLegalMove(Pt m);
	

protected:
	
	bool collapsed;

	std::string buttonText;
	Vec3f buttonColor;
	
	Button *collapsedButton;
	drumInterface *myDrum;
};


#endif
