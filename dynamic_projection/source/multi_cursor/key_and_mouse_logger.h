#ifndef __KEYBOARD_LOGGER_H__
#define __KEYBOARD_LOGGER_H__

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>

// Included files for OpenGL Rendering
#include "../applications/paint/gl_includes.h"


#include "../common/directory_locking.h"
#include "../calibration/planar_interpolation_calibration/planar_calibration.h"

#include "cursor.h"
#include "multiMouse.h"

using std::vector;

// =======================================================================

#define PRIMARY_KEYBOARD  101
#define KEYBOARD_2        102

#define PRIMARY_MOUSE     201
#define MOUSE_2           202
#define MOUSE_3           203
#define MOUSE_4           204
#define MOUSE_5						205
#define MOUSE_6						206
#define MULTIMOUSENUM			5

#define RELATIVE_MOUSE_SCALE 1


//MICE.push_back(201);
//MICE.push_back(202);
//MICE.push_back(203);
//MICE.push_back(204);
//MICE.push_back(205);
//MICE.push_back(206);


#define NUM_NON_PRIMARY_MICE 5

extern Pt GLOBAL_PRIMARY_MOUSE_POS;
extern Pt GLOBAL_MOUSE_2_POS;
extern Pt GLOBAL_MOUSE_3_POS;
extern Pt GLOBAL_MOUSE_4_POS;
extern Pt GLOBAL_MOUSE_5_POS;
extern Pt GLOBAL_MOUSE_6_POS;

extern const int MULTIMICE_ID[MULTIMOUSENUM];
extern Pt MULTIMICE_POS[MULTIMOUSENUM];
extern const Vec3f MULTIMICE_COLOR[MULTIMOUSENUM];
//extern multiMouse M2;
//extern multiMouse M3;
//extern multiMouse M4;
//extern multiMouse M5;
//extern multiMouse M6;


// defined by the application!
void clamp_to_display(Pt &pt);

#define MAX_KEYBOARD_AND_MOUSE_FILES 1000

//FUNCTION PROTOTYPES
//====================================

bool is_keyboard(int k);
bool is_mouse(int m);
Pt& get_GLOBAL_MOUSE_Pt(int device);

// =======================================================================
// =======================================================================

class KeyOrMouseAction {
 public:
  KeyOrMouseAction() {
    keypress = false;
    //    specialkeypress = false;
    mouseaction = false;
    mousemotion = false;
    mousescroll = false;
    which_device = -1;
    button = -1;
    action = -1;
    key = 0;
    scancode = 0;
    //    specialkey = 0;
    x = 0;
    y = 0;
    relative = false;
    glfw_modifiers = 0;
  }

  /*
    // default copy constructor is fine!
  KeyOrMouseAction( const KeyOrMouseAction &A ) {
    this -> keypress = A.keypress;
    this -> specialkeypress = A.specialkeypress;
    this -> mouseaction = A.mouseaction;
    this -> mousemotion = A.mousemotion;
    this -> which_device = A.which_device;
    this -> button = A.button;
    this -> action = A.action;
    this -> key = A.key;
    this -> x = A.x;
    this -> y = A.y;
    this -> relative = A.relative;
    this -> glfw_modifiers = A.glfw_modifiers;
  } // copy constructor
  */

  static KeyOrMouseAction make_keypress(int _which_keyboard, int _key, int _scancode, int _action, int _glfw_modifiers) {
    assert (is_keyboard(_which_keyboard));
    KeyOrMouseAction answer;
    answer.keypress = true;
    answer.which_device = _which_keyboard;
    answer.key = _key;
    answer.scancode = _scancode;
    answer.action = _action;
    answer.glfw_modifiers = _glfw_modifiers;
    return answer;
  }

  /*
  static KeyOrMouseAction make_specialkeypress(int _which_keyboard, unsigned char _specialkey, int _x, int _y, int _glfw_modifiers) {
    assert (is_keyboard(_which_keyboard));
    KeyOrMouseAction answer;
    answer.specialkeypress = true;
    answer.which_device = _which_keyboard;
    answer.specialkey = _specialkey;
    answer.x = _x;
    answer.y = _y;
    answer.glfw_modifiers = _glfw_modifiers;
    return answer;
  }
  */

  static KeyOrMouseAction make_mouseaction(int _which_mouse, int _button, int _action, int _glfw_modifiers) {
    assert (is_mouse(_which_mouse));
    KeyOrMouseAction answer;
    answer.mouseaction = true;
    answer.which_device = _which_mouse;
    answer.button = _button;
    answer.action = _action;
    answer.glfw_modifiers = _glfw_modifiers;
    return answer;
  }

  static KeyOrMouseAction make_mousemotion(int _which_mouse, double _x, double _y) { //, int _glfw_modifiers) {
    assert (is_mouse(_which_mouse));
    KeyOrMouseAction answer;
    answer.mousemotion = true;
    answer.which_device = _which_mouse;
    answer.x = _x;
    answer.y = _y;
    //answer.glfw_modifiers = _glfw_modifiers;
    return answer;
  }

  static KeyOrMouseAction make_mousescroll(int _which_mouse, double _x, double _y) { //, int _glfw_modifiers) {
    assert (is_mouse(_which_mouse));
    KeyOrMouseAction answer;
    answer.mousescroll = true;
    answer.which_device = _which_mouse;
    answer.x = _x;
    answer.y = _y;
    //answer.glfw_modifiers = _glfw_modifiers;
    return answer;
  }

