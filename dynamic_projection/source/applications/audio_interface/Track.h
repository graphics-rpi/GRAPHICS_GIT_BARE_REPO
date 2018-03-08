#ifndef _TRACK_H_
#define _TRACK_H_

#include "PolyButton.h"
#include <string>
#include <vector>

//holds information for a single track.
//holds polybutton objects for a single track's effects.
//has drawing routine for individual tracks with button/effects

class Track{
public:

	//Default Constructor
	//Creates all potential polybuttons, doesn't draw or use any
	//unless they are added and armed later.
	Track(std::string &id, Pt tp, double tw, Vec3f c, double TRH){
		//isRemovable = removable;
		//this->setConnected(true);
		trackPos = tp;
		trackWidth = tw;
		trackColor = c;
		trackId = id;
		//trackId = id;
		isRemovable = true;
		transparency = 1.0;
		isDashed = false;
		trackRegionHeight = TRH;

		if(trackId == "master" || trackId == "drum"){
			isRemovable = false;
		}else{
			trackIdLabel = new FadeLabel(tp, Vec3f(0.0,0.0,0.0), trackId, TRH/26, TRH/26, false);
		}

		polyButtonsInitialized = false;
		
	}

	void moveAndResize(Pt tp, double tw, double TRH);
	void tryToMoveTrack(Pt pos);


	void initializePolyButtons();

	Pt getPos(){ return trackPos; }
	Vec3f getColor(){ return trackColor; }
	double getWidth(){ return trackWidth; }
	double getTransparency(){ return transparency; }
	bool getDashed(){ return isDashed; }
	bool getConnected(){ return trackPolyButton->getConnected(); }
	std::string getId(){ return trackId; }

	void getAddedEffects(std::vector<PolyButton*> &tEffects) const;
	void getAddedEffectArray(std::vector<bool> &tEffects) const{
		for(unsigned int i=0; i<effectAdded.size(); i++){
			tEffects.push_back(effectAdded[i]);
		}
	}

	PolyButton* getTrackButton(){ return trackPolyButton; }
	void getParamNames(std::vector< std::vector< std::string > >& paramNames) const;
	void getParamValues(std::vector< std::vector< int > >& paramValues) const;

	

	void setTransparency(double t){ transparency = t; }
	void setDashed(bool d){ isDashed = d; }
	void setConnected(bool c){ trackPolyButton->setConnected(c); }
	void setButtonCollapsed(bool c){ trackPolyButton->setCollapsed(c); }
	void collapseEffects();
	void setId(std::string id){ 
		trackId = id;
		trackIdLabel->setText(trackId);
	}
	
	void draw(){
		drawTrackBase();
		drawExpandedElements();
	}

	void drawTrackBase();
	void drawExpandedElements();

	void drawTrackLine();
	void drawEffects();
	void drawCollapsedEffects();
	void drawExpandedEffects();
	void addEffect(std::string name);
	bool tryToRemoveEffect(PolyButton *pb);

	void verticalMove(double newY);

protected:

	//bool isConnected;	//determines if track is muted or not
	bool isRemovable; //determines if track can be deleted
	Vec3f trackColor;
	Pt trackPos;
	double trackWidth;
	double trackRegionHeight;
	std::string trackId;
	//int trackId;
	double transparency;
	bool isDashed;
	bool polyButtonsInitialized;


	PolyButton *trackPolyButton;
	FadeLabel* trackIdLabel;
	std::vector<PolyButton*> effects;
	std::vector<bool> effectAdded;

};



#endif
