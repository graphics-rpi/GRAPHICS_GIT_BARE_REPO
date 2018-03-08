#include "../paint/gl_includes.h"

#include <map>

#include "ClickableObject.h"
#include "text.h"
#include "../../common/Image.h"

int ClickableObject::next_last_touched = 1;
int ClickableObject::max_last_touched = 1;
extern int HandleGLError(std::string foo);

#define MEMORY_FALLOFF 100

// ==========================
// CONSTRUCTORS
ClickableObject::ClickableObject(const Pt &p, double w, double h, const Vec3f &c)
  : DrawableObject(p,w,h,c) {
  assert (w > 0 && h > 0);
  initialize(); 
  color = c;
}


ClickableObject::ClickableObject()
  : DrawableObject() {
  initialize();
}



// ==========================
// HELPER FUNCTIONS
void ClickableObject::initialize() { 

  // action
  is_pressable = true;
  pressed = false;
  pressed_by_id = -1;
  pressed_position = Pt(0,0);
  press_memory_msec = -1;
  auto_release_msec = -1;
  last_touched = 0;
  //touch();

  assert (border_info.size() == 0);
}

// ==========================
// INTERACTION
bool ClickableObject::Hover(const std::list<Pt> &trail, int threshold_count) const {
  std::list<Pt>::const_reverse_iterator itr = trail.rbegin();
  int in_count = 0;
  int out_count = 0;
  while (itr != trail.rend()) {
    if (PointInside(*itr))
      in_count++;
    else
      out_count++;
    itr++;
    if (in_count + out_count > 1.1*threshold_count)
      break;
  }
  //std::cout << "HOVER TEST" << in_count << " " << out_count << std::endl;
  if (in_count > threshold_count) return true;
  return false;
}

void ClickableObject::press(int ar_msec){
	pressed = true; 
    auto_release_msec = ar_msec;
    gettimeofday(&press_tv,NULL);
    touch();
}

void ClickableObject::press(const Vec3f &bc, int id, const Pt &pos) { 
	addBorder(BorderInfo(bc,5,0.5));
	//border_color = bc; 
	pressed_by_id = id;
	Pt tmp = pos;

	if (tmp.x <  -raw_width/2.0) tmp.x =  -raw_width/2.0;
	if (tmp.x >   raw_width/2.0) tmp.x =   raw_width/2.0;
	if (tmp.y < -raw_height/2.0) tmp.y = -raw_height/2.0;
	if (tmp.y >  raw_height/2.0) tmp.y =  raw_height/2.0;

	pressed_position = tmp;
	press(); 
}

void ClickableObject::release(int press_memory){
  std::cout << "ClickableObject::release()" << std::endl;

	pressed = false; 
	pressed_by_id = -1; 
	pressed_position = Pt(0,0);
	if (press_memory < 0) {
		press_memory_msec = -1;
	} else {
		press_memory_msec = press_memory;
		gettimeofday(&release_tv,NULL);
	}
	clearBorders();
}

double ClickableObject::getMotionMemoryFactor(const timeval &now){
	assert (pressed == false);
	if (press_memory_msec <= 0) return 0;
	int msec = timevaldiff(release_tv,now);
	if (msec < press_memory_msec) return 1;
	msec -= press_memory_msec;
	if (msec > MEMORY_FALLOFF*press_memory_msec) return 0;  
	double answer = 1 -  (msec / double(MEMORY_FALLOFF*press_memory_msec));
	answer = pow(answer,0.1);
	assert (answer >= 0 && answer <= 1);
	return answer;
}

void ClickableObject::release_if_time_expired(const timeval &now){
	if (pressed == false) return;
	assert (auto_release_msec > 0);
	int msec = timevaldiff(press_tv,now);
	if (msec > auto_release_msec) {
	    std::cout << "release!" << std::endl;
	    pressed = false;
	}
	clearBorders();
}
