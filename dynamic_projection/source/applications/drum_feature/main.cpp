#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>
#include <cmath>
#include <sys/time.h>

#include "../../common/directory_locking.h"
#include "../../../../remesher/src/vectors.h"

#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"

#include "argparser.h"
#include "../paint/button.h"
#include "../paint/path.h"
//#include "../../common/Image.h"
#include "drumInterface.h"
#include "polyButton.h"
#include "../../multi_cursor/interaction.h"
#include "../../multi_cursor/cursor.h"

//#include "pong.h"
//#include "../paint/text.h"

//for OSC library
#include "../../common/oscpack/osc/OscOutboundPacketStream.h"
#include "../../common/oscpack/ip/UdpSocket.h"

#define ADDRESS "128.213.17.125"
#define PORT 7000
#define OUTPUT_BUFFER_SIZE 1024

//#define IR_STATE_DIRECTORY                "../state/ir_tracking"
//#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"

#define MK_STATE_DIRECTORY                   "../state/mouse_and_keyboard"
#define MK_ACTION_FILENAME_TEMPLATE          "../state/mouse_and_keyboard/actions_XXX.txt"
//#define APPLICATIONS_STATE_DIRECTORY            "../state/applications/"

#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args;
//DirLock global_dirlock(IR_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;

drumInterface *drum;
drumInterface *reverb;
drumInterface *delay;
std::vector<polyButton*> polyButtons;
std::vector<Path> trackStrokes;
std::vector<myLabel> trackLabels;
std::vector< std::list<Pt> > strokePtLists;

int glutLoopNumber = 0;
//PointTracker *global_point_tracker;
//PongGame *game;

//Colors global_colors;
//std::vector<Button> buttons;

//double TILE_w;
//double TILE_h;

//Mouse Globals======= 
//DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);
DirLock global_mk_dirlock(MK_STATE_DIRECTORY);

//std::vector<std::pair<std::list<Pt>,Vec3f> > GLOBAL_strokes;

//Pt mouse_location;
//std::vector<Pt> positions;
//std::vector<Pt> correctpositions;

//vcDraw drawRoutine;

//std::vector<bool> isClicked;
// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

//void draw_strokes(bool button_strokes);
//void draw_buttons();

//void initialize_buttons();
//void check_for_button_press();
//void check_for_button_motion();

//for mice=============
//void draw_cursors();

//void TryToPressButton(int id, double x, double y);
//void TryToReleaseButton(int id, double x, double y);
//void TryToMoveButton(int id, double x, double y);

void keyfunc(unsigned char key, int x, int y);
void specialkeyfunc(int key, int x, int y);		
void mousefunc(int button, int state, int x, int y);
void motionfunc(int x, int y);

//void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers);
//void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers);		 
//void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers);
//void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers);

/*void clamp_to_display(Pt &pt) { 
  int x = pt.x;
  int y = pt.y;
  x = std::max(0,std::min(x,args->tiled_display.full_display_width));
  y = std::max(0,std::min(y,args->tiled_display.full_display_height));
  pt = Pt(x,y);
}*/
//=====================

