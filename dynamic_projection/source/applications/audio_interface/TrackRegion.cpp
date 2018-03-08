#include "TrackRegion.h"

void TrackRegion::moveAndResize(double width, double height, Pt pos){

	TRW = width;
	TRH = height;
	TRPos = pos;

	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		(*itr)->moveAndResize(Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, TRH);
	}

	drumTrack->moveAndResize(Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, TRH);
	//Track *holder = new Track(id, Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, Vec3f(1.0,1.0,1.0), TRH);
	masterTrack->moveAndResize(Pt(TRPos.x + (TRW/2) + (TRW/10), TRPos.y + (TRH/2)), TRW/3, TRH);

	verticallyArrangeTracks();

}

//search for polybutton in tracks.
//trash it if found
void TrackRegion::trashPolyButton(Cursor *c, void *objPtr){
	//std::vector<PolyButton*> tEffects;
	PolyButton *pb = (PolyButton*)objPtr;

	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		if((*itr)->tryToRemoveEffect(pb)){
			c->setGrabbedVoidObj(NULL);
			std::string objType = "NULL";
			c->setObjType(objType);
			return;
		}
	}
	if(drumTrack->tryToRemoveEffect(pb)){
		c->setGrabbedVoidObj(NULL);
		std::string objType = "NULL";
		c->setObjType(objType);
		return;
	}
	if(masterTrack->tryToRemoveEffect(pb)){
		c->setGrabbedVoidObj(NULL);
		std::string objType = "NULL";
		c->setObjType(objType);
		return;
	}
}

//search for polybutton in tracks.
//trash it if found
Track* TrackRegion::trashTrack(Cursor *c, void *objPtr){
	//std::cout << "tried to kill track" << std::endl;


	Track *t = (Track*)((PolyButton*)objPtr)->getTrack();
	assert(t != NULL);

	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); itr++){
		if(t == (*itr)){
			normalTracks.erase(itr);
			verticallyArrangeTracks();
			return t;
		}
	}
	return NULL;
}

void* TrackRegion::tryToPressObj(Cursor* c){
	std::vector<PolyButton*> tEffects;
	void *objPtr = NULL;

	//normal tracks=================================================
	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		(*itr)->getAddedEffects(tEffects);

		for(unsigned int i=0; i<tEffects.size(); i++){
			objPtr = tEffects[i]->tryToPressPolyButton(c);
			if(objPtr != NULL){
				return objPtr;
			}
		}
		objPtr = (*itr)->getTrackButton()->tryToPressPolyButton(c);
		if(objPtr != NULL){
			return objPtr;
		}
	}
	
	//drum track====================================================
	drumTrack->getAddedEffects(tEffects);
	for(unsigned int i=0; i<tEffects.size(); i++){
		objPtr = tEffects[i]->tryToPressPolyButton(c);
		if(objPtr != NULL){
			return objPtr;
		}
	}
	objPtr = drumTrack->getTrackButton()->tryToPressPolyButton(c);
	if(objPtr != NULL){
		return objPtr;
	}

	//master track=================================================
	masterTrack->getAddedEffects(tEffects);
	for(unsigned int i=0; i<tEffects.size(); i++){
		objPtr = tEffects[i]->tryToPressPolyButton(c);
		if(objPtr != NULL){
			return objPtr;
		}
	}

	return NULL;
}

void TrackRegion::tryToExpandObj(Cursor* c, void *objPtr){
	//TRY TO EXAPAND OBJECT ==============================
	//--expansion of polyButton
	//--movement of polyButton back to original position if moved and released
	//====================================================
	((PolyButton*)objPtr)->tryToExpandPolyButton(c);

}

void TrackRegion::tryToMoveObj(Cursor *c, void *objPtr, std::string &objType){

	if(objType == "PolyButton" && c->isStateDown()){
		((PolyButton*)objPtr)->tryToMovePolyButton(c->getScreenPosition());
	}else if(objType == "NormalTrackButton" && c->isStateDown()){
		Track *t = (Track*)((PolyButton*)objPtr)->getTrack();
		t->tryToMoveTrack(c->getScreenPosition());
	}else if(objType == "DrumTrackButton" && c->isStateDown()
				&& !((PolyButton*)objPtr)->isCollapsed()){
		Track *t = (Track*)((PolyButton*)objPtr)->getTrack();
		t->tryToMoveTrack(c->getScreenPosition());
	}
}

void TrackRegion::verticallyArrangeTracks(){

	std::vector<double> newHeights;
	double trackNum = normalTracks.size()+1;
	double trackGap = 6;

	if(trackNum >= 6){
		trackGap = trackNum;
	}

	for(unsigned int i=0; i<normalTracks.size()+1; i++){
		newHeights.push_back((TRPos.y+TRH/2) - (((trackNum-1) * TRH/trackGap)/2) + (i * TRH/trackGap));
	}
	
	assert(newHeights.size() >= (normalTracks.size() + 1));

	drumTrack->verticalMove(newHeights[0]);
	//for(unsigned int i=0; i<normalTracks.size(); i++){
	std::list<Track*>::iterator itr;
	unsigned int i = 0;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		(*itr)->verticalMove(newHeights[i+1]);
		i++;
	}
	//addButtonTrack->verticalMove(newHeights[newHeights.size()-1]);

}

