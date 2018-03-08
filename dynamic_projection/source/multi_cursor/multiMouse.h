#ifndef _MULTIMOUSE_H_
#define _MULTIMOUSE_H_

#include "../applications/paint/gl_includes.h"
#include "cursor.h"


using std::cerr;
using std::cout;
using std::endl;

class MultiMouse : public Cursor {

public:
  MultiMouse(const std::string& n, Pt p, int mid, int cid, const Vec3f& c);

  int getMouseId(){return mouseId;}

  // Inherited from cursor, must be implemented
  void draw(bool screen_space, const Vec3f &background_color) const;
  int getCurrentStateAndGesture() { return -1; }

  std::vector<glm::vec2> getCursorShape();
  std::vector<glm::vec2> getCursorOutline();

  glm::vec3 getCursorOutlineColor();

  // This is a function to update the position of the mouse
  void updatePosition( const Pt u );

  virtual bool isMouse() { return true; }
  
  unsigned long getTimeOfPreviousClick() const { return timeOfPreviousClick; }
  void setTimeOfPreviousClick(unsigned long v) { timeOfPreviousClick = v; }

 protected:
  //INHERITED FROM CURSOR
  
  //std::string name
  //Pt position;
  //Vec3f color;
  //char state;
  
  //char scroll_state;
  //int scroll_pos;
  
  int mouseId;


 private:
//  static void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers) {}
//  static void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers) {}
//  static void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers);
//  static void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers);

  unsigned long timeOfPreviousClick;


};

#endif