int HandleGLError(std::string foo) {
  GLenum error;
  int i = 0;
  while ((error = glGetError()) != GL_NO_ERROR) {
    printf ("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
    std::cout << foo << std::endl;
    if (error != GL_INVALID_OPERATION) i++;
  }
  if (i == 0) return 1;
  return 0;
}

//std::vector<MySlider> GLOBAL_volume_sliders;
//std::vector<MySlider> GLOBAL_EQ_sliders;

// ==========================================================================================
// DRAWING ROUTINES
// ==========================================================================================

//Hard coded draw routine for tracks
//for use in EMPAC CCC demo - modify later as a class implementation
void initializeTracks(double fdw, double fdh){
	//  Path(const std::list<Pt> &t, const Vec3f &c, int w) {
	//Constructor myLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
	//										int HEIGHT, int WIDTH, bool FADES)
	std::list<Pt> slist;
	slist.push_back(Pt(fdw*(1/12.0),fdh*(3/4.0)));
	slist.push_back(Pt(fdw*(6/12.0),fdh*(3/4.0)));
	slist.push_back(Pt(fdw*(6/12.0),fdh*(1/2.0)));
	slist.push_back(Pt(fdw*(11/12.0),fdh*(1/2.0)));
	trackStrokes.push_back(Path(slist, Vec3f(0.8,0.3,0.0), 10));
	trackLabels.push_back(myLabel(Pt(fdw*(1/12.0)+fdw/40.0,fdh*(3/4.0)+fdh/40.0), Vec3f(1.0,1.0,1.0),
												"Violin", (int)fdh/20.0, (int)fdh/20.0, false));
	//std::cout << "hereerer: " << fdh/20.0 << std::endl;
	slist.clear();
	
	slist.push_back(Pt(fdw*(1/12.0),fdh*(1/2.0)));
	slist.push_back(Pt(fdw*(6/12.0),fdh*(1/2.0)));
	slist.push_back(Pt(fdw*(6/12.0),fdh*(1/2.0)));
	slist.push_back(Pt(fdw*(11/12.0),fdh*(1/2.0)));
	trackStrokes.push_back(Path(slist, Vec3f(0.8,0.3,0.0), 10));
	trackLabels.push_back(myLabel(Pt(fdw*(1/12.0)+fdw/40.0,fdh*(1/2.0)+fdh/40.0), Vec3f(1.0,1.0,1.0),
												"Bass Guitar", (int)fdh/20.0, (int)fdh/10.0, false));
	slist.clear();
	
	slist.push_back(Pt(fdw*(1/12.0),fdh*(1/4.0)));
	slist.push_back(Pt(fdw*(6/12.0),fdh*(1/4.0)));
	slist.push_back(Pt(fdw*(6/12.0),fdh*(1/2.0)));
	slist.push_back(Pt(fdw*(11/12.0),fdh*(1/2.0)));
	trackStrokes.push_back(Path(slist, Vec3f(0.8,0.3,0.0), 10));
	trackLabels.push_back(myLabel(Pt(fdw*(1/12.0)+fdw/40.0,fdh*(1/4.0)+fdh/40.0), Vec3f(1.0,1.0,1.0),
												"Drums", (int)fdh/20.0, (int)fdh/20.0, false));
	slist.clear();

}

void sendOSCMessages(int pInd){

	//const std::vector<int> &vol = drum->getVolumeVec();

  UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
  	args->osc_port ) );
  char buffer[OUTPUT_BUFFER_SIZE];
  
  //for(int i=0; i<polyButtons.size(); i++){
  	const std::vector<int> &param = polyButtons[pInd]->getDrumInterface()->getVolumeVec();
  	std::string first = "/";
  	first += polyButtons[pInd]->getDrumInterface()->getId();
  	for(unsigned int j=0; j<param.size(); j++){
			osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
			p << osc::BeginBundleImmediate 
				<< osc::BeginMessage(first.c_str())
			  << int(j+1)
				<< param[j]
				<< osc::EndMessage;
			transmitSocket.Send( p.Data(), p.Size() );
			//std::cout << "param: " << j+1 << " " << param[j] << std::endl;
		}
  //}

}

void drawTracksDemo(double fdh){
	for(unsigned int i=0; i<trackStrokes.size(); i++){
		trackStrokes[i].draw();
		trackLabels[i].render(fdh);
	}
	
}

