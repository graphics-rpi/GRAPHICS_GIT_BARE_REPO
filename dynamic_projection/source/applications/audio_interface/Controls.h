#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <string>
#include <vector>
#include <list>
#include <iostream>
//#include "../../multi_mouse/interaction.h"
#include "../paint/button.h"
//#include "../volume_control/MySlider.h"
#include "FadeLabel.h"
#include "PolyButton.h"

class Controls {
public:

	Controls(double width, double height, Pt pos){
		CW = width;
		CH = height;
		CPos = pos;
		isPlaying = false;

		//Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
		//										int HEIGHT, int WIDTH, bool FADES)
		titleW = CW/3;
		titleH = CH/5;
		controlsTitle = new FadeLabel(Pt(CPos.x + titleW/2, CPos.y + CH), Vec3f(1.0,1.0,1.0), "CONTROLS", titleH, titleW, false);

		  // MySlider(const std::string &n, double val, const Vec3f &c, 
  		// 	const std::string &filename, double bw, double bh, double bx, double by,
  		// 	double miny, double maxy) {
		//tempoSlider = new MySlider("Tempo", )
	}

	void draw(){
		drawBorder();
		drawElements();
	}

	void* tryToPressObj(Cursor *c);
	void tryToReleaseObj(Cursor *c, void *objPtr);
	void tryToReleaseObj(Cursor *c, std::string objType);
	void tryToMoveObj(Cursor *c, void *objPtr);

	void moveAndResize(double w, double h, Pt pos);

	void initializeElements();

	std::string getButtonName(void *objPtr);
	double getButtonHeight(void *objPtr);
	bool getIsPlaying(){ return isPlaying; }
	void setIsPlaying(bool i){ isPlaying = i; }


protected:

	//protected methods
	void drawBorder();
	void drawElements();

	double CW;
	double CH;
	double titleW;
	double titleH;
	Pt CPos;

	bool isPlaying;
	Button* playButton;
	Button* stopButton;
	PolyButton* tempoPolyButton;
	//std::vector<Button*> elements;
	//std::list<Button*> ghostElements;
	FadeLabel* controlsTitle;
	//MySlider* tempoSlider;

};

#endif
