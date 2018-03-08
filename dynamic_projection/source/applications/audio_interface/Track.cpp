#include "Track.h"

void Track::initializePolyButtons(){

	PolyButton *pb; //holder
	std::string image_filename;
	image_filename.assign("../source/applications/audio_interface/images/ppm/dist_texture.ppm");
	//DISTORTION polybutton initialization
	std::vector<std::string> distVertNames;
	distVertNames.push_back("Fuzzy");
	distVertNames.push_back("Light");
	distVertNames.push_back("Crisp");
	distVertNames.push_back("Loose");
	//PolyButton(Pt pos, std::string bText, double w, double h, double f, std::vector<std::string> &vn, Pt r, Vec3f color, std::string texture){
	pb = new PolyButton(Pt(trackPos.x+(trackWidth/4), trackPos.y),"Dist", 
		trackRegionHeight/4, trackRegionHeight/4, trackRegionHeight, distVertNames, Pt(0,32), Vec3f(0.3,0.3,0.3), image_filename);
	effects.push_back(pb);

	//MODULATION polybutton initialization	
	image_filename.assign("../source/applications/audio_interface/images/ppm/mod_texture.ppm");
	std::vector<std::string> modVertNames;
	modVertNames.push_back("Fast");
	modVertNames.push_back("Slow");
	modVertNames.push_back("High");
	modVertNames.push_back("Low");
	pb = new PolyButton(Pt(trackPos.x+(trackWidth/2),trackPos.y),"Mod", 
		trackRegionHeight/4, trackRegionHeight/4, trackRegionHeight, modVertNames, Pt(0,64), Vec3f(0.3,0.3,0.3), image_filename);
	effects.push_back(pb);

	//EQUALIZER polybutton initialization
	image_filename.assign("../source/applications/audio_interface/images/ppm/eq_texture.ppm");			
	std::vector<std::string> eqVertNames;
	eqVertNames.push_back("Bass");
	eqVertNames.push_back("Mid");
	eqVertNames.push_back("Treble");
	pb = new PolyButton(Pt(trackPos.x+(trackWidth*3)/4, trackPos.y),"EQ", 
		trackRegionHeight/4, trackRegionHeight/4, trackRegionHeight, eqVertNames, Pt(85,118), Vec3f(0.3,0.3,0.3), image_filename);
	effects.push_back(pb);

	//normal track buttons
	if(trackId != "master" && trackId != "drum"){

		//TRACK INSTRUMENT polybutton initialization
		image_filename.assign("../source/applications/audio_interface/images/ppm/track_button.ppm");			
		std::vector<std::string> instrumentVertNames;
		instrumentVertNames.push_back("Piano");
		instrumentVertNames.push_back("Violin");
		instrumentVertNames.push_back("Clarinet");
		instrumentVertNames.push_back("Synth");
		trackPolyButton = new PolyButton(Pt(trackPos.x, trackPos.y),"NormalTrack", 
			trackRegionHeight/4, trackRegionHeight/4, trackRegionHeight, instrumentVertNames, Pt(15,90), Vec3f(0.3,0.3,0.3), image_filename);
		trackPolyButton->setTrack(this);
		int h = trackPolyButton->getPolyInterface()->getHeight();
		Pt pos = trackPolyButton->getPolyInterface()->getCenter();
		pos.y += (h/2 - h/8);
		trackPolyButton->getPolyInterface()->MoveButton(pos);
	//drum track button
	}else if(trackId == "drum"){
		//TRACK INSTRUMENT polybutton initialization
		image_filename.assign("../source/applications/audio_interface/images/ppm/drum_button.ppm");			
		std::vector<std::string> beatVertNames;
		beatVertNames.push_back("Straight");
		beatVertNames.push_back("Funk");
		beatVertNames.push_back("Hip-Hop");
		beatVertNames.push_back("Rock");
		beatVertNames.push_back("Metal");
		trackPolyButton = new PolyButton(Pt(trackPos.x, trackPos.y),"DrumTrack", 
			trackRegionHeight/4, trackRegionHeight/4, trackRegionHeight, beatVertNames, Pt(0,100), Vec3f(0.3,0.3,0.3), image_filename);
		trackPolyButton->setTrack(this);
		int h = trackPolyButton->getPolyInterface()->getHeight();
		Pt pos = trackPolyButton->getPolyInterface()->getCenter();
		pos.y += (h/2 - h/8);
		trackPolyButton->getPolyInterface()->MoveButton(pos);
	}

	//Set so none of the effects are initally added to the tracks
	for (unsigned int i=0; i<effects.size(); i++){
		//DEBUG: change to true here to show all possible effects for all tracks
		effectAdded.push_back(false);	
	}

	polyButtonsInitialized = true;

}

void Track::getAddedEffects(std::vector<PolyButton*> &tEffects) const{
	tEffects.clear(); 
	for(unsigned int i=0; i<effects.size(); i++){
		if(effectAdded[i]){
			tEffects.push_back(effects[i]); 
		}
	}
}