void draw() { 

//	drawRoutine.initial_draw();
	
//	if(args->include_lasers){
//  	drawRoutine.draw_strokes(true);
//  	drawRoutine.draw_sliders();
//  	drawRoutine.draw_strokes(false);
//  }else{
//  	drawRoutine.draw_sliders();
//	}
//	drawRoutine.draw_cursors();


	//INITIALIZE ================================================
	static GLfloat amb[] =  {0.4, 0.4, 0.4, 0.0};
  static GLfloat dif[] =  {1.0, 1.0, 1.0, 0.0};
 	double fdw = args->tiled_display.full_display_width;
  double fdh = args->tiled_display.full_display_height;

  float s = 0.0;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  amb[3] = dif[3] = cos(s) / 2.0 + 0.5;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT2);
  glDisable(GL_LIGHT1);
  amb[3] = dif[3] = 0.5 - cos(s * .95) / 2.0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  args->tiled_display.ORTHO();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  //=====================================================
  
  	if(glutLoopNumber == 15){
  		polyButton *pb;
  		std::cout << "fdh " << fdh << std::endl;
			std::cout << "fdw " << fdw << std::endl;
			std::vector<std::string> beatVertNames;
			beatVertNames.push_back("Straight");
			beatVertNames.push_back("Funk");
			beatVertNames.push_back("Hip-Hop");
			beatVertNames.push_back("Rock");
			beatVertNames.push_back("Metal");
  		pb = new polyButton(Pt((fdw/4),(fdh/4)),"Beat", 220, 220, fdh, beatVertNames, Pt(0,110), Vec3f(0.3,0.3,0.3));
  		polyButtons.push_back(pb);
  		
  		std::vector<std::string> distVertNames;
			distVertNames.push_back("Clip");
			distVertNames.push_back("Mid-Boost");
			distVertNames.push_back("Scoop");
  		pb = new polyButton(Pt((fdw/4),(fdh/2)),"Dist.", 220, 220, fdh, distVertNames, Pt(57,115), Vec3f(0.3,0.3,0.3));
  		polyButtons.push_back(pb);
  		
  		std::vector<std::string> modVertNames;
			modVertNames.push_back("Phaser");
			modVertNames.push_back("Chorus");
			modVertNames.push_back("Flanger");
			modVertNames.push_back("Tremelo");
  		pb = new polyButton(Pt((fdw/4),((fdh*3)/4)),"Mod", 220, 220, fdh, modVertNames, Pt(64,115), Vec3f(0.3,0.3,0.3));
  		polyButtons.push_back(pb);
  		
  		std::vector<std::string> eqVertNames;
			eqVertNames.push_back("Mid");
			eqVertNames.push_back("Bass");
			eqVertNames.push_back("Treble");
  		pb = new polyButton(Pt(((fdw*3)/4),(fdh/2)),"EQ", 220, 220, fdh, eqVertNames, Pt(64,110), Vec3f(0.3,0.3,0.3));
  		polyButtons.push_back(pb);
  		
  		initializeTracks(fdw,fdh);
  		
  		glutLoopNumber++;
		}else if(glutLoopNumber < 15){
			glutLoopNumber++;
		}else{ //glutLoopNumber > 15
			
			drawTracksDemo(fdh);
			
			for(unsigned int i=0; i<polyButtons.size(); i++){	
				polyButtons[i]->drawPB();
				if(glutLoopNumber == 16){
					polyButtons[i]->getDrumInterface()->updateParameterVec();
					sendOSCMessages(i);
				}
			}
			//drawTracksDemo();
			//drum->draw();
			//reverb->draw();
			//delay->draw();
			//std::cout << drum->getCenter() << std::endl;
			Interaction::drawCursors(); //fdh);
		}
		//=========================================================================
		
		glDisable(GL_LINE_SMOOTH);
	  glPopMatrix();
  	glMatrixMode(GL_PROJECTION);
	  glPopMatrix();
  	glPopAttrib();
	  glMatrixMode(GL_MODELVIEW);
		glutSwapBuffers();
		
}

void display(void) {
  HandleGLError("BEFORE DISPLAY");
  draw();
  HandleGLError("AFTER DISPLAY");
}

// ==========================================================================================
// TRACKING ROUTINES
// ==========================================================================================

//void AddTrackedPoint(const TrackedPoint &pt) {
//  int id = pt.getID();
//  global_colors.AssignRandomAvailableColor(id);
//  global_colors.ReAssignColor(id,0);  // starts
//}


