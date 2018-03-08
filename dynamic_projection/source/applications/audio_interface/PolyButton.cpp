#include "PolyButton.h"

//is run as a child pthread
//allows movement to happen in parallel with other actions
void* gradualMovement(void* pb){

	Button* button = ((PolyButton*)pb)->getCollapsedButton();
	Pt pos = ((PolyButton*)pb)->getPosition();

	float distMoveBack = 100.0;
	float xdiff;
	float ydiff;
	//int i = 0;
	while(distMoveBack > 1.0){
		button->MoveChooseDampingCoeff(pos, 0.0000001);
		xdiff = button->getCentroid().x - pos.x;
		ydiff = button->getCentroid().y - pos.y;
		distMoveBack = sqrt(xdiff*xdiff + ydiff*ydiff);
	}
	((PolyButton*)pb)->setConnected(true);
	return NULL;
}

//TRY TO EXAPAND OBJECT ==============================
//--expansion of polyButton
//--movement of polyButton back to original position if moved and released
//====================================================
void PolyButton::tryToExpandPolyButton(Cursor* c){
	if(collapsed){
		if(collapsedButton->PointInside(c->getScreenPosition())){
			//float distance = sqrt(v1.x*v2.x + v1.y*v2.y);
			Pt v1, v2;
			v1 = c->getScreenPosition();
			v2 = position;
			float xdiff = v1.x-v2.x;
			float ydiff = v1.y-v2.y;
			float distance = sqrt(xdiff*xdiff + ydiff*ydiff);
			//std::cout << "v1: " << v1 << " v2: " << v2 << std::endl;
			if(distance < buttonWidth){
				c->setGrabbedVoidObj(this);
				std::string objType;
				if(buttonText == "DrumTrack"){
					objType = "DrumTrackButton";
				}else if(buttonText == "NormalTrack"){
					objType = "NormalTrackButton";
				}else{
					objType = "PolyButton";
				}
				c->setObjType(objType);
				//set background color
				Vec3f bcolor = Vec3f(c->getColor().x()/2,c->getColor().y()/2,c->getColor().z()/2);
				this->getPolyInterface()->setBackgroundColor(bcolor);
				this->setCollapsed(false);
			}else{
				c->setGrabbedVoidObj(NULL);
				std::string objType = "NULL";
				c->setObjType(objType);
			}
			//create child thread to gradually move the button back to its original position
			//if the button hasn't been put in the trash
			pthread_t tid[ 1 ];   /* keep track of the thread IDs */
			int rc = pthread_create( &tid[0], NULL, gradualMovement, this);
			if ( rc != 0 ){
				perror( "MAIN: Could not create child thread" );
			}	
			//===============================================
			//need to check if cursor position is over the trash
			//if so, need to "remove" the effect
			//===================================================
		}
	}
}

void PolyButton::slideBackToPosition(){
	pthread_t tid[ 1 ];   /* keep track of the thread IDs */
	int rc = pthread_create( &tid[0], NULL, gradualMovement, this);
	if ( rc != 0 ){
		perror( "MAIN: Could not create child thread" );
	}	
}

PolyButton* PolyButton::tryToPressPolyButton(Cursor *c){

	//double fdh = args->tiled_display.full_display_height;
	Vec3f bcolor;
	//std::cout << "gets here" << std::endl << std::endl;
		
	std::string objType;
	//Button *b;
	PolyButton *prevPB = (PolyButton*)c->getGrabbedVoidObj();

	//if the cursor is already assigned a PolyButton, and clicks away from it, release it
	if(prevPB != NULL){
		assert(prevPB->getPolyInterface() != NULL);
		assert(c != NULL);
	  	if(!prevPB->getPolyInterface()->isLegalMove(c->getScreenPosition())) {
	  		//std::cout << "tries to release polyButton" << std::endl;
			//bcolor = Vec3f(0.15,0.15,0.15);
			prevPB->setCollapsed(true);
			c->setGrabbedVoidObj(NULL);
			objType = "NULL";
			c->setObjType(objType);
		}
	}

	//if the PolyButton is in collapsed form
	if(collapsed){
		//std::cout << "collapsed" << std::endl;
		//std::cout << "trys to press collapsed polyButton" << std::endl;
		if(collapsedButton->PointInside(c->getScreenPosition())){
			c->setGrabbedVoidObj(this);
			//std::string objType;
			if(buttonText == "DrumTrack"){
				objType = "DrumTrackButton";
			}else if(buttonText == "NormalTrack"){
				objType = "NormalTrackButton";
			}else{
				objType = "PolyButton";
			}
			//objType = "PolyButton";
			c->setObjType(objType);
			//set background color
			//bcolor = Vec3f(c->getColor().x()/2,c->getColor().y()/2,c->getColor().z()/2);
			//this->getPolyInterface()->setBackgroundColor(bcolor);
			//this->setCollapsed(false);
			return this;
		
		}
	//if the PolyButton is in expanded form	
	}else{
		//std::cout << "expanded" << std::endl;
		//std::cout << "trys to press polyButton" << std::endl;
		if(myPolyInterface->isLegalMove(c->getScreenPosition())
			&& (this == (PolyButton*)c->getGrabbedVoidObj())){
			//b = myPolyInterface->getButton();
			//b->MoveNoDamping(c->getPosition());
			myPolyInterface->MoveButton(c->getScreenPosition());

			return this;
		}
	}
		
	return NULL;
}

void PolyButton::tryToMovePolyButton(Pt pos){


	if(!collapsed && myPolyInterface->isLegalMove(pos)){
		myPolyInterface->MoveButton(pos);
		//connected = true;
	}else if(collapsed){
		//std:: cout << "tries to move polybutton" << std::endl;

		collapsedButton->MoveNoDamping(pos);
	}
}

void PolyButton::moveAndResize(Pt pos, double w, double h){

	//for the resize
	position = pos;
	polyWidth = w;
	polyHeight = h;
	buttonWidth = w/4;
	buttonHeight = w/4;

	if(myPolyInterface->getNumVerts() == 2){
		//std::cout << "HERHERHEHREHREHREHRHERHERHER" << std::endl;
		polyWidth /= 3;
		polyHeight /= 3;
	}

	collapsedButton->setDimensions(buttonWidth, buttonHeight);
	myPolyInterface->Resize(polyWidth, polyHeight);

	collapsedButton->MoveNoDamping(position);
	myPolyInterface->Move(position);


}
