#include "Bank.h"

void Bank::initializeElements(){


	double elemW;
	double elemH;
	if(BW/8.0 < BH*(4.0/5.0)){
		elemW = BW/8.0;
		elemH = BW/8.0;
	}else{
		elemW = BH*(4.0/5.0);
		elemH = BH*(4.0/5.0);
	}
	Pt elemPos = Pt(BPos.x + BW/10.0, BPos.y + BH/2.0 - elemH/2.0);

	//place holder pointer
	std::string image_filename = "../source/applications/audio_interface/images/ppm/newTrack_texture.ppm";
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("Track");
	b->SetRenderText(false);
	b->enable_texture();
	elements.push_back(b);

	elemPos.x += BW/10.0 + elemW;
	//place holder pointer
	image_filename.clear();
	image_filename.assign("../source/applications/audio_interface/images/ppm/dist_texture.ppm");
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("Dist");
	b->SetRenderText(false);
	b->enable_texture();
	elements.push_back(b);

	// b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), "Dist.");
	// elements.push_back(b);

	elemPos.x += BW/10.0 + elemW;
	image_filename.clear();
	image_filename.assign("../source/applications/audio_interface/images/ppm/mod_texture.ppm");
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("Mod");
	b->SetRenderText(false);
	b->enable_texture();
	elements.push_back(b);
	// b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), "Mod");
	// elements.push_back(b);

	elemPos.x += BW/10.0 + elemW;
	image_filename.clear();
	image_filename.assign("../source/applications/audio_interface/images/ppm/eq_texture.ppm");
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("EQ");
	b->SetRenderText(false);
	b->enable_texture();
	elements.push_back(b);
	// b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), "EQ");
	// elements.push_back(b);

}

void Bank::drawBorder(){

	//outer border lines
	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
		
		glColor3f(1.0,1.0,1.0);
		glVertex2f(BPos.x, BPos.y);
		glVertex2f(BPos.x, BPos.y+BH);
		glVertex2f(BPos.x+BW, BPos.y+BH);
		glVertex2f(BPos.x+BW, BPos.y);
		glVertex2f(BPos.x, BPos.y);
		
	glEnd();

	//Cut out top left part of the border for the title
	glLineWidth(2.0);
	glBegin(GL_LINES);
	//Pt(BPos.x, BPos.y + BH)
	//Pt(BPos.x + titleW, BPos.y + BH)
		
		glColor3f(0.0,0.0,0.0);
		glVertex2f(BPos.x, BPos.y + BH);
		glVertex2f(BPos.x + titleW, BPos.y + BH);
		
	glEnd();

	bankTitle->render(BH);

}
void Bank::drawElements(){

	for(unsigned int i=0; i<elements.size(); i++){
		if(i != 0 || canAddTrack){
			elements[i]->paint();
		}
	}

	std::list<Button*>::iterator itr;
	for(itr = ghostElements.begin(); itr != ghostElements.end(); ++itr){
		(*itr)->paint();
	}

}

