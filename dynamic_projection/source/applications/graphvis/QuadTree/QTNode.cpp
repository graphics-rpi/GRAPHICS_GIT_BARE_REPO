/*
 * QTNode.cpp
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */

#include "QTNode.h"

#include "../../../calibration/planar_interpolation_calibration/MersenneTwister.h"

#include <iostream>

#include <sstream>

#include "Layer.h"
// with EMPAC's 4K projector:
//#define BUTTON_MULTIPLIER 1.8

// on a normal screen:
//#define BUTTON_MULTIPLIER 1


//smaller that clear
#define BUTTON_MULTIPLIER 0.6


QTNode::QTNode(int id_, Layer *l, /* const std::string name_,*/ const std::string &texname_,
	       const Pt &p_, const Vec3f &color_, Metadata t_, /*const Pt & _a, const Pt & _b, */ double relativesize_)
  : 
  // the sizes are ignored in the map app...  (not sure this is the right design)
  ClickableObject( p_, relativesize_ * BUTTON_MULTIPLIER, relativesize_ * BUTTON_MULTIPLIER, color_ ),
  Button(p_, relativesize_ * BUTTON_MULTIPLIER ,relativesize_ * BUTTON_MULTIPLIER, color_, texname_, ""),
  QTElement(id_, p_, relativesize_ * BUTTON_MULTIPLIER, relativesize_ * BUTTON_MULTIPLIER, color_/*,  _a,  _b*/)
{

  assert (l != NULL);
  layer = l;
  layer->AddNode(this);
  boundingbox.Extend(p_);
  
  data = Metadata(t_);
  raw_size = 20*BUTTON_MULTIPLIER;
  //node_name = name_;
  draw_mode = 1;
  //ID = id_;
  
  relativesize = relativesize_;
  current_scale = relativesize_; //1.0;
  
  //  collapsed = -1;
  //depth = 0;
  //level = 0;
  counter = 0;
    
  setCircle(true);
  
  setButtonBorders();
  
  power_percentage = water_percentage = waste_percentage = comm_percentage = -1;

}

const std::string& QTNode::getType() const { 
  assert (layer != NULL);
  return layer->GetName(); 
}


#include "layer_selector/mapcolors.h"
#include "layer_selector/LayerSelector.h"

#define RING_THICKNESS 0.5

extern bool GLOBAL_BLINK;
extern LayerSelector* g_layerselector;

void QTNode::setButtonBorders() {
  clearBorders();

  assert (g_layerselector != NULL);
  bool power_vis;
  bool water_vis;
  bool waste_vis;
  bool comm_vis;

  g_layerselector->getServiceVisiblityToggles(power_vis,water_vis,waste_vis,comm_vis);
  //std::cout << "P" << power_vis << "  W" << water_vis << "  S" << waste_vis << "  C" << comm_vis << std::endl;

  bool power_ring = power_vis && (power_percentage >= 0 && power_percentage < 0.99);
  bool water_ring = water_vis && (water_percentage >= 0 && water_percentage < 0.99);
  bool waste_ring = waste_vis && (waste_percentage >= 0 && waste_percentage < 0.99);
  bool comm_ring  = comm_vis && (comm_percentage >= 0 && comm_percentage < 0.99);


  if (1) { //power_ring || water_ring || waste_ring || comm_ring) {
    addBorder(BorderInfo(Vec3f(1,1,1),0.1*raw_size,1));
  }

  //if (GLOBAL_BLINK) return;

  if (waste_ring) {
    addBorder(BorderInfo(GLOBAL_getmapcolor("waste"),RING_THICKNESS*raw_size,1));
  }
  if (water_ring) {
    addBorder(BorderInfo(GLOBAL_getmapcolor("water"),RING_THICKNESS*raw_size,1));
  }
  if (comm_ring) {
    addBorder(BorderInfo(GLOBAL_getmapcolor("comm"),RING_THICKNESS*raw_size,1));
  }
  if (power_ring) {
    addBorder(BorderInfo(GLOBAL_getmapcolor("power"),RING_THICKNESS*raw_size,1));
  }
}


