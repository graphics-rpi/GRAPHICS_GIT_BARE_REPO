// audio_interface main

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
#include "rdtsc.h"
//#include "../../common/Image.h"
#include "PolyInterface.h"
#include "PolyButton.h"
#include "Track.h"
#include "TrackRegion.h"
#include "Bank.h"
#include "TrashCan.h"
#include "Controls.h"
#include "RecordDialog.h"
#include "stdint.h"

#include "../../multi_cursor/interaction.h"
#include "../../multi_cursor/cursor.h"
#include "../../multi_cursor/key_and_mouse_logger.h"

//for OSC library
#include "../../common/oscpack/osc/OscOutboundPacketStream.h"
#include "../../common/oscpack/ip/UdpSocket.h"

#define ADDRESS "128.213.17.125"
#define PORT 7000
#define OUTPUT_BUFFER_SIZE 1024

#define DISPLAY_WIDTH 1920.0
#define DISPLAY_HEIGHT 1200.0

#define IR_STATE_DIRECTORY                "../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"

#define MK_STATE_DIRECTORY                   "../state/mouse_and_keyboard"
#define MK_ACTION_FILENAME_TEMPLATE          "../state/mouse_and_keyboard/actions_XXX.txt"
#define APPLICATIONS_STATE_DIRECTORY            "../state/applications/"

#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args;
//DirLock global_dirlock(IR_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;

TrackRegion *TR;
Bank *bank;
TrashCan *trash;
Controls *ctrl;
RecordDialog *record;
double secondsRecorded = 0.0;

uint64_t timerStart = 0.0;
uint64_t timerEnd = 0.0;

/*
unsigned long long timerStart = 0.0;
unsigned long long timerEnd = 0.0;
*/
//bool RECORDING = false;

std::vector<PolyButton*> polyButtons;
std::vector<Path> trackStrokes;
std::vector<FadeLabel> trackLabels;
std::vector< std::list<Pt> > strokePtLists;
std::vector< int > trackCheckSums(6,-1); // = {-1, -1, -1, -1, -1, -1};

int glutLoopNumber = 0;


//Mouse Globals======= 
//DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);
//DirLock global_mk_dirlock(MK_STATE_DIRECTORY);

// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

void keyfunc(unsigned char key, int x, int y);
void specialkeyfunc(int key, int x, int y);		
void mousefunc(int button, int state, int x, int y);
void motionfunc(int x, int y);

//void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers);
//void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers);		 
//void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers);
//void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers);



// Included to make Interaction interface work
void AddTrackedPoint(TrackedPoint const&)
{

} // end of AddTrackedPoint

void RemoveTrackedPoint(TrackedPoint const&)
{

} // end of RemoveTrackedPoint

// ==========================================================================================
// DRAWING ROUTINES
// ==========================================================================================

void sendOSCMessage(std::vector< std::string >& oscArgs){

	assert(oscArgs.size() > 0);

	UdpTransmitSocket transmitSocket( IpEndpointName( args->osc_ip_address, 
		args->osc_port ) );
	char buffer[OUTPUT_BUFFER_SIZE];

	osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
	p << osc::BeginBundleImmediate
	  << osc::BeginMessage(oscArgs[0].c_str());
	for(unsigned int i=1; i<oscArgs.size(); i++){
		p << oscArgs[i].c_str();
	}
	p << osc::EndMessage;
	transmitSocket.Send( p.Data(), p.Size() );
	//oscArgs.clear();

}

void accessPolyButtonParams(std::vector< std::string >& oscArgs,
		std::vector< std::vector< std::string > >& paramNames,
		std::vector< std::vector< int > >& paramValues,
		std::vector< bool >& effectsAdded){

	std::stringstream ss;
	for(unsigned int i=0; i<paramNames.size(); i++){
		assert(paramNames[i].size() != 0);
		//if instrument polyButton
		oscArgs.push_back(paramNames[i].back());
		paramNames[i].pop_back();
		for(unsigned int j=0; j<paramNames[i].size(); j++){
			//std::cout << "name: " << paramNames[i][j] << std::endl;
			//std::cout << "valu: " << paramValues[i][j] << std::endl;
			ss.str("");
			ss << paramNames[i][j];// << " " << paramValues[i][j];
			oscArgs.push_back(ss.str());
			ss.str("");
			ss << paramValues[i][j];// << " " << paramValues[i][j];
			oscArgs.push_back(ss.str());
		}
	}
	oscArgs.push_back("EffectOnOff");
	for(unsigned int i=0; i<effectsAdded.size(); i++){
		if(i == 0){
			oscArgs.push_back("Dist");
		}else if(i == 1){
			oscArgs.push_back("Mod");
		}else if(i == 2){
			oscArgs.push_back("EQ");
		}


		if(effectsAdded[i] == true){
			ss.str("");
			ss << (int)127;
			oscArgs.push_back(ss.str());
		}else{
			ss.str("");
			ss << (int)0;
			oscArgs.push_back(ss.str());
		}
	}


}