void* Bank::tryToPressObj(Cursor *c){

	//seach all elements and try to press one of them
	for(unsigned int i=0; i<elements.size(); i++){
		if(elements[i]->PointInside(c->getScreenPosition()) && !elements[i]->isPressed()){

			// Vec3f ghostColor = Vec3f(elements[i]->getColor().x()/2,
			// 						elements[i]->getColor().y()/2,
			// 						elements[i]->getColor().z()/2);

			std::string image_filename;
			Button* ghostButton;

			if(elements[i]->getText(0) == "Track" && canAddTrack){
				image_filename = "../source/applications/audio_interface/images/ppm/newTrack_texture.ppm";

				ghostButton = new Button(elements[i]->getLowerLeftCorner(), elements[i]->getWidth(), 
					elements[i]->getHeight(), elements[i]->getColor(), image_filename, image_filename );
				ghostButton->enable_texture();
				ghostButton->addText(elements[i]->getText(0));
				ghostButton->SetRenderText(false);
			}else if(elements[i]->getText(0) == "Dist"){
				image_filename = "../source/applications/audio_interface/images/ppm/dist_texture.ppm";

				ghostButton = new Button(elements[i]->getLowerLeftCorner(), elements[i]->getWidth(), 
					elements[i]->getHeight(), elements[i]->getColor(), image_filename, image_filename );
				ghostButton->enable_texture();
				ghostButton->addText(elements[i]->getText(0));
				ghostButton->SetRenderText(false);
			}else if(elements[i]->getText(0) == "Mod"){
				image_filename = "../source/applications/audio_interface/images/ppm/mod_texture.ppm";

				ghostButton = new Button(elements[i]->getLowerLeftCorner(), elements[i]->getWidth(), 
					elements[i]->getHeight(), elements[i]->getColor(), image_filename, image_filename );
				ghostButton->enable_texture();
				ghostButton->addText(elements[i]->getText(0));
				ghostButton->SetRenderText(false);
			}else if(elements[i]->getText(0) == "EQ"){
				image_filename = "../source/applications/audio_interface/images/ppm/eq_texture.ppm";

				ghostButton = new Button(elements[i]->getLowerLeftCorner(), elements[i]->getWidth(), 
					elements[i]->getHeight(), elements[i]->getColor(), image_filename, image_filename );
				ghostButton->enable_texture();
				ghostButton->addText(elements[i]->getText(0));
				ghostButton->SetRenderText(false);
			}else{
				ghostButton = new Button(elements[i]->getLowerLeftCorner(), elements[i]->getWidth(), 
					elements[i]->getHeight(), elements[i]->getColor(), elements[i]->getText(0));
				
			}

			if(elements[i]->getText(0) != "Track" || canAddTrack){
				ghostElements.push_back(ghostButton);
				ghostButton->press(c->getColor(), c->getId(), c->getScreenPosition());
				c->setGrabbedVoidObj(ghostButton);
	  			std::string objType = "BankButton";
	  			c->setObjType(objType);
				return ghostButton;
			}
		}
	}
	//if nothing was clicked
	c->setGrabbedVoidObj(NULL);
	std::string objType = "NULL";
  	c->setObjType(objType);
	return NULL;
}
void Bank::tryToReleaseObj(Cursor *c, void *objPtr){

	((Button*)objPtr)->release();
	std::list<Button*>::iterator itr;
	for(itr = ghostElements.begin(); itr != ghostElements.end(); ++itr){
		if((*itr) == objPtr){
			ghostElements.erase(itr);
			c->setGrabbedVoidObj(NULL);
			std::string nullType = "NULL";
			c->setObjType(nullType);
			delete((Button*)objPtr);
			return;
		}
	}


}

void Bank::tryToMoveObj(Cursor *c, void *objPtr){

	((Button*)objPtr)->Move(c->getScreenPosition());
}

std::string Bank::getButtonName(void *objPtr){

	std::cout << "numTextStrings = " << ((Button*)objPtr)->numTextStrings() << std::endl;
	assert(((Button*)objPtr)->numTextStrings() == 1);
	return ((Button*)objPtr)->getText(0);
}

void Bank::moveAndResize(double w, double h, Pt pos){

	BW = w;
	BH = h;
	BPos = pos;

	//Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
	//										int HEIGHT, int WIDTH, bool FADES)
	titleW = BW/10.0;
	titleH = BH/5.0;
	delete bankTitle;
	bankTitle = new FadeLabel(Pt(BPos.x + titleW/2.0, BPos.y + BH), Vec3f(1.0,1.0,1.0), "BANK", titleH, titleW, false);


	double elemW;
	double elemH;
	if(BW/8.0 < BH*(4.0/5.0)){
		elemW = BW/8.0;
		elemH = BW/8.0;
	}else{
		elemW = BH*(4.0/5.0);
		elemH = BH*(4.0/5.0);
	}


	Pt elemPos = Pt(BPos.x + BW/10.0 + elemW/2, BPos.y + BH/2.0);


	for(unsigned int i=0; i<elements.size(); i++){
		elements[i]->MoveNoDamping(elemPos);
		elements[i]->setDimensions(elemW, elemH);

		elemPos.x += BW/10.0 + elemW;
	}



}


double Bank::getButtonHeight(void *objPtr){
	
	return ((Button*)objPtr)->getHeight();
}