//void RemoveTrackedPoint(const TrackedPoint &pt) {
//  int id = pt.getID();
//  Vec3f color = global_colors.GetColor(id);
//  global_colors.RemoveId(id);


//  for (unsigned int i = 0; i < drawRoutine._volume_sliders.size(); i++) {
//    if (drawRoutine._volume_sliders[i].button->isPressed() && 
//    		drawRoutine._volume_sliders[i].button->PressedBy() == id) {
//      //button_pressed_by[i] = -1;
//      drawRoutine._volume_sliders[i].button->release();
//    }
//  }

//}


// ==========================================================================================
// IDLE
// ==========================================================================================



void idle(void) {

	if (args->tiled_display.is_tiled_display || 1) {
    load_and_save_key_and_mouse_data(global_mk_dirlock,MK_ACTION_FILENAME_TEMPLATE,
				     Interaction::keyfunc_helper,
				     Interaction::specialkeyfunc_helper,
				     Interaction::mousefunc_helper,
				     Interaction::motionfunc_helper);
  }
	
//	if(args->include_lasers){
//		// look for new IR data
//		while (!global_dirlock.TryLock()) {
//		  usleep(1000);
//		}
//		std::vector<IR_Data_Point> raw_points;    
//		bool success;
//		{ /* SCOPE FOR istr */
//		  std::ifstream istr(FOUND_IR_POINTS_FILENAME);
//		  assert (istr);
//		  success = PointTracker::ReadIRPointData(istr,raw_points);
//		} /* SCOPE FOR istr */
//		global_dirlock.Unlock();
//		if (!success) {
//		  // new data not yet available
//		  usleep(1000);
//		  return;
//		}
//		global_point_tracker->ProcessPointData(raw_points);
//	}
  
  //if(args->include_lasers){
  //check_for_button_press();
	//}
  //check_for_button_motion();
	
  display();
	//if(glutLoopNumber > 15){
  	//sendOSCMessages();
	//}
  // ~ 60 fp
  usleep(1000);
  // ~ 30 fp
  //usleep(30000);
  //usleep(50000);
  	
  	
}

//MOUSE BUTTON INTERACTION CALLS======================================================
//====================================================================================

void TryToPressObj(int cursIndex) {
	
  //Cursor *c = Interaction::cursorVec[cursIndex];
  Cursor *c = Interaction::getCursor(cursIndex);
  std::cout << "PRESS BY: " << *c << std::endl;

  //	double fdh = args->tiled_display.full_display_height;
	Vec3f bcolor;
	std::cout << "gets here" << std::endl << std::endl;
		
  //drumInterface *d;
	Button *cb;
	Button *b;
	int pInd = c->getPressedPolyButtonInd();
	//if the cursor is already assigned a polyButton, and clicks away from it, release it
	if(pInd != -1){
	  if(!polyButtons[pInd]->getDrumInterface()->isLegalMove(c->getPosition())) { //fdh))){
			//bcolor = Vec3f(0.15,0.15,0.15);
			polyButtons[pInd]->setCollapsed(true);
			c->setPolyButtonInd(-1);
		}
	}
	for(unsigned int i=0; i<polyButtons.size(); i++){
		//if the polyButton is in collapsed form
		if(polyButtons[i]->isCollapsed()){
			std::cout << "collapsed" << std::endl;
			cb = polyButtons[i]->getCollapsedButton();
			if(cb->PointInside(c->getPosition()) && !cb->isPressed()){
					c->setPolyButtonInd(i);
					//set background color
					bcolor = Vec3f(c->getColor().x()/3,c->getColor().y()/3,c->getColor().z()/3);
					polyButtons[i]->getDrumInterface()->setBackgroundColor(bcolor);
					polyButtons[i]->setCollapsed(false);
			}
		//if the polyButton is in expanded form	
		}else{
			std::cout << "expanded" << std::endl;
			b = polyButtons[i]->getDrumInterface()->getButton();
			//b = d->getButton();
			if(b->PointInside(c->getPosition()) && !b->isPressed()){
					//void press(const Vec3f &bc, int id, const Pt &pos);
					b->press(c->getColor(), c->getId(), c->getPosition());
					c->setButton(b);
			}
		}
		
	}

		
		/*
		Button *b = drum->getButton();
		if(b->PointInside(c->getPosition(fdh))){
			if(!b->isPressed()){
				//void press(const Vec3f &bc, int id, const Pt &pos);
				b->press(c->getColor(), c->getId(), c->getPosition(fdh));
				c->setButton(b);
			}
		}
		*/
}