/*bool QTNode::operator==(const QTNode &other){

  if(getID() == other.getID())
		return true;
	else
		return false;

}

bool QTNode::operator<(const QTNode &other){
	//std::cout << "derp\n";
  return(getID() < other.getID());
}

bool QTNode::operator<(const int k){
  return((int)getID() < k);
}

float QTNode::hittest(BoundingBox2f _bounds)
{
    return getBox().Distance(_bounds);
}
*/

void QTNode::AddConnection(QTEdge *e) {
	// assert (!IsConnected(n));
	connections.push_back(e);
}

/*
void QTNode::AddLayer(int k) {

	layers.push_back(k);

}

void QTNode::RemoveLayer(int k){

	std::vector<int>::iterator itr;
	for(itr = layers.begin(); itr != layers.end(); ++itr)
	{
		if(*itr == k)
		{
		  layers.erase(itr);
		  return;
		}
	}

	if(itr != layers.end())
		layers.erase(itr);

}
*/

void QTNode::RemoveConnection(QTEdge *e) {
	QTEdge *temp;

	for(std::vector<QTEdge*>::iterator itr = connections.begin();
			itr != connections.end(); ++itr)
	{
	  temp = (*itr);

		if(temp->getID() == e->getID())
		{
			connections.erase(itr);
			return;
		}
	}
}

// void QTNode::updateStatus(long fieldmask, long value){
// 	//std::cout<<"Mask: "<<(unsigned int)fieldmask<<'\n';
// 	//std::cout<<"Value: "<<(unsigned int)value<<'\n';
// 	long curStatus = atol(data.getVal("status").c_str());
// 	curStatus &= ~fieldmask;
// 	curStatus |= value;
// 	char buffer[10];
// 	//std::cout<<"Current status: "<<(unsigned int)curStatus<<'\n';
// 	sprintf(buffer,"%d",(unsigned int)curStatus);
// 	data.addData(std::string("status"),std::string(buffer));
//
//
// 	setButtonBorders();
// }

/*
double QTNode::GetConnectionDistance(QTNode *n) {
  if (thing->isAnimal()) {
    return IDEAL_DISTANCE*2+getButton()->getHeight();
  } else if (n->thing->isAnimal()) {
    return IDEAL_DISTANCE*2+n->getButton()->getHeight();
  }
  double d1 = IDEAL_DISTANCE*(10-this->getLevel());
  double d2 = IDEAL_DISTANCE*(10-n->getLevel());
  double d = 0.5*(d1+d2);
  return d;
}
 */

double QTNode::GetConnectionDistance(QTNode */*n*/) const {
	return 240;
	//return IDEAL_DISTANCE*2+getButton()->getHeight();
}


bool QTNode::IsConnected(QTNode *n) const {

	QTEdge *curr;
	for(std::vector<QTEdge*>::const_iterator itr = connections.begin();
			itr != connections.end(); ++itr)
	{
		curr = (*itr);

		if(curr->GetStart()->getID() != getID() && curr->GetEnd()->getID() == n->getID())
		{
			return true;
		}
		else if(curr->GetEnd()->getID() != getID() && curr->GetStart()->getID() == n->getID())
		{
			return true;
		}
	}

	return false;

}


void QTNode::setPosition(double x, double y) {
	MoveNoDamping(Pt(x-getWidth()/2.0,y-getHeight()/2.0));
}




std::ostream& operator<<(std::ostream &ostr, const QTNode &e){
  ostr << "QTNode: " << e.getID() << "  " << e.connections.size() << " connections" << std::endl;
  for(unsigned int i = 0; i < e.connections.size(); ++i)
    ostr << "   " << *(e.connections[i]);
  return ostr;
}

void QTNode::SetDrawMode(int i){

	if(i >= 1 && i <= 4)
	{
		draw_mode = i;
	}

}

