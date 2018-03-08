#include "TrashCan.h"

bool TrashCan::pointIside(Pt pos){
	if(collapsed){
		if(pos.x > buttonPos.x && pos.x < (buttonPos.x + buttonWidth)){
			if(pos.y > buttonPos.y && pos.y < (buttonPos.y + buttonHeight)){
				return true;
			}
		}
	}else{
		if(pos.x > regionPos.x && pos.x < (regionPos.x + regionWidth)){
			if(pos.y > regionPos.y && pos.y < (regionPos.y + regionHeight)){
				return true;
			}
		}
	}
	return false;
}


void TrashCan::initializeElements(){

	//place holder pointer
	std::string image_filename = "../source/applications/audio_interface/images/ppm/trash_closed.ppm";
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	Button *b = new Button(buttonPos, buttonWidth, buttonHeight, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("TrashCanClosed");
	b->SetRenderText(false);
	b->enable_texture();
	trashCanClosed = b;

	image_filename = "../source/applications/audio_interface/images/ppm/trash_open.ppm";
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	b = new Button(buttonPos, buttonWidth, buttonHeight, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("TrashCanOpen");
	b->SetRenderText(false);
	b->enable_texture();
	trashCanOpen = b;

}

void TrashCan::drawTracks(){
	std::list<Track*>::iterator itr;
	for(itr = tracks.begin(); itr != tracks.end(); ++itr){
		(*itr)->draw();
	}
}

void TrashCan::drawBorder(){

	if(collapsed){

		trashCanClosed->paint();

	}else{
		//outer border lines
		glLineWidth(2.0);
		glBegin(GL_LINE_LOOP);
			
			glColor3f(1.0,1.0,1.0);
			glVertex2f(regionPos.x, regionPos.y);
			glVertex2f(regionPos.x, regionPos.y+regionHeight);
			glVertex2f(regionPos.x+regionWidth, regionPos.y+regionHeight);
			glVertex2f(regionPos.x+regionWidth, regionPos.y);
			glVertex2f(regionPos.x, regionPos.y);
			
		glEnd();

		trashCanOpen->paint();

	}

}

void TrashCan::verticallyArrangeTracks(){
	double trackGap = 8;
	if(tracks.size() >= trackGap){
		trackGap = tracks.size();
	}

	std::list<Track*>::iterator itr;
	int i = 0;
	for(itr = tracks.begin(); itr != tracks.end(); ++itr){
		Pt newPos = Pt(regionPos.x + regionWidth/6, 
			(regionPos.y + regionHeight/2) - (((tracks.size()-1) * regionHeight/trackGap)/2) + (i * regionHeight/trackGap));
		(*itr)->moveAndResize(newPos, regionWidth*2/3, regionHeight);
		i++;
	}
}

void* TrashCan::tryToPressObj(Cursor *c, TrackRegion *TR, Bank *bank, Controls *ctrl){


	if(collapsed){
		//trash can button
		if(trashCanClosed->PointInside(c->getScreenPosition())){
			TR->moveAndResize(800, 700, Pt(150,350));
			bank->moveAndResize(475, 106, Pt(475,150));
			ctrl->moveAndResize(300, 106, Pt(475-(475-150),150));
			collapsed = false;
			return NULL;
		}
	//not collapsed
	}else{
		//trash can button
		if(trashCanOpen->PointInside(c->getScreenPosition())){
			TR->moveAndResize(1350, 900, Pt(250,250));
			bank->moveAndResize(800, 180, Pt(800,40));
			ctrl->moveAndResize(500, 180, Pt(250,40));
			//drawBorder();
			collapsed = true;
			return NULL;
		}

		//void* TrackRegion::tryToPressObj(Cursor* c){
		//std::vector<PolyButton*> tEffects;
		PolyButton *objPtr = NULL;

		//normal tracks=================================================
		std::list<Track*>::iterator itr;
		for(itr = tracks.begin(); itr != tracks.end(); ++itr){
			objPtr = (*itr)->getTrackButton()->tryToPressPolyButton(c);
			if(objPtr != NULL){
				c->setGrabbedVoidObj(objPtr);
				std::string objType = "TrashTrackButton";
				c->setObjType(objType);
				return objPtr;
			}
		}
		return NULL;
	}

    // compiler warning, must return something
	//shouldn't get here
	return NULL;
}
void TrashCan::tryToReleaseObj(Cursor *c, void *objPtr){
	//((Button*)objPtr)->release();
	c->setGrabbedVoidObj(NULL);
	std::string nullType = "NULL";
	c->setObjType(nullType);

	std::list<Track*>::iterator itr;
	for(itr = tracks.begin(); itr != tracks.end(); ++itr){
		Track *t = (Track*)((PolyButton*)objPtr)->getTrack();
		if((*itr) == t){
			tracks.erase(itr);	
			verticallyArrangeTracks();
			return;
		}
	}
}

void TrashCan::tryToMoveObj(Cursor *c, void *objPtr, std::string &objType){

	if(objType == "TrashTrackButton" && c->isStateDown()){
		Track *t = (Track*)((PolyButton*)objPtr)->getTrack();
		t->tryToMoveTrack(c->getScreenPosition());
	}
}

void TrashCan::slideBack(Cursor* c, void* objPtr, std::string &objType){

	if(objType == "TrashTrackButton"){
		Track *t = (Track*)((PolyButton*)objPtr)->getTrack();
		t->setDashed(false);
		((PolyButton*)objPtr)->slideBackToPosition();
	}
	c->setGrabbedVoidObj(NULL);
	std::string nullType = "NULL";
	c->setObjType(nullType);

}

void TrashCan::moveAndResize(double w, double h, Pt pos){

	// BW = w;
	// BH = h;
	// BPos = pos;

	// //Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
	// //										int HEIGHT, int WIDTH, bool FADES)
	// titleW = BW/10.0;
	// titleH = BH/5.0;
	// delete TrashCanTitle;
	// TrashCanTitle = new FadeLabel(Pt(BPos.x + titleW/2.0, BPos.y + BH), Vec3f(1.0,1.0,1.0), "Trash", titleH, titleW, false);


	// double elemW;
	// double elemH;
	// if(BW/8.0 < BH*(4.0/5.0)){
	// 	elemW = BW/8.0;
	// 	elemH = BW/8.0;
	// }else{
	// 	elemW = BH*(4.0/5.0);
	// 	elemH = BH*(4.0/5.0);
	// }


	// Pt elemPos = Pt(BPos.x + BW/10.0 + elemW/2, BPos.y + BH/2.0);


	// for(unsigned int i=0; i<elements.size(); i++){
	// 	elements[i]->MoveNoDamping(elemPos);
	// 	elements[i]->setDimensions(elemW, elemH);

	// 	elemPos.x += BW/10.0 + elemW;
	// }

}


// std::string TrashCan::getButtonName(void *objPtr){

// 	return ((Button*)objPtr)->getText(0);
// }

// double TrashCan::getButtonHeight(void *objPtr){
	
// 	return ((Button*)objPtr)->getHeight();
// }

