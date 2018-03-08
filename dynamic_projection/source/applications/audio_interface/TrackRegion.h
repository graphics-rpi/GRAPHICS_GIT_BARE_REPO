#ifndef _TRACKREGION_H_
#define _TRACKREGION_H_

#include "Track.h"
#include <string>
#include <vector>

//holds information for all tracks, and every other object in the track region.
//includes all tracks
//has drawing routines for the track region
//has drawing routines for track connections
//maintains track positions/organizations

class TrackRegion{
public:

	//Public Methods================================

	TrackRegion(double width, double height, Pt pos){
		TRW = width;
		TRH = height;
		TRPos = pos;
		std::string masterId = "master";
		std::string drumId = "drum";
		ghostTrackListItr = normalTracks.begin();
		hasGhostTrack = false;
		numNormalTracks = 0;


		//Track(bool removable, Pt tp, double tw, Vec3f c){
		masterTrack = new Track(masterId, Pt(TRPos.x + (TRW/2) + (TRW/10), TRPos.y + (TRH/2)), TRW/3, Vec3f(1.0,1.0,1.0), TRH);
		drumTrack = new Track(drumId, Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, Vec3f(1.0,1.0,1.0), TRH);
		//drumTrack->setTransparency(0.1);
		//drumTrack->setDashed(true);
		//addButtonTrack = new Track(false, Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, Vec3f(1.0,1.0,0.0));

		verticallyArrangeTracks();
	}

	void moveAndResize(double width, double height, Pt pos);

	void trashPolyButton(Cursor *c, void *objPtr);
	Track* trashTrack(Cursor *c, void *objPtr);

	void* tryToPressObj(Cursor *c);
	void tryToExpandObj(Cursor* c, void *objPtr);
	void tryToMoveObj(Cursor *c, void *objPtr, std::string &objType);


	void draw(){
		drawBorder();
		drawTracks();
		drawTrackConnections();
	}

	void initializeTracks(){ 
		masterTrack->initializePolyButtons();
		drumTrack->initializePolyButtons();
	}

	void insertNewTrack(bool isDashed, std::list<Track*>::iterator itr);
	void insertOldTrack(bool isDashed, std::list<Track*>::iterator itr, Track* track);
	//void pushBackTrack(bool isDashed);
	void removeGhostTrack();
	void removeLastAddedTrack();
	void getLastTrackNumAndId(std::string& num, std::string& id);
	const std::list<Track*>& getNormalTracks(){ return normalTracks; }
	const Track* getMasterTrack(){ return masterTrack; }
	const Track* getDrumTrack(){ return drumTrack; }

	bool tryToAddTrack(bool isDashed, Pt pos, void* trackButtonFromTrash);
	void tryToAddEffect(Pt pos, std::string effectName, double bankButtonHeight);


protected:

	//Protected Methods=============================

	void drawBorder();
	void drawTracks();
	void drawTrackConnections();
	void verticallyArrangeTracks();



	//Member Variables===============================

	double TRW; //track region width
	double TRH; //track region height
	Pt TRPos;		//track region position
	std::list<Track*>::iterator ghostTrackListItr;
	bool hasGhostTrack;
	std::list<Track*> normalTracks;
	int numNormalTracks;
	//std::list<Button*> ghostElements;
	Track* masterTrack;
	Track* drumTrack;


};



#endif