bool QTNode::AnimateSize() {

  int maxsize =10;
  
  if(draw_mode != 4)
    current_scale = relativesize; //1;
  setDimensions(raw_size*current_scale,raw_size*current_scale);
  SetRenderText(false);
  
  if (draw_mode == 1) {
    return false;
  }
  
  if (draw_mode == 2) {
    if (current_scale < relativesize+0.0001) {//1.00001) {
      // no animation necessary
      draw_mode = 1;
      return false;
    }
    else {
      // decrease size
      current_scale = std::max(current_scale * 0.95, relativesize); //1.0);
      setDimensions(raw_size*current_scale,raw_size*current_scale);
      return true;
    }
  }
  else if(draw_mode == 3) {
    if (current_scale > maxsize - 0.0001) {
      // no animation necessary
      draw_mode = 1;
      return false;
    }
    else {
      // increase size
      //current_scale = std::min(current_scale * 1.05, double(maxsize));
      current_scale = std::min(current_scale * 1.2, double(maxsize));
      setDimensions(raw_size*current_scale,raw_size*current_scale);
      return true;
    }
  }
  else if(draw_mode == 4) {
    if(current_scale > 40 - 0.0001) {
      SetRenderText(true);
      return false;
    }
    else {
      SetRenderText(true);
      //current_scale = std::min(current_scale * 1.05, double(maxsize));
      current_scale = std::min(current_scale * 1.2, double(maxsize));
      setDimensions(raw_size*current_scale,raw_size*current_scale);
      return true;
    }
  }
  
  return false;
}

bool QTNode::isVisible() const {
  return layer->GetNodeShow();
}

double QTNode::getZOrder() const {
    return (double)getLastTouched()/(double)getMaxTouched();
}


void QTNode::Draw() const {
  if (relativesize != 1 || current_scale != 1) {
    //   std::cout << "QTNode::Draw() id=" << getID() << " raw_size=" << raw_size << "  current_scale=" << current_scale << "  relativesize=" << relativesize << std::endl;
  }

  if (!isVisible()) return;

  ((QTNode*)this)->setButtonBorders();

  ((QTNode*)this)->AnimateSize();


  //Button::paint();
}


void QTNode::SetCounter(float _counter)
{
	if(_counter <= 0)
	{
		SetPress(false);
	}
	else
	{
		SetPress(true);
	}

	if(_counter < 0)
	{
		counter = _counter;
		//		button.SetBorderOpacity(0);
		//AnimateSize();
	}
	//Change here and change in synenv/trunk/visualizations/MapView/layer_graph/Graphlayer.cpp
	else if(_counter > SELECTING_COUNTER_HACK) //4)
	{
		if(draw_mode == 4)
		{
		  //button.SetBorderOpacity(0);
			SetDrawMode(2);
			select_recover = 2;
			counter = 0;
			return;
		}

		//button.SetBorderOpacity(1);
		select_recover = 2;
		SetDrawMode(4);
		counter = 0;
		return;
	}
	else
	  //button.SetBorderOpacity(_counter);

	counter = _counter;

}

#define DEMAND_DETAILS 0
//#define DEMAND_DETAILS 1

// ===========================================================================================
// ===========================================================================================