bool trackHasChanged(std::vector< std::vector< int > >& paramValues,
					std::string& trackId, unsigned int index){
	int checksum = 0;
	for(unsigned int i=0; i<paramValues.size(); i++){
		for(unsigned int j=0; j<paramValues[i].size(); j++){
			checksum += paramValues[i][j];
		}
	}
	for(unsigned int i=0; i<trackId.size(); i++){
		checksum += (int)(char)trackId[i];
	}
	if(checksum == trackCheckSums[index]){
		return false;
	}else{
		trackCheckSums[index] = checksum;
		return true;
	}

}

void updatePdTracks(){

	std::list<Track*> normalTracks = TR->getNormalTracks();
	const Track* drumTrack = TR->getDrumTrack();
	const Track* masterTrack = TR->getMasterTrack();

	std::string trackNum;
	std::string trackId;
	std::stringstream ss;
	std::vector< std::string > oscArgs;
	std::vector< std::vector< std::string > > paramNames;
	std::vector< std::vector< int > > paramValues;
	std::vector< bool > effectsAdded;

	if(normalTracks.size() >= 4 || ctrl->getIsPlaying()){
		bank->setCanAddTrack(false);
	}else{
		bank->setCanAddTrack(true);
	}

	std::list<Track*>::iterator itr;
	unsigned int i = 1;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		ss.str("");
		ss << "track" << i;
		trackNum = ss.str();
		trackId  = "/tmp/" + (*itr)->getId() + ".mid";

		oscArgs.clear();
		effectsAdded.clear();
		oscArgs.push_back("/track");
		oscArgs.push_back(trackNum.c_str());
		oscArgs.push_back(trackId.c_str());
		(*itr)->getParamValues(paramValues);
		if(trackHasChanged(paramValues, trackId, i-1)){
			(*itr)->getParamNames(paramNames);
			(*itr)->getAddedEffectArray(effectsAdded);
			accessPolyButtonParams(oscArgs, paramNames, paramValues, effectsAdded);
			sendOSCMessage(oscArgs);
			if(i == 1){
				sendOSCMessage(oscArgs);
				// sendOSCMessage(oscArgs);
				// sendOSCMessage(oscArgs);
			}
			std::cout << trackNum << " changed" << std::endl;
		}

		i++;
		if(i >= 5){ break; }
	}
	while(i < 5){
		ss.str("");
		ss << "track" << i;
		trackNum = ss.str();
		trackId  = "none";

		oscArgs.clear();
		effectsAdded.clear();
		oscArgs.push_back("/track");
		oscArgs.push_back(trackNum.c_str());
		oscArgs.push_back(trackId.c_str());
		std::vector< std::vector< int > > blankVec;
		if(trackHasChanged(blankVec, trackId, i-1)){
			sendOSCMessage(oscArgs);
			std::cout << trackNum << " [no recording] changed " << trackCheckSums[i-1] << std::endl;
		}
		//sendOSCMessage(oscArgs);
		i++;
	}
	ss.str("");
	assert(i == 5);

	//DRUM TRACK Info
	oscArgs.clear();
	effectsAdded.clear();
	oscArgs.push_back("/track");
	trackId = "drum";
	oscArgs.push_back(trackId.c_str());
	drumTrack->getParamValues(paramValues);
	if(trackHasChanged(paramValues, trackId, i-1)){
		drumTrack->getParamNames(paramNames);
		drumTrack->getAddedEffectArray(effectsAdded);
		accessPolyButtonParams(oscArgs, paramNames, paramValues, effectsAdded);
		//accessPolyButtonParams(oscArgs, paramNames, paramValues);
		sendOSCMessage(oscArgs);
		std::cout << "drum's changed " << trackCheckSums[i-1] << std::endl;
	}
	// drumTrack->getParamNames(paramNames);
	// accessPolyButtonParams(oscArgs, paramNames, paramValues);
	// sendOSCMessage(oscArgs);


	//MASTER TRACK Info
	i++;
	oscArgs.clear();
	effectsAdded.clear();
	oscArgs.push_back("/track");
	trackId = "master";
	oscArgs.push_back(trackId.c_str());
	masterTrack->getParamValues(paramValues);
	if(trackHasChanged(paramValues, trackId, i-1)){
		masterTrack->getParamNames(paramNames);
		masterTrack->getAddedEffectArray(effectsAdded);
		accessPolyButtonParams(oscArgs, paramNames, paramValues, effectsAdded);
		sendOSCMessage(oscArgs);
		sendOSCMessage(oscArgs);
		sendOSCMessage(oscArgs);
		std::cout << "master's changed " << trackCheckSums[i-1] << std::endl;
	}
	// masterTrack->getParamNames(paramNames);
	// accessPolyButtonParams(oscArgs, paramNames, paramValues);
	// sendOSCMessage(oscArgs);

}

