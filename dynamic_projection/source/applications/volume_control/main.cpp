#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>
#include <cmath>

#include "../../common/directory_locking.h"
#include "../../../../remesher/src/vectors.h"

#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
//#include "../../calibration/planar_interpolation_calibration/colors.h"

//#include "argparser.h"
//#include "../paint/button.h"
//#include "../paint/path.h"
//#include "../../common/Image.h"
//#include "../../multi_mouse/key_and_mouse_logger.h"

//#include "pong.h"
#include "../paint/text.h"

//for OSC library
#include "../../common/oscpack/osc/OscOutboundPacketStream.h"
#include "../../common/oscpack/ip/UdpSocket.h"

#include "vcDraw.h"
//#include "MySlider.h"

#define ADDRESS "128.213.17.125"
#define PORT 7000
#define OUTPUT_BUFFER_SIZE 1024


#define IR_STATE_DIRECTORY                "../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"

#define MK_STATE_DIRECTORY                   "../state/mouse_and_keyboard"
#define MK_ACTION_FILENAME_TEMPLATE          "../state/mouse_and_keyboard/actions_XXX.txt"
//#define APPLICATIONS_STATE_DIRECTORY            "../state/applications/"

#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args;
DirLock global_dirlock(IR_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;
PointTracker *global_point_tracker;
//PongGame *game;

Colors global_colors;
//std::vector<Button> buttons;

double TILE_w;
double TILE_h;

//Mouse Globals======= 
//DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);
DirLock global_mk_dirlock(MK_STATE_DIRECTORY);

std::vector<std::pair<std::list<Pt>,Vec3f> > GLOBAL_strokes;


Pt mouse_location;
std::vector<Pt> positions;
std::vector<Pt> correctpositions;

vcDraw drawRoutine;

//std::vector<bool> isClicked;
// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

//void draw_strokes(bool button_strokes);
//void draw_buttons();

//void initialize_buttons();
void check_for_button_press();
void check_for_button_motion();

//for mice=============
//void draw_cursors();

void TryToPressButton(int id, double x, double y);
void TryToReleaseButton(int id, double x, double y);
void TryToMoveButton(int id, double x, double y);

void keyfunc(unsigned char key, int x, int y);
void specialkeyfunc(int key, int x, int y);		
void mousefunc(int button, int state, int x, int y);
void motionfunc(int x, int y);

void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers);
void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers);		 
void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers);
void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers);

void clamp_to_display(Pt &pt) { 
  int x = pt.x;
  int y = pt.y;
  x = std::max(0,std::min(x,args->tiled_display.full_display_width));
  y = std::max(0,std::min(y,args->tiled_display.full_display_height));
  pt = Pt(x,y);
}
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

void draw() { 

	drawRoutine.initial_draw();
	
	if(args->include_lasers){
  	drawRoutine.draw_strokes(true);
  	drawRoutine.draw_sliders();
  	drawRoutine.draw_strokes(false);
  }else{
  	drawRoutine.draw_sliders();
	}
	drawRoutine.draw_cursors();
	

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

void AddTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  global_colors.AssignRandomAvailableColor(id);
  global_colors.ReAssignColor(id,0);  // starts
}


void RemoveTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  Vec3f color = global_colors.GetColor(id);
  global_colors.RemoveId(id);


  for (unsigned int i = 0; i < drawRoutine._volume_sliders.size(); i++) {
    if (drawRoutine._volume_sliders[i].button->isPressed() && 
    		drawRoutine._volume_sliders[i].button->PressedBy() == id) {
      //button_pressed_by[i] = -1;
      drawRoutine._volume_sliders[i].button->release();
    }
  }

}


// ==========================================================================================
// IDLE
// ==========================================================================================