  /*
  static KeyOrMouseAction make_mouseaction_relative(int _which_mouse, int _button, int _action) {
    assert (is_mouse(_which_mouse));
    KeyOrMouseAction answer;
    answer.mouseaction = true;
    answer.which_device = _which_mouse;
    answer.button = _button;
    answer.action = _action;
    answer.relative = true;
    return answer;
  }
  */

  static KeyOrMouseAction make_mousemotion_relative(int _which_mouse, double _delta_x, double _delta_y) {
    assert (is_mouse(_which_mouse));
    KeyOrMouseAction answer;
    answer.mousemotion = true;
    answer.which_device = _which_mouse;
    answer.x = _delta_x;
    answer.y = _delta_y;
    answer.relative = true;
    return answer;
  }

  bool keypress;
  //  bool specialkeypress;
  bool mouseaction;
  bool mousemotion;
  bool mousescroll;
  int which_device;
  int button;
  int action;
  int key;
  int scancode;
  //  int specialkey;
  double x;
  double y;
  bool relative;
  int glfw_modifiers;

  friend std::ostream& operator<< (std::ostream &ostr, const KeyOrMouseAction &km) {
    if (km.keypress) {
      ostr << "keypress " << km.which_device << " " << km.key << " " << km.scancode << " " << km.action << " " << km.glfw_modifiers << "\n";
    } else if (km.mouseaction && !km.relative) {
      ostr << "mouseaction " << km.which_device << " " << km.button << " " << km.action << " " << " " << km.glfw_modifiers << "\n";
    } else if (km.mousemotion && !km.relative) {
      ostr << "mousemotion " << km.which_device << " " << km.x << " " << km.y << "\n";
    } else if (km.mousescroll) {
      ostr << "mousescroll " << km.which_device << " " << km.x << " " << km.y << "\n";
    } else {
      assert (km.mousemotion && km.relative);
      ostr << "mousemotion_relative " << km.which_device << " " << km.x << " " << km.y << "\n";
    }
    return ostr;
  }

  friend std::istream& operator>> (std::istream &istr, KeyOrMouseAction &km) {
    std::string token;
    istr >> token;
    km = KeyOrMouseAction();
    if (token == "keypress") {
      km.keypress = true;
      istr >> km.which_device >> km.key >> km.scancode >> km.action >> km.glfw_modifiers;
      assert (is_keyboard(km.which_device));
      /*
   } else if (token == "specialkeypress") {
      km.specialkeypress = true;
      istr >> km.which_device >> km.specialkey >> km.x >> km.y >> km.glfw_modifiers;
      assert (is_keyboard(km.which_device));
      */
    } else if (token == "mouseaction") {
      km.mouseaction = true;
      istr >> km.which_device >> km.button >> km.action >> km.glfw_modifiers;
      assert (is_mouse(km.which_device));
    } else if (token == "mousemotion") {
      km.mousemotion = true;
      istr >> km.which_device >> km.x >> km.y; 
      assert (is_mouse(km.which_device));
    } else if (token == "mousescroll") {
      km.mousescroll = true;
      istr >> km.which_device >> km.x >> km.y; 
      assert (is_mouse(km.which_device));
      /*
    } else if (token == "mouseaction_relative") {
      km.mouseaction = true;
      istr >> km.which_device >> km.button >> km.action;
      km.relative = true;
      assert (is_mouse(km.which_device));
      */
} else {
      assert (token == "mousemotion_relative");
      km.mousemotion = true;
      km.relative = true;
      istr >> km.which_device >> km.x >> km.y;
      assert (is_mouse(km.which_device));
    }
    return istr;
  }
};

// GLOBAL VARIABLES
// =======================================================================
extern std::vector<KeyOrMouseAction> GLOBAL_key_and_mouse_actions;

//FUNCTION PROTOTYPES
// =======================================================================

std::string keyboard_and_mouse_CURRENT_filename(const std::string &temp);

std::string keyboard_and_mouse_filename(const std::string &temp, int num);

/*
void log_keypress(int which_keyboard, unsigned char key, int x, int y, int glfw_modifiers);
//void log_specialkeypress(int which_keyboard, int key, int x, int y, int glfw_modifiers);
void log_mouseaction(int which_mouse, int button, int action, int x, int y, int glfw_modifiers);
void log_mousemotion(int which_mouse, int x, int y); //, int glfw_modifiers);
*/

void log_keypress(int which_keyboard, int key, int scancode, int action, int mods); 
void log_mouseaction(int which_mouse, int button, int action, int glfw_modifiers);
void log_mousemotion(int which_mouse, double x, double y);
void log_mousescroll(int which_mouse, double x, double y);

//void log_mouseaction_relative(int which_mouse, int button, int action);
void log_mousemotion_relative(int which_mouse, double delta_x, double delta_y);

int next_frame_to_be_written(DirLock &global_mk_dirlock, 
				    const std::string &mk_action_filename_template);

void set_next_frame_to_be_written(const std::string &mk_action_filename_template, int next_frame);

bool load_and_save_key_and_mouse_data(DirLock &global_mk_dirlock, 
                                      const std::string &mk_action_filename_template,
                                      void (*my_keyfunc)(int which_keyboard, int key, int scancode, int action, int glfw_modifiers),
                                      //void (*my_specialkeyfunc)(int which_keyboard, int specialkey, int x, int y, int glfw_modifiers),
                                      void (*my_mousefunc)(int which_mouse, int button, int action, int glfw_modifiers),
                                      void (*my_motionfunc)(int which_mouse, double x, double y),
                                      void (*my_scrollfunc)(int which_mouse, double x, double y));

// Overloaded function
bool load_and_save_key_and_mouse_data( DirLock &global_mk_dirlock, 
                                       const std::string &mk_action_filename_template,
                                       vector< KeyOrMouseAction* > &actions );

#endif