void drawTracksDemo(double fdh){
	for(unsigned int i=0; i<trackStrokes.size(); i++){
		trackStrokes[i].draw();
		trackLabels[i].render(fdh);
	}
	
}

void draw() { 

	//INITIALIZE GL================================================
	static GLfloat amb[] =  {0.4, 0.4, 0.4, 0.0};
	static GLfloat dif[] =  {1.0, 1.0, 1.0, 0.0};

	/*
	double fdw = args->tiled_display.full_display_width;
	double fdh = args->tiled_display.full_display_height;
	*/

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

		//initialize bank elements with their textures
		trash->initializeElements();
		bank->initializeElements();
		record->initializeElements();
		ctrl->initializeElements();
		TR->initializeTracks();

		glutLoopNumber++;

	}else if(glutLoopNumber < 15){
		glutLoopNumber++;
	}else{ //glutLoopNumber > 15
		//if(!record->isRecording()){
		trash->draw();
		ctrl->draw();
		TR->draw();
		bank->draw();
		if(!record->isInDialog()){
			updatePdTracks();
		}

		if(record->isInDialog() && !record->isRecording()){
			record->draw();
		}
		else if(record->isInDialog() && record->isRecording()){
			double secondsTotal = record->getRecTime();
			record->drawRecordBar((double)(secondsRecorded/secondsTotal), secondsRecorded);

			//timer since the last iteration throught the glutLoop
			timerEnd = rdtsc();
			uint64_t totalCycles;
			if(timerStart <= 0.0){
				//first time through this loop
				//START RECORDING
				// if(ctrl->getIsPlaying()){
				// 	//Send OSC Message
				// 	std::vector< std::string > oscArgs;
				// 	oscArgs.push_back( "/stop" );
				// 	sendOSCMessage(oscArgs);
				// 	ctrl->setIsPlaying(false);
				// }
				totalCycles = 0.0;

				//Send OSC Message
				std::string trackNum;
				std::string trackId;
				TR->getLastTrackNumAndId(trackNum, trackId);
				trackNum = "track" + trackNum;
				if(trackId != ""){
					trackId ="/tmp/" + trackId + ".mid";
				}else{
					trackId = "none";
				}
				std::vector< std::string > oscArgs;
				oscArgs.push_back( "/record" );
				oscArgs.push_back(trackNum.c_str());
				oscArgs.push_back(trackId.c_str());
				sendOSCMessage(oscArgs);
				//updatePdTracks();

			}else{
				totalCycles = timerEnd - timerStart;
			}
			//divide by cycles per second
			//adjusted for 3.0 GHz on Dozer
			double totalSeconds = totalCycles / (3.000 * 1000000000);
			secondsRecorded += totalSeconds;
			timerStart = rdtsc();

			if(secondsRecorded >= secondsTotal){
				//STOP RECORDING
				secondsRecorded = 0.0;
				timerStart = 0.0;
				record->setInDialog(false);
				record->setRecording(false);

				//Send OSC Message
				std::string trackNum;
				std::string trackId;
				TR->getLastTrackNumAndId(trackNum, trackId);
				trackNum = "track" + trackNum;
				std::vector< std::string > oscArgs;
				oscArgs.push_back( "/stoprec" );
				oscArgs.push_back(trackNum.c_str());
				//oscArgs.push_back(trackId.c_str());
				sendOSCMessage(oscArgs);
				//updatePdTracks();
			}
		}


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
// IDLE
// ==========================================================================================

void idle(void) {

  // Update the gestures, call the appropriate callback
  Interaction::determineCallBacks();


/*	if (args->tiled_display.is_tiled_display || 1) {
    load_and_save_key_and_mouse_data(global_mk_dirlock,MK_ACTION_FILENAME_TEMPLATE,
				     Interaction::keyfunc_helper,
				     Interaction::specialkeyfunc_helper,
				     Interaction::mousefunc_helper,
				     Interaction::motionfunc_helper);
  }
*/


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

//====================================================================================
//MOUSE BUTTON INTERACTION CALLS======================================================
//====================================================================================

void TryToPressObj(int cursIndex) {

	Cursor *c = Interaction::getCursor( cursIndex );
	assert(c != NULL);
	std::cout << "PRESS BY: " << *c << std::endl;

	//for testing vertical rearrange
	//TR->pushBackTrack(false);
	//std::string objType;
	//	ClickableObject* objPtr = c->getClickedObject();
	//std::string objType = c->getObjType();

	//when not in recording dialog
	if(!record->isRecording() && !record->isInDialog()){
		//bank
		if(objPtr == NULL){
			objPtr = bank->tryToPressObj(c);
		}
		//track region
		if(objPtr == NULL || objType == "PolyButton"
					|| objType == "DrumTrackButton" 
					|| objType == "NormalTrackButton"){
			objPtr = TR->tryToPressObj(c);
		}
		//trash can
		if(objPtr == NULL){
			objPtr = trash->tryToPressObj(c,TR,bank,ctrl);
		}
		//controls
		if(objPtr == NULL){
			objPtr = ctrl->tryToPressObj(c);
		}
	}else if(!record->isRecording() && record->isInDialog()){
		//record dialog
		if(objPtr == NULL){
			objPtr = record->tryToPressObj(c);
		}
	}
}

void TryToReleaseObj(int cursIndex) {

	Cursor *c = Interaction::getCursor(cursIndex);
	//std::cout << "RELEASE BY: " << *c << std::endl;

	void* objPtr = c->getGrabbedVoidObj();
	std::string objType = c->getObjType();
	std::string nullType = "NULL";

	if(objPtr != NULL){

		//releasing a Bank Button
		if(objType == "BankButton"){

			//if track button, try to add a ghost track as a real track
			std::string buttonName = bank->getButtonName(objPtr);
			if(buttonName == "Track"){
				//RECORD STUFF========================================
				//here's where we need to record
				//====================================================
				if(TR->tryToAddTrack(false, c->getScreenPosition(), NULL)){
					//SetInDialog = true;
					record->setInDialog(true);
				}
			//if effect button, try to add an effect to a track
			}else{
				TR->tryToAddEffect(c->getScreenPosition(), buttonName, bank->getButtonHeight(objPtr));
			}

			bank->tryToReleaseObj(c, objPtr);
		}else if(objType == "PlayButton"){ 
			ctrl->tryToReleaseObj(c, objType);
			//SEND OSC MESSAGE
			std::vector< std::string > oscArgs;
			oscArgs.push_back( "/play" );
			sendOSCMessage(oscArgs);

		}else if(objType == "StopButton"){
			ctrl->tryToReleaseObj(c, objType);
			//SEND OSC MESSAGE
			std::vector< std::string > oscArgs;
			oscArgs.push_back( "/stop" );
			sendOSCMessage(oscArgs);

		}else if(objType == "PolyButton"
				|| objType == "DrumTrackButton" 
				|| objType == "NormalTrackButton"){
			//determine if the polyButton has been dragged to trash
			if(!(trash->pointIside(c->getScreenPosition()))){
	  		//TRY TO EXAPAND OBJECT ==============================
	  		//--movement of polyButton back to original position if moved and released
				TR->tryToExpandObj(c, objPtr);
			}else{
				if(objType == "PolyButton"){
					//throw away the polyButton
					TR->trashPolyButton(c, objPtr);
				}else if(objType == "NormalTrackButton"){
					//throw away the polyButton
					Track* t;
					t = TR->trashTrack(c, objPtr);
					if(t != NULL){
						trash->addTrack(t);
					}
				}
			}
		}else if(objType == "TrashTrackButton"){
			if(TR->tryToAddTrack(false, c->getScreenPosition(), objPtr)){
				//std::cout << "successful add track from trash" << std::endl;
				//if track was successfully added
				trash->tryToReleaseObj(c, objPtr);
			}else{
				//std::cout << "unsuccessful add track from trash" << std::endl;
				//if track wasn't added and needs to slide back to trash
				trash->slideBack(c, objPtr, objType);
			}
		}else if(objType == "RecordButton" || objType == "CancelButton"){
			if(!record->tryToReleaseObj(c, objPtr, objType)){
				TR->removeLastAddedTrack();
			}
		}
	}
}

void TryToMoveObj(int cursIndex) {

	Cursor *c = Interaction::getCursor(cursIndex);
	//std::cout << "MOVE FROM: " << *c << std::endl;

	ClickableObject* objPtr = c->getClickedObject();
	std::string objType = objPtr->getObjType();

	if(objPtr != NULL){
		//std::cout << "gets here pointer not null" << std::endl;

		//Moving a Bank Button
		if(objType == "BankButton"){
			bank->tryToMoveObj(c, objPtr);

			//if track button, try to add ghost track on hover
			std::string buttonName = bank->getButtonName(objPtr);
			if(buttonName == "Track"){
				TR->tryToAddTrack(true, c->getScreenPosition(), NULL);
			}
		}

		else if(objType == "PolyButton" 
				|| objType == "NormalTrackButton" 
				|| objType == "DrumTrackButton"){
			TR->tryToMoveObj(c, objPtr, objType);
			//make this work if you want to be able to move track orderings
			// if(objType == "NormalTrackButton"){
			// 	TR->tryToAddTrack(true, c->getScreenPosition(), objPtr);
			// }
		}

		else if(objType == "TrashTrackButton"){
			trash->tryToMoveObj(c, objPtr, objType);
			TR->tryToAddTrack(true, c->getScreenPosition(), objPtr);	
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
}

// ===================================================================
// MOUSE FUNCTIONS

void keyfunc(unsigned char key, int x, int y) {
  Interaction::keyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
}

void specialkeyfunc(int key, int x, int y) {
  Interaction::specialkeyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
}

void mousefunc(int button, int state, int x, int y) {
  //int flipped_y = -1 * (y - DISPLAY_HEIGHT);
  Interaction::mousefunc_helper(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
}

void motionfunc(int x, int y) {
  //int flipped_y = -1 * (y - DISPLAY_HEIGHT);
  Interaction::motionfunc_helper(PRIMARY_MOUSE,x,y,glutGetModifiers());
}

// ===================================================================
 

int main(int argc, char **argv) {

  args = new ArgParser(argc,argv);

  // Register callbacks
  Interaction::registerMove(TryToMoveObj);
  Interaction::registerSelect(TryToPressObj);
  Interaction::registerRelease(TryToReleaseObj);

//  global_point_tracker = NULL;
  global_calibration_data = new 
  	PlanarCalibration	(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,
  	PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);
  	
  	
	//std::vector<cursor*> cursorVec;
  Interaction::setupCursors(&(args->tiled_display));
  Interaction::invertCursorYAxis();

  trash = new TrashCan(120, 120, Pt(1650, 70),
  						770, 980, Pt(1000, 70));

  //TrackRegion(double trw, double trh, Pt pos){
  TR = new TrackRegion(1350, 900, Pt(250,250));

  //Bank()
  //bank = new Bank(1000, 500, Pt(100,50));
  bank = new Bank(800, 180, Pt(800,40));

  double rw = 500.0;
  double rh = 300.0;
  // double dispW = 1920.0;
  // double dispH = 1200.0;
  record = new RecordDialog(rw, rh, Pt((DISPLAY_WIDTH/2.0)-(rw/2.0),(DISPLAY_HEIGHT/2.0)-(rh/2.0)));

  ctrl = new Controls(500, 180, Pt(250, 40));
  
	
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
  
  glutMainLoop();

  return 0;
}