void TryToReleaseObj(int cursIndex) {

  Cursor *c = Interaction::cursorVec[cursIndex];
  std::cout << "RELEASE BY: " << *c << std::endl;

	//double fdh = args->tiled_display.full_display_height;
	Button *b = c->getPressedButton();
	if(b != NULL){
		b->release();
	}
	c->setButton(NULL);

}

void TryToMoveObj(int cursIndex) {

  Cursor *c = Interaction::cursorVec[cursIndex];
  std::cout << "MOVE FROM: " << *c << std::endl;

  //	double fdh = args->tiled_display.full_display_height;
	Button *b = c->getPressedButton();
	if(b != NULL){
		int pInd = c->getPressedPolyButtonInd();
		if(pInd != -1){
			if(polyButtons[pInd]->getDrumInterface()->isLegalMove(c->getPosition())){
				b->Move(c->getPosition());
				sendOSCMessages(pInd);
			}
		}
	}

}

////===================================================================================

//#define HOVER_THRESHOLD 10


//void check_for_button_motion() {

//	if(args->include_lasers){
//		for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
//		  if (!drawRoutine._volume_sliders[j].button->isPressed()) continue;
//		  int id = drawRoutine._volume_sliders[j].button->PressedBy();
//		  assert (id != -1);

//		  //global_colors.ReAssignColor(id,j+1); 

//		  Button *b2 = drawRoutine._volume_sliders[j].button;
//		  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
//		    if (id != (*global_point_tracker)[i].getID()) continue;


//		    Pt current_pos = b2->getCentroid();

//		    Pt p = (*global_point_tracker)[i].getCurrentPosition();
//		    Pt offset = drawRoutine._volume_sliders[j].button->getPressPosition();

//		    Pt q = Pt(p.x-offset.x,p.y-offset.y);

//		    //q += Pt(buttons[j].getWidth(), buttons[j].getHeight()); // * 0.5;

//		    q.y = std::max(q.y,1.0*b2->getHeight());
//		    q.y = std::min(q.y,(double)args->
//		    	tiled_display.full_display_height-1.0*b2->getHeight());

//		    double slider_length = args->
//		    	tiled_display.full_display_height - 2.0 * b2->getHeight();
//		    double slider_pos = q.y - 1.0*b2->getHeight();

//		    double slider_frac = slider_pos / double(slider_length);

//		    //std::cout << "button " << j << " new value " << q.y << " " << slider_frac 
//		    	//<< 	 std::endl;

//		    assert (j < drawRoutine._volume_sliders.size());
//		    drawRoutine._volume_sliders[j].value = slider_frac;
//			    
//		    q = Pt(current_pos.x,q.y);

//		    //b2.Move(q);
//		    b2->Move(q); //WithMaxSpeed(q, 10);
//		  }
//		}
//  }
//  
//  //This is the correct place to send messages from
//  
//  //send messages for volume sliders
//  for (unsigned int i = 0; i < drawRoutine._volume_sliders.size(); i++) {
//  	
//  	//if slider at bottom, mute the sound entirely
//  	int value;
//  	if((drawRoutine._volume_sliders[i].value) < 0.01){
//  		value = 0;
//		}else{
//			value = (int)(drawRoutine._volume_sliders[i].value * 45 + 85);
//		}

