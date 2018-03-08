#ifndef __CLICKABLE_H__
#define __CLICKABLE_H__

#include "../paint/gl_includes.h"

#include <string>
#include <sys/time.h>
#include <list>

#include "DrawableObject.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../../../remesher/src/vectors.h"
//#include "../graph_interaction/object_group.h"

class ObjectGroup;

class ClickableObject : public DrawableObject {
public:
	// ==========================
  // OPERATORS	
	bool operator< (const ClickableObject &b) const { 
          return last_touched < b.last_touched; 
        }

	// ==========================
	// INTERACTION
	bool Hover(const std::list<Pt> &trail, int threshold_count) const;
	bool isPressed() const { return pressed; }
	
	void press(int auto_release_msec = -1);
  void press(const Vec3f &bc, int id, const Pt &pos);

	void release(int press_memory = -1);
	// returns 1 if the button was very recently pressed & 0 if it hasn't been pressed in a long time
	double getMotionMemoryFactor(const timeval &now);
	void release_if_time_expired(const timeval &now);
	int getLastTouched() const { return last_touched; }
    int getMaxTouched() const { return max_last_touched; }

	// should only be called by sync-ing tiled displays
	void setLastTouched(int lt) { 
		last_touched = lt; 
        ClickableObject::next_last_touched = std::max(lt+1,ClickableObject::next_last_touched); 

        ClickableObject::max_last_touched = ClickableObject::next_last_touched-1;
	}

	void touch() {  
		last_touched = ClickableObject::next_last_touched;
        ClickableObject::next_last_touched++; 

        ClickableObject::max_last_touched = ClickableObject::next_last_touched-1;
	}
	int PressedBy() const { assert (pressed == true); return pressed_by_id; }
	Pt getPressPosition() const { assert (pressed == true); return pressed_position; }
	void setUnpressable() { is_pressable = false; }
	bool isPressable() const { return is_pressable; }
	void SetPress(bool val){ pressed = val; }
	
	
	
// CHRIS S: This is icky, remove it when cleaning up somehow
ObjectGroup* getParentObjectGroup() { return parentObjectGroup; }
void setParentObjectGroup( ObjectGroup* o ) { parentObjectGroup = o; }
bool isObjectGroup() { return is_object_group; }
Pt getPosition() { return position;}
void setPosition( Pt& p ) { position = p; }
protected:
	// ==========================
	// CONSTRUCTORS
  ClickableObject(const Pt &p, double w, double h, const Vec3f &c);
  ClickableObject();

  void initialize(); //const Pt &p, double w, double h);

  // ==========================
  // REPRESENTATION
  
  // button action
  bool is_pressable;  
  bool pressed;
  int pressed_by_id;
  Pt pressed_position;
  int press_memory_msec;
  timeval press_tv;
  timeval release_tv;
  int auto_release_msec;
  int last_touched;  
  
  static int next_last_touched;
  static int max_last_touched;


  // FOR AUDIO INTERFACE (should be inheritance based...)
  std::string tylers_object_type;
 public:
  std::string getObjectType() { return tylers_object_type; }
  void setObjectType(std::string t) { tylers_object_type = t; }
 protected:
  // END FOR AUDIO INTERFACE


  
// CHRIS S: This is icky, remove it when cleaning up somehow
ObjectGroup* parentObjectGroup;
bool is_object_group;
Pt position;
};


inline bool button_pointer_sorter(const ClickableObject *c1, const ClickableObject *c2) { 
	return (*c1) < (*c2); 
}

#endif