void idle(void) {

	if (args->tiled_display.is_tiled_display || 1) {
    load_and_save_key_and_mouse_data(global_mk_dirlock,MK_ACTION_FILENAME_TEMPLATE,
				     keyfunc_helper,
				     specialkeyfunc_helper,
				     mousefunc_helper,
				     motionfunc_helper);
  }
	
	if(args->include_lasers){
		// look for new IR data
		while (!global_dirlock.TryLock()) {
		  usleep(1000);
		}
		std::vector<IR_Data_Point> raw_points;    
		bool success;
		{ /* SCOPE FOR istr */
		  std::ifstream istr(FOUND_IR_POINTS_FILENAME);
		  assert (istr);
		  success = PointTracker::ReadIRPointData(istr,raw_points);
		} /* SCOPE FOR istr */
		global_dirlock.Unlock();
		if (!success) {
		  // new data not yet available
		  usleep(1000);
		  return;
		}
		global_point_tracker->ProcessPointData(raw_points);
	}
  
  //if(args->include_lasers){
  check_for_button_press();
	//}
  check_for_button_motion();

  display();
  // ~ 60 fp
  usleep(1000);
  // ~ 30 fp
  //usleep(30000);
  //usleep(50000);
}

//MOUSE BUTTON INTERACTION CALLS======================================================
//====================================================================================

void TryToPressButton(int id, double x, double y) {

  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;
  
  y = args->tiled_display.full_display_height-y;

  //std::cout << "TRY TO PRESS" << std::endl;

  // check if this tracker is already attached to a vol button
  for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
    if (drawRoutine._volume_sliders[j].button->isPressed() && 
    		drawRoutine._volume_sliders[j].button->PressedBy() == id) {
      //already = true;
      //std::cout << "ALREADY PRESSING A BUTTON! " << std::endl;
      return;
    }
  }
  // check if this tracker is already attached to an eq button
  for (unsigned int j = 0; j < drawRoutine._eq_sliders.size(); j++) {
    if (drawRoutine._eq_sliders[j].button->isPressed() && 
    		drawRoutine._eq_sliders[j].button->PressedBy() == id) {
      //already = true;
      //std::cout << "ALREADY PRESSING A BUTTON! " << std::endl;
      return;
    }
  }
  // check if this tracker is already attached to an fx button
  for (unsigned int j = 0; j < drawRoutine._fx_sliders.size(); j++) {
    if (drawRoutine._fx_sliders[j].button->isPressed() && 
    		drawRoutine._fx_sliders[j].button->PressedBy() == id) {
      //already = true;
      //std::cout << "ALREADY PRESSING A BUTTON! " << std::endl;
      return;
    }
  }
  // check if this tracker is already attached to a control button
  for (unsigned int j = 0; j < drawRoutine._buttons.size(); j++) {
    if (drawRoutine._buttons[j]->isPressed() && 
    		drawRoutine._buttons[j]->PressedBy() == id) {
      //already = true;
      //std::cout << "ALREADY PRESSING A BUTTON! " << std::endl;
      return;
    }
  }

	//volume slider button press
  for (int j = int(drawRoutine._volume_sliders.size())-1; j >= 0; j--) {
    Button *b2 = drawRoutine._volume_sliders[j].button;
    if (b2->PointInside(Pt(x,y))) { //Hover(trail,HOVER_THRESHOLD)) {
      
      if (drawRoutine._volume_sliders[j].button->isPressed()) {
				//std::cout << "ALREADY PRESSED" << std::endl;
				continue;
      }

      // don't steal from someone else!
      if (!drawRoutine._volume_sliders[j].button->isPressed()) {
				Pt pt = b2->Offset(Pt(x,y)); //(*global_point_tracker)[i].getCurrentPosition());
				b2->press(global_colors.GetColor(id),id,pt);
      }
      
      // don't click more than one button!!!
      break;
    }
  }
  //eq slider button press
  for (int j = int(drawRoutine._eq_sliders.size())-1; j >= 0; j--) {
    Button *b2 = drawRoutine._eq_sliders[j].button;
    if (b2->PointInside(Pt(x,y))) { //Hover(trail,HOVER_THRESHOLD)) {
      
      if (drawRoutine._eq_sliders[j].button->isPressed()) {
				//std::cout << "ALREADY PRESSED" << std::endl;
				continue;
      }

      // don't steal from someone else!
      if (!drawRoutine._eq_sliders[j].button->isPressed()) {
				Pt pt = b2->Offset(Pt(x,y)); //(*global_point_tracker)[i].getCurrentPosition());
				b2->press(global_colors.GetColor(id),id,pt);
      }
      
      // don't click more than one button!!!
      break;
    }
  }
  //fx slider button press
  for (int j = int(drawRoutine._fx_sliders.size())-1; j >= 0; j--) {
    Button *b2 = drawRoutine._fx_sliders[j].button;
    if (b2->PointInside(Pt(x,y))) { //Hover(trail,HOVER_THRESHOLD)) {
      
      if (drawRoutine._fx_sliders[j].button->isPressed()) {
				//std::cout << "ALREADY PRESSED" << std::endl;
				continue;
      }

      // don't steal from someone else!
      if (!drawRoutine._fx_sliders[j].button->isPressed()) {
				Pt pt = b2->Offset(Pt(x,y)); //(*global_point_tracker)[i].getCurrentPosition());
				b2->press(global_colors.GetColor(id),id,pt);
      }
      
      // don't click more than one button!!!
      break;
    }
  }
  //control buttons press
  for (int j = int(drawRoutine._buttons.size())-1; j >= 0; j--) {
    Button *b2 = drawRoutine._buttons[j];
    if (b2->PointInside(Pt(x,y))) { //Hover(trail,HOVER_THRESHOLD)) {
      
      if (drawRoutine._buttons[j]->isPressed()) {
				//std::cout << "ALREADY PRESSED" << std::endl;
				continue;
      }

      // don't steal from someone else!
      if (!drawRoutine._buttons[j]->isPressed()) {
				Pt pt = b2->Offset(Pt(x,y)); //(*global_point_tracker)[i].getCurrentPosition());
				b2->press(global_colors.GetColor(id),id,pt);
      }
      
      // don't click more than one button!!!
      break;
    }
  }
}

