#ifndef _CURSOR_H_
#define _CURSOR_H_

#include <string>
#include <iostream>
#include "../applications/paint/button.h"
#include "../common/directory_locking.h"
#include <sys/time.h>

using std::cerr;
using std::endl;


const unsigned int NORMAL_MODE = 0;
const unsigned int GROUP_UNGROUP_MODE = 1;

class Cursor {

public:
  // CONSTRUCTOR
  Cursor(const std::string &n, int id, const Vec3f& c);


  // DESTRUCTOR
  ~Cursor() { assert (tracked_point == NULL); }

  // PRINTING
  friend std::ostream& operator<< (std::ostream &ostr, const Cursor &c);

  TrackedPoint* getTrackedPoint() const { return tracked_point; }
  void setTrackedPoint(TrackedPoint *_tp) { tracked_point = _tp; }

  // ACCESSORS
  const std::string& getName() const { return name; }
  int getId() const { return cursorId; }
  int getIdentifiedCursor() const { return cursorId; }
  const Vec3f& getColor() const { return color; }

  // some of these things are tooo button specific and don't belong here

  int getWhichButton() const { return whichbutton; }
  bool isStateUp() const { return(state == 'u'); }
  bool isStateDown() const { return(state == 'd'); }
  bool isStateNull() const { return(state == 'n'); }

  bool isScrollUp() const { return(scroll_state == 'u'); }
  bool isScrollDown() const { return(scroll_state == 'd'); }

  
  // Getter and setter of the current mode of the cursor
  bool isNormalMode() { return ( mode == NORMAL_MODE ); }
  bool isGroupUngroupMode() const { return ( mode == GROUP_UNGROUP_MODE ); }
  void setModeGroupUngroup() { mode = GROUP_UNGROUP_MODE; }
  void setModeNormal() { mode = NORMAL_MODE; }
  // Toggle between all possible modes
  void toggleMode() {
    if( mode == NORMAL_MODE ) mode = GROUP_UNGROUP_MODE; 
    else if( mode == GROUP_UNGROUP_MODE ) mode = NORMAL_MODE;
  }

  //returns position adjusted for bottom left at (0,0)
  Pt getScreenPosition() const;
  Pt getWorldPosition() const;
  int getScrollPosition() const;

  const std::list<std::pair<timeval,Pt> >& getWorldPositionTrail() const { return world_position_trail; }

  // MODIFIERS
  void setStateUp() { state = 'u'; }
  void setStateDown() { state = 'd'; }
  void setStateNull() { state = 'n'; }

  void setScrollUp() { scroll_state = 'u'; }
  void setScrollDown() { scroll_state = 'd'; }
  
  char getState() { return state; } 

  void setWhichButton(int wb) { whichbutton = wb; }

  //  void setButton(ClickableObject *b) { pressed_button = b; }
  //void setPolyButtonInd(int pb) { pressed_polyButton_ind = pb; }


  void setClickedObject(ClickableObject *p) { clickedObjectPtr = p; }
  ClickableObject* getClickedObject() { return clickedObjectPtr; }

  // CHRIS S: Use these methods!
  //void setGrabbedVoidObj( void* o ) { obj = o; }
  //void* getGrabbedVoidObj() { return obj; }


  //void setObjType(std::string &str) { objType = str; }
  //std::string getObjType() { return objType; }


  void setScreenPosition(const Pt& p);
  void setWorldPosition(const Pt& p);
  void setScrollPosition(const int p);



  // Virtual functions to be inherited and implemented by child classes (all cursors)
  virtual void draw(bool screen_space, const Vec3f &background_color) const = 0;
  virtual void draw_trail(bool screen_space, const Vec3f &background_color, double seconds, timeval cur) const;

  // Function to determine current gesture of cursor
  virtual int getCurrentStateAndGesture() = 0;
  virtual void updatePosition( const Pt ) = 0;
  virtual bool isMouse() { return false; }
	
 private:

void updateTrail();

  int cursorId;
  std::string name;

 protected:

  Vec3f color;

  // the usual glut stuff
  int whichbutton;
  char state;  //'u'-up; 'd'-down; 'n'-null

  char scroll_state; // 'u' - up; 'd' - down

  bool screen_position_set;
  bool world_position_set;
  Pt screen_pos;
  Pt world_pos;

  int scroll_pos;

  // Which mode is the cursor currently in?
  unsigned int mode;


  ClickableObject *clickedObjectPtr;  //void pointer to a grabbed object
 
  
  std::list<std::pair<timeval,Pt> > world_position_trail;

  TrackedPoint *tracked_point;
  

};






Pt ScreenToWorld(const Pt &p);
Pt WorldToScreen(const Pt &p);


#endif