void QTNode::updateButtonText(){
  static int count = 0;
  std::vector<std::string>& textFields = getTextStrings();
  //  std::cout<<"Update text " << count << " on button '"<<data.getVal(std::string("name"))<<"'\n";
  textFields.clear();

  double p_demand   = 0;
  double p_received = 0;
  double w_demand   = 0;
  double w_received = 0;
  double s_demand   = 0;
  double s_received = 0;
  double c_demand   = 0;
  double c_received = 0;

  bool demand_node = false;

  for(unsigned int i = 0; i < data.numFields(); i++){
    //std::cout<<"\tField: "<<data.getField(i)<<" "<<data.getVal(i) << " " << data.getField(i).length()<<'\n';
    //std::cout<<"\tField: "<<data.getField(i)<<" '"<<data.getVal(i) << "'" << std::endl;
    if (data.getField(i) == "status") continue;
    if (data.getField(i) == "location") continue;

    if (data.getField(i) == "name" && data.getVal(i) == "") continue;
    if (data.getField(i) == "name" && data.getVal(i) == " ") continue;
    if (data.getField(i) == "definition" && data.getVal(i) == "node") continue;
    if (data.getField(i) == "definition" && data.getVal(i) == "Demand") continue;

    //if (data.getField(i) == "type") continue;
 
    if (data.getField(i) == "power demand")    { p_demand = atof(data.getVal(i).c_str());   continue; }
    if (data.getField(i) == "power received")  { p_received = atof(data.getVal(i).c_str()); continue; }
    if (data.getField(i) == "water demand")    { w_demand = atof(data.getVal(i).c_str());   continue; }
    if (data.getField(i) == "water received")  { w_received = atof(data.getVal(i).c_str()); continue; }
    if (data.getField(i) == "waste demand")    { s_demand = atof(data.getVal(i).c_str());   continue; }
    if (data.getField(i) == "waste received")  { s_received = atof(data.getVal(i).c_str()); continue; }
    if (data.getField(i) == "comm demand")     { c_demand = atof(data.getVal(i).c_str());   continue; }
    if (data.getField(i) == "comm received")   { c_received = atof(data.getVal(i).c_str()); continue; }

    if (data.getField(i) == "ID")   { continue; }

    if (data.getField(i) == "type" && data.getVal(i) == "Demand")   { demand_node = true; }
    if (data.getField(i) == "damage" && demand_node) { continue; }


    if (data.getField(i) == "damage")   { 
      if (data.getVal(i) == "") { data.setVal(i, "status ok"); }
      else if (data.getVal(i) == "") { data.setVal(i, "DAMAGED"); }
    }

    std::ostringstream sstr;
    sstr<<data.getField(i)<<": "<<data.getVal(i);
    textFields.push_back(sstr.str());
  }

  assert (p_received <= p_demand);
  assert (w_received <= w_demand);
  assert (s_received <= s_demand);
  assert (c_received <= c_demand);

  std::ostringstream sstr;
  if (p_demand < 0.00001) power_percentage = -1;
  else {
    power_percentage = p_received / p_demand;
#if DEMAND_DETAILS 
    sstr<<"power: "<<p_received <<"/" <<p_demand;
    textFields.push_back(sstr.str());
    sstr.str("");
#endif
    sstr<<"power: "<<(int)(100*power_percentage)<<"%";
    textFields.push_back(sstr.str());
    sstr.str("");
  }
  if (w_demand < 0.00001) water_percentage = -1;
  else {
    water_percentage = w_received / w_demand;
#if DEMAND_DETAILS 
    sstr<<"water: "<<w_received <<"/"<< w_demand;
    textFields.push_back(sstr.str());
    sstr.str("");
#endif
    sstr<<"water: "<<(int)(100*water_percentage)<< "%";
    textFields.push_back(sstr.str());
    sstr.str("");
  }
  if (s_demand < 0.00001) waste_percentage = -1;
  else {
    waste_percentage = s_received / s_demand;
#if DEMAND_DETAILS 
    sstr<<"waste: "<<s_received <<"/"<< s_demand;
    textFields.push_back(sstr.str());
    sstr.str("");
#endif
    sstr<<"waste: "<<(int)(100*waste_percentage)<<"%";
    textFields.push_back(sstr.str());
    sstr.str("");
  }
  if (c_demand < 0.00001) comm_percentage = -1;
  else {
    comm_percentage = c_received / c_demand;
#if DEMAND_DETAILS 
    sstr<<"comm: "<<c_received <<"/"<< c_demand;
    textFields.push_back(sstr.str());
    sstr.str("");
#endif
    sstr<<"comm: "<<(int)(100*comm_percentage)<<"%";
    textFields.push_back(sstr.str());
    sstr.str("");
  }


  //std::ostringstream sstr;
  sstr.str("");
  sstr<<"ID: "<<getID();
  textFields.push_back(sstr.str());
  

  count ++; 
}