void TryToReleaseButton(int id, double x, double y) {
  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;
  y = args->tiled_display.full_display_height-y;
  //volume slider release
  for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
    if (drawRoutine._volume_sliders[j].button->isPressed() && 
    		drawRoutine._volume_sliders[j].button->PressedBy() == id) {
      drawRoutine._volume_sliders[j].button->release();
      return;
    }
  }
  //eq slider release
  for (unsigned int j = 0; j < drawRoutine._eq_sliders.size(); j++) {
    if (drawRoutine._eq_sliders[j].button->isPressed() && 
    		drawRoutine._eq_sliders[j].button->PressedBy() == id) {
      drawRoutine._eq_sliders[j].button->release();
      return;
    }
  }
  //fx slider release
  for (unsigned int j = 0; j < drawRoutine._fx_sliders.size(); j++) {
    if (drawRoutine._fx_sliders[j].button->isPressed() && 
    		drawRoutine._fx_sliders[j].button->PressedBy() == id) {
    	drawRoutine._fx_sliders[j].button->release();
    		
      if(drawRoutine._fx_sliders[j].name == "ps"){
      	//multiply by 30 because messages sent out are converted to a range between -10 and 20
      	//want to make the slider jump to one of the int values for that range
      	double oldVal = drawRoutine._fx_sliders[j].value;
      	int holdi = (int)(oldVal * 30.0);
      	double newVal = (double)holdi;
      	newVal /= 30.0;
      	//drawRoutine._fx_sliders[j].value = newVal;
      	
      	float y_max = drawRoutine._fx_sliders[j].button_max_height;
      	float y_min = drawRoutine._fx_sliders[j].button_min_height;
      	float y_dist = y_max - y_min;
				
				std::cout << (double)((newVal - oldVal)*y_dist)<< std::endl;
				
      	Pt q = Pt(drawRoutine._fx_sliders[j].button->getCentroid().x,
      					drawRoutine._fx_sliders[j].button->getCentroid().y + ((newVal - oldVal)*y_dist));
				drawRoutine._fx_sliders[j].button->MoveNoDamping(q);
    	}
    	
      return;
    }
  }
  //control buttons release
  for (unsigned int j = 0; j < drawRoutine._buttons.size(); j++) {
    if (drawRoutine._buttons[j]->isPressed() && 
    		drawRoutine._buttons[j]->PressedBy() == id) {
      drawRoutine._buttons[j]->release();
      return;
    }
  }
  //std::cout << "OH WELL, NOT PRESSING A BUTTON! " << std::endl;
}