//    UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
//    	args->osc_port ) );
//    char buffer[OUTPUT_BUFFER_SIZE];
//    
//    osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//    p << osc::BeginBundleImmediate 
//      << osc::BeginMessage( "vol" ) 
//      << drawRoutine._volume_sliders[i].name.c_str()
//      << value
//      << osc::EndMessage;
//      transmitSocket.Send( p.Data(), p.Size() );
//    
//    //std::cout << int(drawRoutine._volume_sliders[i].value * 45 + 85) << std::endl;
//  }
//  //send messages for eq sliders
//  for (unsigned int i = 0; i < drawRoutine._eq_sliders.size(); i++) {

//    UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
//    	args->osc_port ) );
//    char buffer[OUTPUT_BUFFER_SIZE];
//    
//    osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//    p << osc::BeginBundleImmediate 
//      << osc::BeginMessage( "eq" ) 
//      << drawRoutine._eq_sliders[i].name.c_str()
//      << int(drawRoutine._eq_sliders[i].value * 100)
//      << osc::EndMessage;
//      transmitSocket.Send( p.Data(), p.Size() );
//    
//    //std::cout << int(drawRoutine._eq_sliders[i].value * 128) << std::endl;
//  }
//  
//  //send messages for fx sliders
//  for (unsigned int i = 0; i < drawRoutine._fx_sliders.size(); i++) {
//  	
//  	double newVal = double(drawRoutine._fx_sliders[i].value);
//  	
//  	if(drawRoutine._fx_sliders[i].name == "ps"){
//  		//std::cout << drawRoutine._fx_sliders[i].name.c_str() << std::endl;
//  		newVal *= 30;
//  		newVal = int(newVal);
//  		newVal = (newVal - 10) * (-1);
//  		//std::cout << "new " << newVal << std::endl;
//  		
//  		for(int j=0; j<drawRoutine._buttons.size(); j++){
//  			std::string &hold1 = drawRoutine._buttons[j]->getText(0);
//      	if(hold1 == "Pitchshifter"){
//      		std::string &hold2 = drawRoutine._buttons[j]->getText(1);
//      		if(hold2 == "OFF"){
//      			newVal = 0;
//    			}
//    		}
//  		}
//  	}
//		
//    UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
//    	args->osc_port ) );
//    char buffer[OUTPUT_BUFFER_SIZE];
//    
//    //messages for the fx sliders
//		  osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//		  p << osc::BeginBundleImmediate 
//		    << osc::BeginMessage( "fx" ) 
//		    << drawRoutine._fx_sliders[i].name.c_str()
//		    << (float)newVal
//		    << osc::EndMessage;
//		    transmitSocket.Send( p.Data(), p.Size() );
//    
//    //std::cout << int(drawRoutine._eq_sliders[i].value * 128) << std::endl;
//  }

//}

//void check_for_button_press() {

//	if(args->include_lasers){	
//		for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
//		  if (!(*global_point_tracker)[i].IsActive()) continue;
//		  int id = (*global_point_tracker)[i].getID();
//		  const std::list<Pt> &trail = (*global_point_tracker)[i].getPtTrail();      

//		  bool already = false;
//		  // check if this tracker is already attached to a button
//		  for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
//		    if (drawRoutine._volume_sliders[j].button->isPressed() && 
//		    		drawRoutine._volume_sliders[j].button->PressedBy() == id) {
//					already = true;
//		    }
//		  }
//		  if (already) continue;


//		  
//		  //for (unsigned int j = 0; j < buttons.size(); j++) {
//		  for (int j = int(drawRoutine._volume_sliders.size())-1; j >= 0; j--) {
//		    Button *b2 = drawRoutine._volume_sliders[j].button;
//		    if (b2->Hover(trail,HOVER_THRESHOLD)) {

//					// don't steal from someone else!
//					if (!drawRoutine._volume_sliders[j].button->isPressed()) { 
//						Pt pt = b2->Offset((*global_point_tracker)[i].getCurrentPosition());
//						global_colors.ReAssignColor(id,j+1); 
//						b2->press(global_colors.GetColor(id),id,pt);
//					}