// ===========================================================================================
// ===========================================================================================


bool QTNode::checkStatus(long /*field*/){
return true;
}

// void QTNode::updateStatus(long field, bool status){
//
// 		//
// 	if(!status)
// 		std::cout<<"Got a false status!\n";
//
//
// 	unsigned char newStatus = (status)?(1):(0);
//
// 	if(data.getVal(std::string("damage"))=="1")
// 		newStatus = 0;
//
// 	switch(field){
// 		case PowerPath:
// 			updateStatus( 1, newStatus);
// 			break;
// 		case WaterPath:
// 			updateStatus( (1<<1),newStatus<<1);
// 			break;
// 		case WastePath:
// 			updateStatus( (1<<2), newStatus<<2);
// 			break;
// 		case CommPath:
// 			updateStatus( (1<<3), newStatus<<3);
// 			break;
// 		case Power:
// 			updateStatus( (1<<4), newStatus<<4);
// 			break;
// 		case Water:
// 			updateStatus( (1<<5), newStatus<<5);
// 			break;
// 		case Waste:
// 			updateStatus( (1<<6), newStatus<<6);
// 			break;
// 		case Comm:
// 			updateStatus( (1<<7), newStatus<<7);
// 			break;
// 		default:
// 			break;
// 	}
// 	assert(visited==false);
// 	visited = true;
// 	for(unsigned int i = 0; i < connections.size(); i++){
//
// 		std::string newval((newStatus)?("1"):("0"));
// 		//update the edge
// 		connections[i]->GetData()->addData(std::string("status"),newval);
//
// 		if(connections[i]->IsEndValid()){
//
// 			QTNode* endNode = connections[i]->GetEnd();
//
// 			if(!endNode->visited){
// 				endNode->updateStatus(field,newStatus,data.getType());
// 			}
//
// 		}
// 	}
// 	updateButtonText();
// }
//
// void QTNode::updateStatus(StatusField field, bool status,std::string originalType){
// 	//}
//
// 	if(!status){
// 		std::cout<<"got a false status!\n";
// 	}
// 	unsigned char newStatus = (status)?(1):(0);
// 	if(data.getVal("damage")=="1"){
// 		newStatus = 0;
// 	}
// 	switch(field){
// 		case PowerPath:
// 			updateStatus( 1, newStatus);
// 			break;
// 		case WaterPath:
// 			updateStatus( (1<<1), newStatus<<1);
// 			break;
// 		case WastePath:
// 			updateStatus( (1<<2), newStatus<<2);
// 			break;
// 		case CommPath:
// 			updateStatus( (1<<3), newStatus<<3);
// 			break;
// 		case Power:
// 			updateStatus( (1<<4), newStatus<<4);
// 			break;
// 		case Water:
// 			updateStatus( (1<<5), newStatus<<5);
// 			break;
// 		case Waste:
// 			updateStatus( (1<<6), newStatus<<6);
// 			break;
// 		case Comm:
// 			updateStatus( (1<<7), newStatus<<7);
// 			break;
// 		default:
// 			break;
// 	}
// 	visited = true;
//
// 	for(unsigned int i = 0; i < connections.size(); i++){
// 		std::string newval((newStatus)?("1"):("0"));
//
// 		connections[i]->GetData()->addData(std::string("status"),newval);
//
// 		if(connections[i]->IsEndValid()){
//
// 			QTNode* endNode = connections[i]->GetEnd();
//
// 			if(data.getType()==originalType && !endNode->visited){
// 				endNode->updateStatus(field,newStatus,originalType);
// 			}
//
// 		}
// 	}
//
// 	updateButtonText();
// }

// void QTNode::reset(){
// 	data.addData(std::string("status"),std::string("0"));
// 	data.addData(std::string("damage"),std::string("0"));
// 	updateButtonText();
// }