void TryToMoveButton(int id, double x, double y) {
  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;
  y = args->tiled_display.full_display_height-y;
  // check if this tracker is already attached to a volume button
  for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
    if (drawRoutine._volume_sliders[j].button->isPressed() && 
    		drawRoutine._volume_sliders[j].button->PressedBy() == id) {
      Button *b2 = drawRoutine._volume_sliders[j].button;
      Pt p = Pt(x,y);
      Pt offset = drawRoutine._volume_sliders[j].button->getPressPosition();
      float y_move = p.y-offset.y;
      float y_max = drawRoutine._volume_sliders[j].button_max_height;
      float y_min = drawRoutine._volume_sliders[j].button_min_height;
      Pt q;
      if(y_move < y_max && y_move >= y_min){
      	q = Pt(drawRoutine._volume_sliders[j].button->getCentroid().x,p.y-offset.y);
      }else if(y_move >= y_max){
      	q = Pt(drawRoutine._volume_sliders[j].button->getCentroid().x,y_max);
    	}else if(y_move < y_min){
      	q = Pt(drawRoutine._volume_sliders[j].button->getCentroid().x,y_min);
    	}
    	
    	
    	double slider_length = y_max - y_min;
    	double slider_frac = (q.y - y_min) / (double)slider_length;
    	
    	assert (j < drawRoutine._volume_sliders.size());
      drawRoutine._volume_sliders[j].value = slider_frac;
    	
    	b2->Move(q);
      return;
    }
  }
  // check to see if this tracker is attached to an eq button
  for (unsigned int j = 0; j < drawRoutine._eq_sliders.size(); j++) {
    if (drawRoutine._eq_sliders[j].button->isPressed() && 
    		drawRoutine._eq_sliders[j].button->PressedBy() == id) {
      Button *b2 = drawRoutine._eq_sliders[j].button;
      Pt p = Pt(x,y);
      Pt offset = drawRoutine._eq_sliders[j].button->getPressPosition();
      float y_move = p.y-offset.y;
      float y_max = drawRoutine._eq_sliders[j].button_max_height;
      float y_min = drawRoutine._eq_sliders[j].button_min_height;
      Pt q;
      if(y_move < y_max && y_move >= y_min){
      	q = Pt(drawRoutine._eq_sliders[j].button->getCentroid().x,p.y-offset.y);
      }else if(y_move >= y_max){
      	q = Pt(drawRoutine._eq_sliders[j].button->getCentroid().x,y_max);
    	}else if(y_move < y_min){
      	q = Pt(drawRoutine._eq_sliders[j].button->getCentroid().x,y_min);
    	}
    	
    	
    	double slider_length = y_max - y_min;
    	double slider_frac = (q.y - y_min) / (double)slider_length;
    	
    	assert (j < drawRoutine._eq_sliders.size());
      drawRoutine._eq_sliders[j].value = slider_frac;
    	
    	b2->Move(q);
      return;
    }
  }
  // check to see if this tracker is attached to an fx button
  for (unsigned int j = 0; j < drawRoutine._fx_sliders.size(); j++) {
    if (drawRoutine._fx_sliders[j].button->isPressed() && 
    		drawRoutine._fx_sliders[j].button->PressedBy() == id) {
      Button *b2 = drawRoutine._fx_sliders[j].button;
      Pt p = Pt(x,y);
      Pt offset = drawRoutine._fx_sliders[j].button->getPressPosition();
      float y_move = p.y-offset.y;
      float y_max = drawRoutine._fx_sliders[j].button_max_height;
      float y_min = drawRoutine._fx_sliders[j].button_min_height;
      Pt q;
      if(y_move < y_max && y_move >= y_min){
      	q = Pt(drawRoutine._fx_sliders[j].button->getCentroid().x,p.y-offset.y);
      }else if(y_move >= y_max){
      	q = Pt(drawRoutine._fx_sliders[j].button->getCentroid().x,y_max);
    	}else if(y_move < y_min){
      	q = Pt(drawRoutine._fx_sliders[j].button->getCentroid().x,y_min);
    	}
    	
    	
    	double slider_length = y_max - y_min;
    	double slider_frac = (q.y - y_min) / (double)slider_length;
    	
    	assert (j < drawRoutine._fx_sliders.size());
      drawRoutine._fx_sliders[j].value = slider_frac;
    	
    	b2->Move(q);
      return;
    }
  }
}

//===================================================================================