void TrackRegion::insertNewTrack(bool isDashed, std::list<Track*>::iterator itr){
	std::string ghostId = "";
	Track *holder = new Track(ghostId, Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, Vec3f(1.0,1.0,1.0), TRH);
	holder->initializePolyButtons();
	holder->setDashed(isDashed);

	//ghostTrackAdded = true;
	if(isDashed){
		ghostTrackListItr = normalTracks.insert(itr,holder);
	}

}

void TrackRegion::insertOldTrack(bool isDashed, std::list<Track*>::iterator itr, Track* track){
	// std::string ghostId = "";
	// Track *holder = new Track(ghostId, Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, Vec3f(1.0,1.0,1.0), TRH);
	// holder->initializePolyButtons();
	track->setDashed(isDashed);

	//ghostTrackAdded = true;
	if(isDashed){
		ghostTrackListItr = normalTracks.insert(itr,track);
		std::cout << "got here before crash" << std::endl;
	}
}

void TrackRegion::removeGhostTrack(){

	normalTracks.erase(ghostTrackListItr);
	hasGhostTrack = false;

}

void TrackRegion::removeLastAddedTrack(){
	std::stringstream ss;
	ss << char(numNormalTracks + 64);

	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		if((*itr)->getId() == ss.str()){
			//std::cout << "found match" << std::endl;

			numNormalTracks--;
			normalTracks.erase(itr);
			verticallyArrangeTracks();
			return;
		}
	}
}

void TrackRegion::getLastTrackNumAndId(std::string& num, std::string& id){
	std::stringstream ss;
	ss << char(numNormalTracks + 64);

	std::list<Track*>::iterator itr;
	int i = 1;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		if((*itr)->getId() == ss.str()){
			//std::cout << "found match" << std::endl;

			std::stringstream convert;
			convert << i;
			num = convert.str(); 
			id = ss.str();
			return;
		}
		i++;
	}
}

bool TrackRegion::tryToAddTrack(bool isDashed, Pt pos, void* trackButtonFromTrash){

	//if the pos is within the "add track" x and y boundaries
	if(pos.x >= drumTrack->getPos().x
	&& pos.x <= (drumTrack->getPos().x + drumTrack->getWidth())
	&& pos.y >= TRPos.y && pos.y <= (TRPos.y + TRH)){

		//if new track is not dashed, that means the ghost must turn
		//into a full track (happens on a cursor release)
		if(!isDashed){
			if(hasGhostTrack){


				(*ghostTrackListItr)->setDashed(false);
				//void moveAndResize(Pt tp, double tw, double TRH);
				if(trackButtonFromTrash != NULL){
					(*ghostTrackListItr)->moveAndResize(Pt(TRPos.x + (TRW/10), TRPos.y + (TRH/2)), TRW/3, TRH);
					(*ghostTrackListItr)->setConnected(true);
					verticallyArrangeTracks();
				}else{
					numNormalTracks++;
					//set id as alpha capital character coresponding to 
					//the number of tracks 
					std::string id;
					id.push_back(char(numNormalTracks + 64));
					(*ghostTrackListItr)->setId(id);
				}
				hasGhostTrack = false;

			}
			return true;
		}
		//if track is dashed ghost

		std::list<Track*>::iterator itr;
		std::list<Track*>::iterator newGhostItr = normalTracks.begin();
		bool isBottom = true;
		for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
			if(pos.y > (*itr)->getPos().y){
				isBottom = false;
				newGhostItr = itr;					
			}
		}

		bool moveGhost = true;

		//check to see if line below cursor is the ghost track
		if(hasGhostTrack && newGhostItr == ghostTrackListItr){
			moveGhost = false;
		}

		if(newGhostItr != normalTracks.end() && !isBottom){
		 	newGhostItr++;
		}

		//now newGhostItr has been incremented (if needed) track above
		//check to see if line above cursor is the ghost track
		if(hasGhostTrack && newGhostItr == ghostTrackListItr){
			moveGhost = false;
		}

		//return if ghost doesn't need to move
		if(!moveGhost){
			return false;
		}

		//if ghost track exists and needs to move
		//need to remove old ghost track
		if(hasGhostTrack){
			removeGhostTrack();
		}
		if(trackButtonFromTrash == NULL){
			insertNewTrack(true, newGhostItr);
		}else{
			Track *t = (Track*)((PolyButton*)trackButtonFromTrash)->getTrack();
			insertOldTrack(true, newGhostItr, t);
		}
		hasGhostTrack = true;

	}else{

		if(hasGhostTrack){
			//std::cout << "got here" << std::endl;
			removeGhostTrack();
			hasGhostTrack = false;
		}
	}


	verticallyArrangeTracks();
	return false;
}