//					// don't click more than one button!!!
//					break;
//				}
//		  }
//		}
//	}
//	for(int j=0; j<drawRoutine._buttons.size(); j++){
//		if(drawRoutine._buttons[j]->isPressed()){
//			std::cout << "control button pressed!" << std::endl;

//			UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
//    	args->osc_port ) );
//    	char buffer[OUTPUT_BUFFER_SIZE];
//    
//    	osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//    	p << osc::BeginBundleImmediate 
//      	<< osc::BeginMessage( "ctrl" ) 
//      	<< j
//      	<< osc::EndMessage;
//      transmitSocket.Send( p.Data(), p.Size() );
//      
//      std::string &hold1 = drawRoutine._buttons[j]->getText(0);
//      if(hold1 == "Pitchshifter"){
//      	std::string &hold2 = drawRoutine._buttons[j]->getText(1);
//      	if(hold2 == "OFF"){
//      		drawRoutine._buttons[j]->clearText();
//      		drawRoutine._buttons[j]->addText("Pitchshifter");
//      		drawRoutine._buttons[j]->addText("ON");
//    		}else{
//    			drawRoutine._buttons[j]->clearText();
//      		drawRoutine._buttons[j]->addText("Pitchshifter");
//    			drawRoutine._buttons[j]->addText("OFF");
//  			}
//      }
//      //std::cout << "here it be: " << hold << std::endl;
//			drawRoutine._buttons[j]->release();
//		}
//	}
//}

void reshape(int w, int h) {
	HandleGLError("BEFORE RESHAPE");
	args->tiled_display.reshape(w,h);
	//args->width = w;
	//args->height = h;
	glViewport(0,0,w,h);

	glMatrixMode(GL_PROJECTION);
	gluPerspective(40.0, 1.0, 1.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 5.0,
						0.0, 0.0, 0.0,
						0.0, 1.0, 0.);
	glTranslatef(0.0, 0.6, -1.0);
	//drawRoutine.initialize_sliders();

	HandleGLError("AFTER RESHAPE");
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q') { 
    exit(0); 
  }
  else if(key == 'b' || key == ' '){
    //game->keypress();
  }
  else if(key == 'r'){
    //game->resetGame();
  }
  else {
    //std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
  display();
  //  glutPostRedisplay();
}

// ===================================================================
// MOUSE FUNCTIONS

void keyfunc(unsigned char key, int x, int y) {
  //std::cout << "RECEIVED KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
//  if (args->tiled_display.is_tiled_display) {
//    log_keypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
//  } else {
  Interaction::keyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  //}
}

void specialkeyfunc(int key, int x, int y) {
  //std::cout << "RECEIVED SPECIAL KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
//  if (args->tiled_display.is_tiled_display) {
//    log_specialkeypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
//  } else {
  Interaction::specialkeyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  //}
}