#define HOVER_THRESHOLD 10


void check_for_button_motion() {

	if(args->include_lasers){
		for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
		  if (!drawRoutine._volume_sliders[j].button->isPressed()) continue;
		  int id = drawRoutine._volume_sliders[j].button->PressedBy();
		  assert (id != -1);

		  //global_colors.ReAssignColor(id,j+1); 

		  Button *b2 = drawRoutine._volume_sliders[j].button;
		  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
		    if (id != (*global_point_tracker)[i].getID()) continue;


		    Pt current_pos = b2->getCentroid();

		    Pt p = (*global_point_tracker)[i].getCurrentPosition();
		    Pt offset = drawRoutine._volume_sliders[j].button->getPressPosition();

		    Pt q = Pt(p.x-offset.x,p.y-offset.y);

		    //q += Pt(buttons[j].getWidth(), buttons[j].getHeight()); // * 0.5;

		    q.y = std::max(q.y,1.0*b2->getHeight());
		    q.y = std::min(q.y,(double)args->
		    	tiled_display.full_display_height-1.0*b2->getHeight());

		    double slider_length = args->
		    	tiled_display.full_display_height - 2.0 * b2->getHeight();
		    double slider_pos = q.y - 1.0*b2->getHeight();

		    double slider_frac = slider_pos / double(slider_length);

		    //std::cout << "button " << j << " new value " << q.y << " " << slider_frac 
		    	//<< 	 std::endl;

		    assert (j < drawRoutine._volume_sliders.size());
		    drawRoutine._volume_sliders[j].value = slider_frac;
			    
		    q = Pt(current_pos.x,q.y);

		    //b2.Move(q);
		    b2->Move(q); //WithMaxSpeed(q, 10);
		  }
		}
  }
  
  //This is the correct place to send messages from
  
  //send messages for volume sliders
  for (unsigned int i = 0; i < drawRoutine._volume_sliders.size(); i++) {
  	
  	//if slider at bottom, mute the sound entirely
  	int value;
  	if((drawRoutine._volume_sliders[i].value) < 0.01){
  		value = 0;
		}else{
			value = (int)(drawRoutine._volume_sliders[i].value * 45 + 85);
		}

    UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
    	args->osc_port ) );
    char buffer[OUTPUT_BUFFER_SIZE];
    
    osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
    p << osc::BeginBundleImmediate 
      << osc::BeginMessage( "vol" ) 
      << drawRoutine._volume_sliders[i].name.c_str()
      << value
      << osc::EndMessage;
      transmitSocket.Send( p.Data(), p.Size() );
    
    //std::cout << int(drawRoutine._volume_sliders[i].value * 45 + 85) << std::endl;
  }
  //send messages for eq sliders
  for (unsigned int i = 0; i < drawRoutine._eq_sliders.size(); i++) {

    UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
    	args->osc_port ) );
    char buffer[OUTPUT_BUFFER_SIZE];
    
    osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
    p << osc::BeginBundleImmediate 
      << osc::BeginMessage( "eq" ) 
      << drawRoutine._eq_sliders[i].name.c_str()
      << int(drawRoutine._eq_sliders[i].value * 100)
      << osc::EndMessage;
      transmitSocket.Send( p.Data(), p.Size() );
    
    //std::cout << int(drawRoutine._eq_sliders[i].value * 128) << std::endl;
  }
  
  //send messages for fx sliders
  for (unsigned int i = 0; i < drawRoutine._fx_sliders.size(); i++) {
  	
  	double newVal = double(drawRoutine._fx_sliders[i].value);
  	
  	if(drawRoutine._fx_sliders[i].name == "ps"){
  		//std::cout << drawRoutine._fx_sliders[i].name.c_str() << std::endl;
  		newVal *= 30;
  		newVal = int(newVal);
  		newVal = (newVal - 10) * (-1);
  		//std::cout << "new " << newVal << std::endl;
  		
  		for(unsigned int j=0; j<drawRoutine._buttons.size(); j++){
                  const std::string &hold1 = drawRoutine._buttons[j]->getText(0);
      	if(hold1 == "Pitchshifter"){
          const std::string &hold2 = drawRoutine._buttons[j]->getText(1);
      		if(hold2 == "OFF"){
      			newVal = 0;
    			}
    		}
  		}
  	}
		
    UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
    	args->osc_port ) );
    char buffer[OUTPUT_BUFFER_SIZE];
    
    //messages for the fx sliders
		  osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		  p << osc::BeginBundleImmediate 
		    << osc::BeginMessage( "fx" ) 
		    << drawRoutine._fx_sliders[i].name.c_str()
		    << (float)newVal
		    << osc::EndMessage;
		    transmitSocket.Send( p.Data(), p.Size() );
    
    //std::cout << int(drawRoutine._eq_sliders[i].value * 128) << std::endl;
  }

}

