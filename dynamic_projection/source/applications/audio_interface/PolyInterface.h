#ifndef _POLYINTERFACE_H_
#define _POLYINTERFACE_H_

#include <string>
#include <vector>
#include <iostream>
#include "../../multi_cursor/interaction.h"
#include "../paint/button.h"
#include "FadeLabel.h"

class PolyInterface{
public:

	//Constructor PolyInterface(Pt POSITION, std::string ID, double WIDTH, double HEIGHT, vn VERTNAMES, Pt PARAMRANGE)
	PolyInterface(Pt pos, std::string i, double w, double h, double f, std::vector<std::string> &vn, Pt range){
		center = pos;
		id = i;
		width = w;
		height = h;
		fdh = f;
		vertNames = vn;
		assignPentVerts();
		paramValueRange = range;

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
			paramValues.push_back(paramValueRange.x);
		}
		
		backgroundColor = Vec3f(0.15,0.15,0.15);
		
	}
	
	Pt getCenter(){return center;}
	std::string getId(){return id;}
	int getWidth(){return width;}
	int getHeight(){return height;}
	Button* getButton(){return button;}
	Vec3f getBackgroundColor(){return backgroundColor;}
	std::vector<int> getParamValues(){return paramValues;}
	const std::vector<string>& getParamNames(){return vertNames;}
	int getNumVerts(){ return verts.size(); }
	
	void setBackgroundColor(Vec3f c){backgroundColor = c;}
	void assignPentVerts();
	void draw(){
		drawBorder();
		drawButton();
	}
	void drawBorder();
	void drawButton(){ button->paint(); }
	void updateParameterVec();

	void Resize(double w, double h);
	void Move(Pt &pos); 
	void MoveButton(const Pt &pos);

	
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
	std::vector<int> paramValues;
	Pt paramValueRange;
	std::vector<FadeLabel> labels;
	Vec3f backgroundColor;
	
	Button *button;
};


#endif
