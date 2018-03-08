#include "Controls.h"

void Controls::initializeElements(){


	double elemW;
	double elemH;
	if(CW/5.0 < CH*(4.0/5.0)){
		elemW = CW/5.0;
		elemH = CW/5.0;
	}else{
		elemW = CH*(4.0/5.0);
		elemH = CH*(4.0/5.0);
	}
	Pt elemPos = Pt(CPos.x + elemW*1.5, CPos.y + CH/2.0 - elemH/2.0);

	PolyButton *pb; //holder
	std::string image_filename = "../source/applications/audio_interface/images/ppm/metronome.ppm";
	//DISTORTION polybutton initialization
	std::vector<std::string> distVertNames;
	distVertNames.push_back("Faster");
	distVertNames.push_back("Slower");
	//PolyButton(Pt pos, std::string bText, double w, double h, double f, std::vector<std::string> &vn, Pt r, Vec3f color, std::string texture){
	pb = new PolyButton(Pt(elemPos.x, elemPos.y+elemH/2),"Tempo", 
		elemW*5, elemH*5, CH, distVertNames, Pt(57,115), Vec3f(0.3,0.3,0.3), image_filename);
	//effects.push_back(pb);
	tempoPolyButton = pb;

	elemPos.x += elemW*1.5;

	//place holder pointer
	image_filename = "../source/applications/audio_interface/images/ppm/play.ppm";
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("Track");
	b->SetRenderText(false);
	b->enable_texture();
	//elements.push_back(b);
	playButton = b;

	//place holder pointer
	image_filename = "../source/applications/audio_interface/images/ppm/stop.ppm";
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	b->addText("Track");
	b->SetRenderText(false);
	b->enable_texture();
	stopButton = b;



}

void Controls::drawBorder(){

	//outer border lines
	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
		
		glColor3f(1.0,1.0,1.0);
		glVertex2f(CPos.x, CPos.y);
		glVertex2f(CPos.x, CPos.y+CH);
		glVertex2f(CPos.x+CW, CPos.y+CH);
		glVertex2f(CPos.x+CW, CPos.y);
		glVertex2f(CPos.x, CPos.y);
		
	glEnd();

	//Cut out top left part of the border for the title
	glLineWidth(2.0);
	glBegin(GL_LINES);
	//Pt(CPos.x, CPos.y + CH)
	//Pt(CPos.x + titleW, CPos.y + CH)
		
		glColor3f(0.0,0.0,0.0);
		glVertex2f(CPos.x, CPos.y + CH);
		glVertex2f(CPos.x + titleW, CPos.y + CH);
		
	glEnd();

	controlsTitle->render(CH);

}
void Controls::drawElements(){

	if(!isPlaying){
		playButton->paint();
	}else{
		stopButton->paint();
	}
	tempoPolyButton->drawPB();

	// for(unsigned int i=0; i<elements.size(); i++){
	// 	elements[i]->paint();
	// }

	// std::list<Button*>::iterator itr;
	// for(itr = ghostElements.begin(); itr != ghostElements.end(); ++itr){
	// 	(*itr)->paint();
	// }

}

void* Controls::tryToPressObj(Cursor *c){

	if(!isPlaying){
		if(playButton->PointInside(c->getScreenPosition()) && !playButton->isPressed()){
			playButton->press(c->getColor(), c->getId(), c->getScreenPosition());
			std::string objType = "PlayButton";
	  		c->setObjType(objType);
	  		c->setGrabbedVoidObj(playButton);
	  		isPlaying = true;
			return playButton;
		}
	}else{
		if(stopButton->PointInside(c->getScreenPosition()) && !stopButton->isPressed()){
			stopButton->press(c->getColor(), c->getId(), c->getScreenPosition());
			std::string objType = "StopButton";
	  		c->setObjType(objType);
	  		c->setGrabbedVoidObj(stopButton);
	  		isPlaying = false;
			return stopButton;
		}
	}

	void* objPtr = tempoPolyButton->tryToPressPolyButton(c);
	if(objPtr != NULL){
		return objPtr;
	}
	return NULL;
		
}
void Controls::tryToReleaseObj(Cursor *c, std::string objType){

	if(objType == "PlayButton" || objType == "StopButton"){

		//dynamic_cast<Button*>(c->getGrabbedVoidObj())->release();
		((Button*)(c->getGrabbedVoidObj()))->release();
		std::string nullType = "NULL";
		c->setGrabbedVoidObj(NULL);
		c->setObjType(nullType);
		//delete((Button*)objPtr);
		return;
	}


}

// void Controls::tryToMoveObj(Cursor *c, void *objPtr){

// 	// ((Button*)objPtr)->Move(c->getPosition());
// }


void Controls::moveAndResize(double w, double h, Pt pos){


	CW = w;
	CH = h;
	CPos = pos;

	titleW = CW/3;
	titleH = CH/5;
	delete controlsTitle;
	controlsTitle = new FadeLabel(Pt(CPos.x + titleW/2, CPos.y + CH), Vec3f(1.0,1.0,1.0), "CONTROLS", titleH, titleW, false);

	double elemW;
	double elemH;
	if(CW/5.0 < CH*(4.0/5.0)){
		elemW = CW/5.0;
		elemH = CW/5.0;
	}else{
		elemW = CH*(4.0/5.0);
		elemH = CH*(4.0/5.0);
	}
	Pt elemPos = Pt(CPos.x + elemW*1.5, CPos.y + CH/2.0 - elemH/2.0);


	//Pt(elemPos.x, elemPos.y+elemH/2),"Tempo",elemW*5, elemH*5
	tempoPolyButton->moveAndResize(Pt(elemPos.x, elemPos.y+elemH/2), elemW*5, elemH*5);
	elemPos.x += elemW*1.5;

	playButton->MoveNoDamping(Pt(elemPos.x+elemW/2, elemPos.y+elemH/2));
	playButton->setDimensions(elemW, elemH);

	stopButton->MoveNoDamping(Pt(elemPos.x+elemW/2, elemPos.y+elemH/2));
	stopButton->setDimensions(elemW, elemH);

}


// double Controls::getButtonHeight(void *objPtr){
	
// 	// return ((Button*)objPtr)->getHeight();
// }