void check_for_button_press() {

	if(args->include_lasers){	
		for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
		  if (!(*global_point_tracker)[i].IsActive()) continue;
		  int id = (*global_point_tracker)[i].getID();
		  const std::list<Pt> &trail = (*global_point_tracker)[i].getPtTrail();      

		  bool already = false;
		  // check if this tracker is already attached to a button
		  for (unsigned int j = 0; j < drawRoutine._volume_sliders.size(); j++) {
		    if (drawRoutine._volume_sliders[j].button->isPressed() && 
		    		drawRoutine._volume_sliders[j].button->PressedBy() == id) {
					already = true;
		    }
		  }
		  if (already) continue;


		  
		  //for (unsigned int j = 0; j < buttons.size(); j++) {
		  for (int j = int(drawRoutine._volume_sliders.size())-1; j >= 0; j--) {
		    Button *b2 = drawRoutine._volume_sliders[j].button;
		    if (b2->Hover(trail,HOVER_THRESHOLD)) {

					// don't steal from someone else!
					if (!drawRoutine._volume_sliders[j].button->isPressed()) { 
						Pt pt = b2->Offset((*global_point_tracker)[i].getCurrentPosition());
						global_colors.ReAssignColor(id,j+1); 
						b2->press(global_colors.GetColor(id),id,pt);
					}

					// don't click more than one button!!!
					break;
				}
		  }
		}
	}
	for(unsigned int j=0; j<drawRoutine._buttons.size(); j++){
		if(drawRoutine._buttons[j]->isPressed()){
			std::cout << "control button pressed!" << std::endl;

			UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
    	args->osc_port ) );
    	char buffer[OUTPUT_BUFFER_SIZE];
	
    	osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
    	p << osc::BeginBundleImmediate 
	  << osc::BeginMessage( "ctrl" ) 
	  << (int)j
	  << osc::EndMessage;
	transmitSocket.Send( p.Data(), p.Size() );
      
        const std::string &hold1 = drawRoutine._buttons[j]->getText(0);
      if(hold1 == "Pitchshifter"){
      	const std::string &hold2 = drawRoutine._buttons[j]->getText(1);
      	if(hold2 == "OFF"){
      		drawRoutine._buttons[j]->clearText();
      		drawRoutine._buttons[j]->addText("Pitchshifter");
      		drawRoutine._buttons[j]->addText("ON");
    		}else{
    			drawRoutine._buttons[j]->clearText();
      		drawRoutine._buttons[j]->addText("Pitchshifter");
    			drawRoutine._buttons[j]->addText("OFF");
  			}
      }
      //std::cout << "here it be: " << hold << std::endl;
			drawRoutine._buttons[j]->release();
		}
	}
}

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
	drawRoutine.initialize_sliders();

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
  if (args->tiled_display.is_tiled_display) {
    log_keypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  } else {
    keyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  }
}

void specialkeyfunc(int key, int x, int y) {
  //std::cout << "RECEIVED SPECIAL KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    log_specialkeypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  } else {
    specialkeyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  }
}


void mousefunc(int button, int state, int x, int y) {
  //std::cout << "RECEIVED MOUSE DIRECTLY: " << button << " " << state << " " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    // adjust position of click
    int from_top = args->tiled_display.full_display_height - 
    	(args->tiled_display.my_height+args->tiled_display.my_bottom);
    y += from_top;
    log_mouseaction(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
  } else {
    mousefunc_helper(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
  }
}

void motionfunc(int x, int y) {
  //std::cout << "RECEIVED MOTION DIRECTLY: " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    if (x == 0 && y == 0) {
      //std::cout << "IGNORE: OFFSCREEN MOTION BUG" << std::endl;
      return;
    }
    int from_top = args->tiled_display.full_display_height - 
    	(args->tiled_display.my_height+args->tiled_display.my_bottom);
    y += from_top;
    log_mousemotion(PRIMARY_MOUSE,x,y );//,glutGetModifiers());
  } else {
    motionfunc_helper(PRIMARY_MOUSE,x,y,glutGetModifiers());
  }
}


