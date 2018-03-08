#include "RecordDialog.h"

void RecordDialog::initializeElements(){


	double elemW;
	double elemH;
	if(RW/5.0 < RH*(4.0/5.0)){
		elemW = RW/5.0;
		elemH = RW/5.0;
	}else{
		elemW = RH*(4.0/5.0);
		elemH = RH*(4.0/5.0);
	}
	Pt elemPos = Pt(RPos.x + RW/5.0, RPos.y + RH/2.0 - elemH/2.0);

	//place holder pointer
	std::string image_filename = "../source/applications/audio_interface/images/ppm/record.ppm";
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("Track");
	b->SetRenderText(false);
	b->enable_texture();
	RecButton = b;

	elemPos.x += RW/5.0 + elemW;
	//place holder pointer
	image_filename.clear();
	image_filename.assign("../source/applications/audio_interface/images/ppm/close.ppm");
	//std::string image_filename = "../source/applications/volume_control/guitar.ppm";
	b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3), image_filename, image_filename);
	//Button *b = new Button(Pt(elemPos.x,elemPos.y), elemW, elemH, Vec3f(0.3,0.3,0.3),"Track");
	//b->load_textures(image_filename.c_str(), image_filename.c_str());
	b->addText("Dist");
	b->SetRenderText(false);
	b->enable_texture();
	CancelButton = b;


}

void RecordDialog::drawBorder(){

	//outer border lines
	
	glBegin(GL_TRIANGLES);
		glColor3f(0.0,0.0,0.0);
		glVertex2f(RPos.x, RPos.y);
		glVertex2f(RPos.x, RPos.y+RH);
		glVertex2f(RPos.x+RW, RPos.y+RH);

		glVertex2f(RPos.x+RW, RPos.y+RH);
		glVertex2f(RPos.x+RW, RPos.y);
		glVertex2f(RPos.x, RPos.y);

	glEnd();


	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
		
		glColor3f(1.0,1.0,1.0);
		glVertex2f(RPos.x, RPos.y);
		glVertex2f(RPos.x, RPos.y+RH);
		glVertex2f(RPos.x+RW, RPos.y+RH);
		glVertex2f(RPos.x+RW, RPos.y);
		glVertex2f(RPos.x, RPos.y);
		
	glEnd();


	//Cut out top left part of the border for the title
	glLineWidth(2.0);
	glBegin(GL_LINES);
	//Pt(RPos.x, RPos.y + RH)
	//Pt(RPos.x + titleW, RPos.y + RH)
		
		glColor3f(0.0,0.0,0.0);
		glVertex2f(RPos.x, RPos.y + RH);
		glVertex2f(RPos.x + titleW, RPos.y + RH);
		
	glEnd();

	RecordDialogTitle->render(RH);

}
void RecordDialog::drawElements(){

	RecButton->paint();
	CancelButton->paint();

}

void* RecordDialog::tryToPressObj(Cursor *c){

	if(RecButton->PointInside(c->getScreenPosition()) && !RecButton->isPressed()){
		RecButton->press(c->getColor(), c->getId(), c->getScreenPosition());
		c->setGrabbedVoidObj(RecButton);
		std::string strType = "RecordButton";
		c->setObjType(strType);
		return RecButton;
	}else if(CancelButton->PointInside(c->getScreenPosition()) && !CancelButton->isPressed()){
		CancelButton->press(c->getColor(), c->getId(), c->getScreenPosition());
		c->setGrabbedVoidObj(CancelButton);
		std::string strType = "CancelButton";
		c->setObjType(strType);
		return CancelButton;
	}else{
		c->setGrabbedVoidObj(NULL);
		std::string strType = "NULL";
		c->setObjType(strType);
		return NULL;
	}

}

void RecordDialog::drawRecordBar(double percent, double seconds){
	assert(percent <= 1.0);

	drawBorder();
	double newWidth = RW * percent;
	double newHeight = (RH * 2)/3;

	glBegin(GL_TRIANGLES);
		glColor3f(1.0,1.0,1.0);
		glVertex2f(RPos.x, RPos.y);
		glVertex2f(RPos.x, RPos.y+newHeight);
		glVertex2f(RPos.x+newWidth, RPos.y+newHeight);

		glVertex2f(RPos.x+newWidth, RPos.y+newHeight);
		glVertex2f(RPos.x+newWidth, RPos.y);
		glVertex2f(RPos.x, RPos.y);
	glEnd();

	std::stringstream ss;
	seconds -= 1.0;
	double textNum = abs(seconds - totalRecTime);
	if(textNum >= totalRecTime){
		textNum = totalRecTime;
	}
	ss << (int)textNum;
	std::string text = ss.str();
	//if(text == "31"){ text = "30"; }
	currentTime->setText(text);
	currentTime->render(RH);

}

bool RecordDialog::tryToReleaseObj(Cursor* c, void* objPtr, std::string& objType){

	((Button*)objPtr)->release();
	c->setGrabbedVoidObj(NULL);
	std::string strType = "NULL";
	c->setObjType(strType);

	if(objType == "RecordButton"){
		recording = true;
		inDialog = true;
		return true;
	}else if(objType == "CancelButton"){
		inDialog = false;
		return false;
	}
	// COMPILER WARNING, must return something
	return false;
}


void RecordDialog::moveAndResize(double w, double h, Pt pos){

	// RW = w;
	// RH = h;
	// RPos = pos;

	// //Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
	// //										int HEIGHT, int WIDTH, bool FADES)
	// titleW = RW/10.0;
	// titleH = RH/5.0;
	// delete RecordDialogTitle;
	// RecordDialogTitle = new FadeLabel(Pt(RPos.x + titleW/2.0, RPos.y + RH), Vec3f(1.0,1.0,1.0), "RecordDialog", titleH, titleW, false);


	// double elemW;
	// double elemH;
	// if(RW/8.0 < RH*(4.0/5.0)){
	// 	elemW = RW/8.0;
	// 	elemH = RW/8.0;
	// }else{
	// 	elemW = RH*(4.0/5.0);
	// 	elemH = RH*(4.0/5.0);
	// }


	// Pt elemPos = Pt(RPos.x + RW/10.0 + elemW/2, RPos.y + RH/2.0);


	// for(unsigned int i=0; i<elements.size(); i++){
	// 	elements[i]->MoveNoDamping(elemPos);
	// 	elements[i]->setDimensions(elemW, elemH);

	// 	elemPos.x += RW/10.0 + elemW;
	// }



}