void TrackRegion::tryToAddEffect(Pt cPos, std::string effectName, double bankButtonHeight){

	Pt tPos;
	//double tWidth;

	double bDist = bankButtonHeight/2;
	double currentDist;
	double closestDist = bDist + 1.0;
	Track *closestTrack = NULL;

	//if the x pos is within the correct region for a normal track or drum track
	if(cPos.x >= drumTrack->getPos().x
	&& cPos.x <= (drumTrack->getPos().x + drumTrack->getWidth())){

		//Normal Track connections
		std::list<Track*>::iterator itr;
		for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
			if(!(*itr)->getDashed()){
				tPos = (*itr)->getPos();
				//tWidth = (*itr)->getWidth();
				currentDist = abs(tPos.y - cPos.y);
				//if distance is less than half the bank button height, and is the smallest dist yet
				if(currentDist < bDist && currentDist < closestDist){
					closestDist = currentDist;
					closestTrack = (*itr);
				}
			}
		}

		tPos = drumTrack->getPos();
		currentDist = abs(tPos.y - cPos.y);
		//if distance is less than half the bank button height, and is the smallest dist yet
		if(currentDist < bDist && currentDist < closestDist){
			closestDist = currentDist;
			closestTrack = drumTrack;
		}

		//if the cursor was over a track
		if(closestTrack != NULL){
			closestTrack->addEffect(effectName);
		}

	//if the x pos is within the correct region for the master track
	}else if(cPos.x >= masterTrack->getPos().x
	&& cPos.x <= (masterTrack->getPos().x + masterTrack->getWidth())){
		tPos = masterTrack->getPos();
		currentDist = abs(tPos.y - cPos.y);
		//if distance is less than half the bank button height
		if(currentDist < bDist){
			masterTrack->addEffect(effectName);
		}

	}

}

void TrackRegion::drawBorder(){

	//outer border lines
	glLineWidth(2.0);
	glBegin(GL_LINE_LOOP);
		
		glColor3f(1.0,1.0,1.0);
		glVertex2f(TRPos.x, TRPos.y);
		glVertex2f(TRPos.x, TRPos.y+TRH);
		glVertex2f(TRPos.x+TRW, TRPos.y+TRH);
		glVertex2f(TRPos.x+TRW, TRPos.y);
		glVertex2f(TRPos.x, TRPos.y);
		
	glEnd();

}

void TrackRegion::drawTrackConnections(){
	Pt master_pos = masterTrack->getPos();
	Vec3f master_color = masterTrack->getColor();

	//drum track connection
	Pt end = drumTrack->getPos();
	Vec3f color = drumTrack->getColor();
	end.x += drumTrack->getWidth();

	//anti-aliasing
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	glLineWidth(4.0);
	glBegin(GL_LINES);
		glColor3f(color.x(), color.y(), color.z());
		glVertex2f(end.x, end.y);
		glColor3f(master_color.x(), master_color.y(), master_color.z());
		glVertex2f(master_pos.x, master_pos.y);
	glEnd();


	//Normal Track connections
	//for(unsigned int i=0; i<normalTracks.size(); i++){
	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){

		if((*itr)->getConnected()){
			//drum track connection
			Pt end = (*itr)->getPos();
			Vec3f color = (*itr)->getColor();
			end.x += (*itr)->getWidth();

			if((*itr)->getDashed()){
				glLineStipple(10, 0xAAAA);
				glEnable(GL_LINE_STIPPLE);
			}

			glLineWidth(4.0);
			glBegin(GL_LINES);
				glColor3f(color.x(), color.y(), color.z());
				glVertex2f(end.x, end.y);
				glColor3f(master_color.x(), master_color.y(), master_color.z());
				glVertex2f(master_pos.x, master_pos.y);
			glEnd();

			if((*itr)->getDashed()){
				glDisable(GL_LINE_STIPPLE);
			}
		}
	}

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void TrackRegion::drawTracks(){

	//DRAW TRACK BASES FIRST==========================================

	//masterTrack line
	masterTrack->drawTrackBase();

	//drumTrack line
	drumTrack->drawTrackBase();

	//normal Tracks lines
	std::list<Track*>::iterator itr;
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		(*itr)->drawTrackBase();
	}

	//DRAW EXPANDED ELEMENTS LAST (ON TOP)===============================

	//masterTrack line
	masterTrack->drawExpandedElements();

	//drumTrack line
	drumTrack->drawExpandedElements();

	//normal Tracks lines
	for(itr = normalTracks.begin(); itr != normalTracks.end(); ++itr){
		(*itr)->drawExpandedElements();
	}
}