//////////////////////////////////////////////////////////////

// this will get called eventually :)
void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers) {
  //std::cout << "RECEIVED KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x 		<< " " << y << " " << glut_modifiers << std::endl;
  if (key == 'q') {
    exit(0);
  }
  if(key == 'b'){
    //for(unsigned int i = 0; i < piece_pointers.size(); ++i){
    //  piece_pointers[i]->ResetBorder();
    //}
  }
  else {
    //std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
  display();
}


void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers) {
  //std::cout << "RECEIVED SPECIAL KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glut_modifiers << std::endl;

}

void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers) {
  //std::cout << "RECEIVED MOUSE FROM LOGGER: " << which_mouse << " " << button << " " << state << " " << x << " " << y << " " << glut_modifiers << std::endl;

  if (state == GLUT_DOWN) {
  	drawRoutine.set_clicked(which_mouse-201,true);
    TryToPressButton(which_mouse,x,y);
  }
  if (state == GLUT_UP) {
  	drawRoutine.set_clicked(which_mouse-201,false);
    TryToReleaseButton(which_mouse,x,y);
  }

}

void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers) {
  //std::cout << "RECEIVED MOTION FROM LOGGER: " << which_mouse << " " << x << " " << y << 			" " << glut_modifiers << std::endl;

  TryToMoveButton(which_mouse,x,y);

}
// ===================================================================
 

int  main(int argc, char **argv) {

  std::vector<Vec3f> colors;
  colors.push_back(Vec3f(1,1,1));
  colors.push_back(Vec3f(1,0.5,0));
  colors.push_back(Vec3f(1,0.5,0.5));
  colors.push_back(Vec3f(0,0.5,1));
  colors.push_back(Vec3f(0.5,0,1));
	
  global_colors = Colors(colors);

  args = new ArgParser(argc,argv);
  global_point_tracker = NULL;
  global_calibration_data = new 
  	PlanarCalibration	(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,
  	PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);

  //args->width = global_calibration_data->getWidth();
  //args->height = global_calibration_data->getHeight();
  
  UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
  	args->osc_port ) );
  char buffer[OUTPUT_BUFFER_SIZE];
  
  osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
  p << osc::BeginBundleImmediate 
    << osc::BeginMessage( "vc" ) 
		<< "blablabla"
    << osc::EndMessage;
  transmitSocket.Send( p.Data(), p.Size() );
  std::cout << "blablabla" << std::endl;
  
	
  args->tiled_display.set_from_calibration_data(global_calibration_data);
  
  global_colors.AssignRandomAvailableColor(PRIMARY_MOUSE);
	global_colors.AssignRandomAvailableColor(MOUSE_2);
	global_colors.AssignRandomAvailableColor(MOUSE_3);
	global_colors.AssignRandomAvailableColor(MOUSE_4);
	global_colors.AssignRandomAvailableColor(MOUSE_5);
	global_colors.AssignRandomAvailableColor(MOUSE_6);

  // initialize things...
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);
  glutInitWindowPosition(20,20);
  glutCreateWindow("planar calibration");
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

  HandleGLError("BEFORE INITIALIZE BUTTONS");

  //initialize_buttons();
  HandleGLError("AFTER INITIALIZE BUTTONS");
  std::cout << "FINISHED INITIALIZING THE BUTTONS" << std::endl;
  
  if(args->include_lasers){
    global_point_tracker = new PointTracker(global_calibration_data,
					  AddTrackedPoint,RemoveTrackedPoint, 20,0.5);
  }
  
  drawRoutine = vcDraw(args, global_colors, global_point_tracker);
  HandleGLError("BEFORE INITIALIZE BUTTONS");
	drawRoutine.initialize_sliders();
  HandleGLError("AFTER INITIALIZE BUTTONS");
  
  glutMainLoop();

  return 0;
}