void Track::getParamNames(std::vector< std::vector< std::string > >& paramNames) const{
	std::vector< std::string > holder;
	std::vector<PolyButton*> activeEffects;
	paramNames.clear();

	if(trackId != "master"){
		holder = trackPolyButton->getPolyInterface()->getParamNames();
		holder.push_back("Inst");
		paramNames.push_back(holder);
		holder.clear();
	}
	this->getAddedEffects(activeEffects);
	for(unsigned int i=0; i<activeEffects.size(); i++){
		holder = activeEffects[i]->getPolyInterface()->getParamNames();
		holder.push_back(activeEffects[i]->getText());
		paramNames.push_back(holder);
		holder.clear();
	}

}
void Track::getParamValues(std::vector< std::vector< int > >& paramValues) const{
	std::vector< int > holder;
	std::vector<PolyButton*> activeEffects;
	paramValues.clear();

	if(trackId != "master"){
		holder = trackPolyButton->getPolyInterface()->getParamValues();
		paramValues.push_back(holder);
		holder.clear();		
	}
	this->getAddedEffects(activeEffects);
	for(unsigned int i=0; i<activeEffects.size(); i++){
		holder = activeEffects[i]->getPolyInterface()->getParamValues();
		paramValues.push_back(holder);
		holder.clear();
	}
}

void Track::drawCollapsedEffects(){ 
	//draw collapsed effects
	for(unsigned int i=0; i<effects.size(); i++){
		if(effectAdded[i] && effects[i]->isCollapsed()){
			effects[i]->drawPB();
		}
	}
}

void Track::drawExpandedEffects(){ 
	//draw expanded effects
	for(unsigned int i=0; i<effects.size(); i++){
		if(effectAdded[i] && !effects[i]->isCollapsed()){
			effects[i]->drawPB();
		}
	}
}

void Track::drawEffects(){ 
	//draw collapsed effects first
	drawCollapsedEffects();
	drawExpandedEffects();
}
void Track::drawTrackBase(){
	if(trackId != "master" && polyButtonsInitialized){ 
		if(trackPolyButton->getConnected()){
			drawTrackLine();
			drawCollapsedEffects();
		}

		if(trackPolyButton->isCollapsed()){
			trackPolyButton->drawPB(); 
		}
		if(trackPolyButton->isCollapsed() && trackId != "drum" && trackPolyButton->getConnected()){
			trackIdLabel->renderNoBackground(trackRegionHeight);
		}
	}else if(trackId == "master"){
		drawTrackLine();
		drawCollapsedEffects();
	}
}
void Track::drawExpandedElements(){
	if(trackId != "master" && polyButtonsInitialized){ 
		if(trackPolyButton->getConnected()){
			drawExpandedEffects();
		}

		if(!trackPolyButton->isCollapsed()){
			trackPolyButton->drawPB(); 
		}
	}else if(trackId == "master"){
		drawExpandedEffects();
	}
}

void Track::addEffect(std::string name){
	for(unsigned int i=0; i<effects.size(); i++){
		if(effects[i]->getText() == name){
			effectAdded[i] = true;
		}
	}
}

bool Track::tryToRemoveEffect(PolyButton *pb){
	for(unsigned int i=0; i<effects.size(); i++){
		if(effects[i] == pb){
			effectAdded[i] = false;
			pb->moveCollapsedToOriginalPos();
			return true;
		}
	}
	return false;
}

void Track::drawTrackLine(){

	if(!isDashed){
		glLineWidth(4.0);
		glBegin(GL_LINES);
			glColor4f(trackColor.x(), trackColor.y(), trackColor.z(), transparency);
			glVertex2f(trackPos.x, trackPos.y);
			glVertex2f(trackPos.x + trackWidth, trackPos.y);
		glEnd();
	}else{
		glLineStipple(10, 0xAAAA);
		glLineWidth(4.0);
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINES);
			glColor4f(trackColor.x(), trackColor.y(), trackColor.z(), transparency);
			glVertex2f(trackPos.x, trackPos.y);
			glVertex2f(trackPos.x + trackWidth, trackPos.y);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

}

void Track::tryToMoveTrack(Pt pos){
	trackPolyButton->tryToMovePolyButton(pos);
	if(trackPolyButton->isCollapsed()){
		trackPolyButton->setConnected(false);
	}
}

void Track::collapseEffects(){
	for(unsigned int i=0; i<effects.size(); i++){
		effects[i]->setCollapsed(true);
	}
}

void Track::moveAndResize(Pt tp, double tw, double TRH){
	trackPos = tp;
	trackWidth = tw;
	trackRegionHeight = TRH;

	for(unsigned int i=0; i<effects.size(); i++){
	 	// Pt effectPos = effects[i]->getPosition();
	 	// effectPos.y = newY;
	 	// effects[i]->changePositions(effectPos);
	 	effects[i]->moveAndResize(Pt(trackPos.x+(trackWidth*((i+1)*0.25)), trackPos.y), 
	 		trackRegionHeight/4, trackRegionHeight/4);
	}
	//trackButton = new Button(Pt((tp.x-tw/22),(tp.y-tw/22)), tw/11, tw/11, Vec3f(1.0,1.0,1.0), trackId.c_str());

	if(trackId != "master"){
		trackPolyButton->moveAndResize(//Pt((trackPos.x-trackWidth/22),(trackPos.y-trackWidth/22)),
			trackPos,
			trackRegionHeight/4,trackRegionHeight/4);
		if(trackId != "drum"){
			trackIdLabel->Move(trackPos);
		}
	}
}

void Track::verticalMove(double newY){ 
	trackPos.y = newY;
	if(polyButtonsInitialized){
		for(unsigned int i=0; i<effects.size(); i++){
		 	Pt effectPos = effects[i]->getPosition();
		 	effectPos.y = newY;
		 	effects[i]->changePositions(effectPos);
		}
		if(trackId != "master"){
			Pt tbPos = trackPolyButton->getPosition();
			tbPos.y = newY;
			trackPolyButton->changePositions(tbPos);
			if(trackId != "drum"){
				trackIdLabel->Move(tbPos);
			}
		}
	}
}
