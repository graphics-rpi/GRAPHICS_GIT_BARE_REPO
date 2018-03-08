#include "DrawableObject.h"

#define DAMP_MOVEMENT 0.2

DrawableObject::DrawableObject() {
  initialize(Pt(0,0),1,1,Vec3f(0.5,0.5,0.5));
}

DrawableObject::DrawableObject(const Pt &lower_left_corner, double w, double h, const Vec3f &c){
  initialize(lower_left_corner,w,h,c);
}

void DrawableObject::initialize(const Pt &lower_left_corner, double w, double h, const Vec3f &c){
  // geometry
  assert (w > 0 && h > 0);
  computed_centroid = lower_left_corner + Pt(w/2.0,h/2.0);
  magnified_centroid = computed_centroid;
  scale_factor = 1.0;
  raw_width = w;
  raw_height = h;
  
  // appearance
  transparency = 0;
  color = c;
}

// =================================================
// =================================================
bool DrawableObject::PointInside(const Pt &p) const {
  Pt pt = getLowerLeftCorner();
  return (p.x > pt.x && p.y > pt.y && p.x <= pt.x+getWidth() && p.y <= pt.y+getHeight());
}


Pt DrawableObject::Offset(const Pt &p) const{
	if (!PointInside(p)) {  std::cout << "WARNING in Offset, point not inside " << std::endl; }
	return Pt(p.x-computed_centroid.x,p.y-computed_centroid.y);
}
void DrawableObject::Move(const Pt &p){
    computed_centroid = p*DAMP_MOVEMENT + computed_centroid*(1-DAMP_MOVEMENT);
}
void DrawableObject::MoveChooseDampingCoeff(const Pt &p, double dampingCoeff){
  	computed_centroid = p*dampingCoeff + computed_centroid*(1-dampingCoeff);
}
void DrawableObject::MoveNoDamping(const Pt &p){
  	computed_centroid = p;
}
void DrawableObject::MoveNoDampingCentroid(const Pt &p){
  computed_centroid = p + Pt(raw_width/2.0,raw_height/2.0);
}
void DrawableObject::MoveWithMaxSpeed(const Pt &p, double max_speed){
  	Pt dir = p - computed_centroid;
  	double d = dir.Length();
  	if(d > max_speed){
    	//dir.Normalize();
    	computed_centroid = computed_centroid + dir * (max_speed / d);
  	}
  	else{
	    computed_centroid = computed_centroid + dir * DAMP_MOVEMENT;
  	}
}