void mousefunc(int button, int state, int x, int y) {
  //std::cout << "RECEIVED MOUSE DIRECTLY: " << button << " " << state << " " << x << " " << y << std::endl;
//  if (args->tiled_display.is_tiled_display) {
//    // adjust position of click
//    int from_top = args->tiled_display.full_display_height - 
//    	(args->tiled_display.my_height+args->tiled_display.my_bottom);
//    y += from_top;
//    log_mouseaction(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
//  } else {
  Interaction::mousefunc_helper(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
  //}
}

void motionfunc(int x, int y) {
  //std::cout << "RECEIVED MOTION DIRECTLY: " << x << " " << y << std::endl;
//  if (args->tiled_display.is_tiled_display) {
//    if (x == 0 && y == 0) {
//      //std::cout << "IGNORE: OFFSCREEN MOTION BUG" << std::endl;
//      return;
//    }
//    int from_top = args->tiled_display.full_display_height - 
//    	(args->tiled_display.my_height+args->tiled_display.my_bottom);
//    y += from_top;
//    log_mousemotion(PRIMARY_MOUSE,x,y,glutGetModifiers());
//  } else {
  Interaction::motionfunc_helper(PRIMARY_MOUSE,x,y,glutGetModifiers());
  //}
}


////////////////////////////////////////////////////////////////

//// this will get called eventually :)
//void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers) {
//  //std::cout << "RECEIVED KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x 		<< " " << y << " " << glut_modifiers << std::endl;
//  if (key == 'q') {
//    exit(0);
//  }
//  if(key == 'b'){
//    //for(unsigned int i = 0; i < piece_pointers.size(); ++i){
//    //  piece_pointers[i]->ResetBorder();
//    //}
//  }
//  else {
//    //std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
//  }
//  display();
//}


//void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers) {
//  //std::cout << "RECEIVED SPECIAL KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glut_modifiers << std::endl;

//}

////maybe each cursor has its own version of this function and motion function.
////this way specifying which mouse is unnecessary.
////would this screw up load_and_save_key_and_mouse_data() from key_and_mouse_logger.h entirely?

//void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers) {
//  std::cout << "RECEIVED MOUSE FROM LOGGER: " << which_mouse << " " << button << " " << state << " " << x << " " << y << " " << glut_modifiers << std::endl;

//  if (state == GLUT_DOWN) {
//  	//drawRoutine.set_clicked(which_mouse-201,true);
//    //TryToPressButton(which_mouse,x,y);
//    std::cout << "downdowndowndown" << std::endl;
//  }
//  if (state == GLUT_UP) {
//  	//drawRoutine.set_clicked(which_mouse-201,false);
//    //TryToReleaseButton(which_mouse,x,y);
//    std::cout << "upupuppuuppuuppupu" << std::endl;
//  }

//}

//void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers) {
//  std::cout << "RECEIVED MOTION FROM LOGGER: " << which_mouse << " " << x << " " << y << 			" " << glut_modifiers << std::endl;

//  //TryToMoveButton(which_mouse,x,y);

//}
// ===================================================================
 

int  main(int argc, char **argv) {

  args = new ArgParser(argc,argv);
//  global_point_tracker = NULL;
  global_calibration_data = new 
  	PlanarCalibration	(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,
  	PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);
  	
  	
	//std::vector<cursor*> cursorVec;
  Interaction::setupCursors(&(args->tiled_display));
  
	
  args->tiled_display.set_from_calibration_data(global_calibration_data);


  // initialize things...
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);
  glutInitWindowPosition(20,20);
  glutCreateWindow("multi user collaborative audio environment");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialkeyfunc);
	glutMouseFunc(mousefunc);
	glutMotionFunc(motionfunc);

  GLfloat light0_ambient[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat light0_diffuse[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat light1_diffuse[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat light1_position[] = {1.0, 1.0, 1.0, 0.0};
  GLfloat light2_diffuse[] = {0.0, 1.0, 0.0, 1.0};
  GLfloat light2_position[] = {-1.0, -1.0, 1.0, 0.0};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(2.0);
  
  HandleGLError("main 1");

  if (args->tiled_display.full_screen) {
    glutFullScreen();
  } 
  
	//onedw = args->tiled_display.full_display_width/2;
	//onedh = args->tiled_display.full_display_height/2;
	
	//double windowWidth, windowHeight;
	//windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	//windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
  
  //std::cout << "asdfasdfaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa " << windowWidth << " " << windowHeight << std::endl;


//  HandleGLError("BEFORE INITIALIZE BUTTONS");

//  //initialize_buttons();
//  HandleGLError("AFTER INITIALIZE BUTTONS");
//  
//  //drawRoutine = vcDraw(args, global_colors, global_point_tracker);
//  HandleGLError("BEFORE INITIALIZE BUTTONS");
//	//drawRoutine.initialize_sliders();
//  HandleGLError("AFTER INITIALIZE BUTTONS");
  
  glutMainLoop();

  return 0;
}


