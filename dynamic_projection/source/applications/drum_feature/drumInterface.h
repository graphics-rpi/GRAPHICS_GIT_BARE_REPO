#ifndef _DRUMINTERFACE_H_
#define _DRUMINTERFACE_H_

#include <string>
#include <vector>
#include <iostream>
#include "../../multi_cursor/interaction.h"
#include "../paint/button.h"
#include "myLabel.h"

class drumInterface{
public:

	//Constructor drumInterface(Pt POSITION, std::string ID, double WIDTH, double HEIGHT)
	drumInterface(Pt pos, std::string i, double w, double h, double f, std::vector<std::string> &vn, Pt r){
		center = pos;
		id = i;
		width = w;
		height = h;
		fdh = f;
		vertNames = vn;
		assignPentVerts();
		volRange = r;

		button = new Button(Pt((center.x-width/22),(center.y-height/22)), width/11, height/11, Vec3f(1.0,1.0,1.0), id.c_str());
		button->setCircle(true);
		button->SetRenderText(false);
		colors.push_back(Vec3f(1,0.3,0.3));
		colors.push_back(Vec3f(0.3,1,0.3));
		colors.push_back(Vec3f(0.4,0.4,1));
		colors.push_back(Vec3f(1.0,0.74,0.2));
		colors.push_back(Vec3f(0.82,0.43,0.92));
		colors.push_back(Vec3f(1,0.3,0.3));
		colors.push_back(Vec3f(0.3,1,0.3));
		colors.push_back(Vec3f(0.4,0.4,1));
		colors.push_back(Vec3f(1.0,0.74,0.2));
		colors.push_back(Vec3f(0.82,0.43,0.92));

		for(unsigned int i=0; i<colors.size(); i++){
			volumes.push_back(0);
		}
		
		backgroundColor = Vec3f(0.15,0.15,0.15);
		
	}
	
	Pt getCenter(){return center;}
	std::string getId(){return id;}
	int getWidth(){return width;}
	int getHeight(){return height;}
	Button* getButton(){return button;}
	Vec3f getBackgroundColor(){return backgroundColor;}
	std::vector<int> getVolumeVec(){return volumes;}
	
	void setBackgroundColor(Vec3f c){backgroundColor = c;}
	void assignPentVerts();
	void draw(){
		drawBorder();
		drawButton();
	}
	void drawBorder();
	void drawButton(){ button->paint(); }
	void updateParameterVec();
	
	bool isLegalMove(Pt m);
	

protected:
	Pt center;
	std::string id;
	double width;
	double height;
	double fdh;
	std::vector<Pt> verts;
	std::vector<std::string> vertNames;
	std::vector<Vec3f> colors;
	std::vector<int> volumes;
	Pt volRange;
	std::vector<myLabel> labels;
	Vec3f backgroundColor;
	
	Button *button;
};


#endif
