#include "key_and_mouse_logger.h" 

#include "interaction.h"


// to make the mouse move faster.... might be good on EMPACs 4K projector
//#define RELATIVE_MOUSE_SCALE 2


//GLOBALS
//===================================================================
Pt GLOBAL_PRIMARY_MOUSE_POS(100,100);
Pt GLOBAL_MOUSE_2_POS(200,100);
Pt GLOBAL_MOUSE_3_POS(300,100);
Pt GLOBAL_MOUSE_4_POS(400,100);
Pt GLOBAL_MOUSE_5_POS(500,100);
Pt GLOBAL_MOUSE_6_POS(600,100);

const int MULTIMICE_ID[MULTIMOUSENUM] = {MOUSE_2,MOUSE_3,MOUSE_4,MOUSE_5,MOUSE_6};

Pt MULTIMICE_POS[MULTIMOUSENUM] = {GLOBAL_MOUSE_2_POS, GLOBAL_MOUSE_3_POS,
				    GLOBAL_MOUSE_4_POS, GLOBAL_MOUSE_5_POS, GLOBAL_MOUSE_6_POS};
										

//RED GREEN BLUE ORANGE PURPLE
const Vec3f MULTIMICE_COLOR[5] = {

#if 0
  // Tyler's original colors
  Vec3f(1,0.3,0.3),
  Vec3f(0.3,1,0.3),
  Vec3f(0.4,0.4,1),
  Vec3f(1.0,0.74,0.2),
  Vec3f(0.82,0.43,0.92)

#else
  // SYNENV MAP COLORS
  // see also GRAPHICS_GIT_WORKING_CHECKOUT/synenv/visualizations/MapView/layer_selector/

  // m2
  // purple 152 78 163
  //globalmapcolors["roads"] = 
  Vec3f(152/255.0, 78/255.0, 163/255.0),

  // m3
  // blue 55 126 184
  //globalmapcolors["water"]   = 
  Vec3f(55/255.0, 126/255.0, 184/255.0),

  // m4
  // green 77 175 74
  //globalmapcolors["comm"]    = 
  Vec3f(77/255.0, 175/255.0, 74/255.0),  

  // m5
  // brown 166 86 40
  //globalmapcolors["waste"]   = 
  Vec3f(166/255.0, 86/255.0, 40/255.0),

  // m6
  // red 228 26 28
  //globalmapcolors["power"]   = 
  Vec3f(228/255.0, 26/255.0, 28/255.0)
  
#endif

};

std::vector<KeyOrMouseAction> GLOBAL_key_and_mouse_actions;


//STATIC FUNCTIONS
//==============================================================================================


bool is_keyboard(int k) { return (k == PRIMARY_KEYBOARD || k == KEYBOARD_2); }
bool is_mouse(int m) { return (m == PRIMARY_MOUSE || m == MOUSE_2 || m == MOUSE_3 || m == MOUSE_4 || m == MOUSE_5 || m == MOUSE_6); }

Pt& get_GLOBAL_MOUSE_Pt(int device) {
  assert (is_mouse(device));
//  assert (device != PRIMARY_MOUSE);

  if (device == PRIMARY_MOUSE ) return GLOBAL_PRIMARY_MOUSE_POS;
  if (device == MOUSE_2) return GLOBAL_MOUSE_2_POS;
  if (device == MOUSE_3) return GLOBAL_MOUSE_3_POS;
  if (device == MOUSE_4) return GLOBAL_MOUSE_4_POS;
  if (device == MOUSE_5) return GLOBAL_MOUSE_5_POS;
  if (device == MOUSE_6) return GLOBAL_MOUSE_6_POS;
  assert(0);
  exit(0);
}

//==============================================================================================

std::string keyboard_and_mouse_CURRENT_filename(const std::string &temp) {
  std::string prefix   = temp.substr(0,temp.size()-7);
  std::string fill     = temp.substr(temp.size()-7,3);
  std::string suffix   = temp.substr(temp.size()-4,4);
  assert (fill == "XXX");
  assert (suffix == ".txt");
  std::stringstream ss;
  ss << prefix << "CURRENT" << suffix;
  std::string answer = ss.str();
  //  assert (answer.size() == temp.size());
  return answer;
}

std::string keyboard_and_mouse_filename(const std::string &temp, int num) {
  num %= MAX_KEYBOARD_AND_MOUSE_FILES;

  std::string prefix   = temp.substr(0,temp.size()-7);
  std::string fill     = temp.substr(temp.size()-7,3);
  std::string suffix   = temp.substr(temp.size()-4,4);
  assert (fill == "XXX");
  assert (suffix == ".txt");
  assert (num >= 0 && num < 1000);
  std::stringstream ss;
  ss << prefix << std::setfill('0') << std::setw(3) << num << suffix;
  std::string answer = ss.str();
  assert (answer.size() == temp.size());
  return answer;
}

//==================================================================================================



void log_keypress(int which_keyboard, int key, int scancode, int action, int mods) {
  //void log_keypress(int which_keyboard, unsigned char key, int x, int y, int glfw_modifiers) {
  KeyOrMouseAction km = KeyOrMouseAction::make_keypress(which_keyboard,key,scancode,action,mods);
//  std::cout << "LOG KEY ACTION " << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}

/*
void log_specialkeypress(int which_keyboard, int key, int x, int y, int glfw_modifiers) {
  KeyOrMouseAction km = KeyOrMouseAction::make_specialkeypress(which_keyboard,key,x,y,glfw_modifiers);
//  std::cout << "LOG KEY ACTION " << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}
*/

void log_mouseaction(int which_mouse, int button, int action, int glfw_modifiers) {
  //void log_mouseaction(int which_mouse, int button, int action, int x, int y, int glfw_modifiers) {
  KeyOrMouseAction km = KeyOrMouseAction::make_mouseaction(which_mouse,button,action,glfw_modifiers);
//  std::cout << "LOG MOUSE ACTION " << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}

void log_mousemotion(int which_mouse, double x, double y) { 
  KeyOrMouseAction km = KeyOrMouseAction::make_mousemotion(which_mouse,x,y); //,glfw_modifiers);
//  std::cout << "LOG MOUSE MOTION " << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}

void log_mousescroll(int which_mouse, double x, double y) { 
  KeyOrMouseAction km = KeyOrMouseAction::make_mousescroll(which_mouse,x,y); //,glfw_modifiers);
//  std::cout << "LOG MOUSE MOTION " << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}


/*
void log_mouseaction_relative(int which_mouse, int button, int action, int glfw_modifiers) { 
  KeyOrMouseAction km = KeyOrMouseAction::make_mouseaction_relative(which_mouse,button,action,glfw_modifiers);
//  std::cout << "LOG MOUSE ACTION RELATIVE" << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}
*/

void log_mousemotion_relative(int which_mouse, double delta_x, double delta_y) {
  KeyOrMouseAction km = KeyOrMouseAction::make_mousemotion_relative(which_mouse,delta_x,delta_y);
//  std::cout << "LOG MOUSE MOTION RELATIVE" << km << std::endl;
  GLOBAL_key_and_mouse_actions.push_back(km);
}

int next_frame_to_be_written(DirLock &global_mk_dirlock, 
				    const std::string &mk_action_filename_template) {
  
  //std::cout << "next_frame enter" << std::endl;
  while (!global_mk_dirlock.TryLock()) { usleep(1000); }

  int answer;
  
  { /* SCOPE FOR istr */
    // try to open the next file
    std::string filename = keyboard_and_mouse_CURRENT_filename(mk_action_filename_template);
    //std::cout << "Try to open " << filename.c_str() << std::endl;
    std::ifstream istr(filename.c_str());
    if (!istr) {
      // the file we want hasn't been created yet
      answer = 0;
    } else {
      std::string token;
      int frame;
      istr >> token >> frame;
      assert (token == "next_frame");
      answer = frame;
    }
  } /* SCOPE FOR istr */

  global_mk_dirlock.Unlock();
  //  std::cout << "next_frame exit" << std::endl;
  return answer;
}




void set_next_frame_to_be_written(const std::string &mk_action_filename_template, int next_frame) {

  //  assert (next_frame >= 0 && next_frame < MAX_KEYBOARD_AND_MOUSE_FILES);
  
  // directory should already be locked!
  //while (!global_mk_dirlock.TryLock()) { usleep(1000); }

  { /* SCOPE FOR ostr */
    // try to open the next file
    std::string filename = keyboard_and_mouse_CURRENT_filename(mk_action_filename_template);
    std::ofstream ostr(filename.c_str());
    assert (ostr);
    ostr << "next_frame " << next_frame << std::endl;
  } /* SCOPE FOR ostr */

  // directory should stay locked
  //global_mk_dirlock.Unlock();
}

// A function to take the place of load_and_save_key_and_mouse_data below
bool load_and_save_key_and_mouse_data( DirLock &global_mk_dirlock, 
                                       const std::string &mk_action_filename_template,
                                       vector< KeyOrMouseAction* > &actions )
                                       //int &which_device, int &x, int &y, int &glfw_modifiers, int& button, int &action,
                                       //const int this_device ) // this_device is the device calling this function
{
  static int load_frame_counter = next_frame_to_be_written(global_mk_dirlock,mk_action_filename_template);
  static int save_frame_counter = load_frame_counter;

  bool something_happened = false;

  while (!global_mk_dirlock.TryLock()) { usleep(1000); }

  { /* SCOPE FOR istr */
  while (1) {
    // try to open the next file
    std::string filename = keyboard_and_mouse_filename(mk_action_filename_template, load_frame_counter); // % MAX_KEYBOARD_AND_MOUSE_FILES);
    std::ifstream istr(filename.c_str());
    if (!istr) {
	  // the file we want hasn't been created yet
	  break;
    } 
    
    std::string token;
	int frame;
    istr >> token >> frame;
    assert (token == "frame");

    // Ignore the frame you just read and any before it
    if (frame+1 <= load_frame_counter) {	  
      break;
    } // end of if

    if (frame > load_frame_counter) {
      std::cout << "WARNING: MISSED MOUSE & KEYBOARD ACTIONS" << std::endl;
      std::cout << "frame=" << frame << " load_frame_counter=" << load_frame_counter << std::endl;
    } // end of if

	int num_key_and_mouse_actions;
	istr >> token >> num_key_and_mouse_actions;
	assert (token == "num_key_and_mouse_actions");
	assert (num_key_and_mouse_actions >=1 );
    KeyOrMouseAction km;

	for (int i = 0; i < num_key_and_mouse_actions; i++) {
      // Read in the key/mouse action
      istr >> km;
      // Add it to the list of actions
      KeyOrMouseAction* newAction = new KeyOrMouseAction( km );
      actions.push_back( newAction );
    } // end of for

    load_frame_counter = (load_frame_counter+1);
    something_happened = true;

    
  } // end of while
  } // end of istr scope

  assert (load_frame_counter >= save_frame_counter);
  save_frame_counter = load_frame_counter;

  // ======================================================
  // NOW SAVE YOUR OWN ACTIONS
  { /* SCOPE FOR  ostr */
    if (GLOBAL_key_and_mouse_actions.size() != 0) {
      
      something_happened = true;

      std::string filename = keyboard_and_mouse_filename(mk_action_filename_template, save_frame_counter); // % MAX_KEYBOARD_AND_MOUSE_FILES);
      std::ofstream ostr(filename.c_str());
      assert (ostr);
      ostr << "frame " << save_frame_counter << "\n";
      ostr << "num_key_and_mouse_actions " << GLOBAL_key_and_mouse_actions.size() << "\n";
      for (unsigned int i = 0; i < GLOBAL_key_and_mouse_actions.size(); i++) {
        ostr << GLOBAL_key_and_mouse_actions[i];
      } // end of for
      save_frame_counter = (save_frame_counter+1); // % MAX_KEYBOARD_AND_MOUSE_FILES;
      set_next_frame_to_be_written(mk_action_filename_template,save_frame_counter);
    } // end of if
  } /* SCOPE FOR istr & ostr */

  global_mk_dirlock.Unlock();

  GLOBAL_key_and_mouse_actions.clear();

  return something_happened;

} // end of load_and_save



// This function calls back functions passed to it based on what the cursor input does
bool load_and_save_key_and_mouse_data(DirLock &global_mk_dirlock, 
                                      const std::string &mk_action_filename_template,
                                      void (*my_keyfunc)(int which_keyboard, int key, int scancode, int action, int glfw_modifiers),
                                      //void (*my_specialkeyfunc)(int which_keyboard, int specialkey, int x, int y, int glfw_modifiers),
                                      void (*my_mousefunc)(int which_mouse, int button, int action, int glfw_modifiers),
                                      void (*my_motionfunc)(int which_mouse, double x, double y, int glfw_modifiers), 
                                      void (*my_scrollfunc)(int which_mouse, double x, double y, int glfw_modifiers)){
  
  //  std::cout << "next frame... " << std::endl;

  static int load_frame_counter = next_frame_to_be_written(global_mk_dirlock,mk_action_filename_template);
  static int save_frame_counter = load_frame_counter;

  bool something_happened = false;

  while (!global_mk_dirlock.TryLock()) { usleep(1000); }
  
  // ======================================================
  // FIRST CHECK TO SEE IF OTHER THREADS HAVE SAVED ACTIONS
  { /* SCOPE FOR istr */
    while (1) {

      // try to open the next file
      std::string filename = keyboard_and_mouse_filename(mk_action_filename_template, load_frame_counter); // % MAX_KEYBOARD_AND_MOUSE_FILES);
      std::ifstream istr(filename.c_str());
      if (!istr) {
	// the file we want hasn't been created yet
	break;
      } else {
	std::string token;
	int frame;
	istr >> token >> frame;
	assert (token == "frame");

	if (frame+1 == load_frame_counter) {	  
	  // this is the fram I just read, ignore it
	  break;
	}

	if (frame < load_frame_counter) {	  
	  /*  static bool first_time = true;
	  if (first_time) {
	    first_time = false;
	  } else {
	    std::cout << "WARNING: SOMETHING BUGGY WITH THE FRAME COUNTER" << std::endl;
	    std::cout << "frame=" << frame << " load_frame_counter=" << load_frame_counter << std::endl;
	    
	    // TAKE THIS OUT BEFORE THE DEMO
	    exit(0);
	  }
	  */
	  break;
	}
	//	if (frame != load_frame_counter) {	  
	if (frame > load_frame_counter) {
	  std::cout << "WARNING: MISSED MOUSE & KEYBOARD ACTIONS" << std::endl;
	  std::cout << "frame=" << frame << " load_frame_counter=" << load_frame_counter << std::endl;

	  // TAKE THIS OUT BEFORE THE DEMO
	  //exit(0);

	}

	// BARB NEEDS TO FIX
	//  if (frame >= load_frame_counter) {
	//std::cout << "	  assert (frame <= load_frame_counter)" << std::endl;
	// }
	  //	  assert (frame <= load_frame_counter);	
	  // not the file we want (it's old, we've already read it)
	  //break;	  
	//} else {
	  // yeah! this is the file we want
	  //std::cout << "try to LOAD KEYBOARD ACTIONS " << load_frame_counter << filename << std::endl;	  
	  int num_key_and_mouse_actions;
	  istr >> token >> num_key_and_mouse_actions;
	  assert (token == "num_key_and_mouse_actions");
	  assert (num_key_and_mouse_actions >=1 );
	  KeyOrMouseAction km;
	  for (int i = 0; i < num_key_and_mouse_actions; i++) {
	    istr >> km;
            //std::cout << "read this: " << km << std::endl;
            // Standard keypress on a keyboard
	    if (km.keypress) {
	      my_keyfunc(km.which_device,km.key,km.scancode,km.action,km.glfw_modifiers);
              /*
	    } else if (km.specialkeypress) {
	      my_specialkeyfunc(km.which_device,km.specialkey,km.x,km.y,km.glfw_modifiers);
              */
	    } else if (km.mouseaction) {
	      my_mousefunc(km.which_device,km.button,km.action,km.glfw_modifiers);
	    } else if (km.mousemotion && !km.relative) {
	      assert (km.which_device == PRIMARY_MOUSE);
	      GLOBAL_PRIMARY_MOUSE_POS = Pt(km.x,km.y);
	      my_motionfunc(km.which_device,km.x,km.y, km.glfw_modifiers);
	    } else if(km.mousescroll) {
            //TODO: SHOULDNT PASS IN 0 FOR GLFW_MODIFIERS
            my_scrollfunc(km.which_device, km.x, km.y, 0 );    
        }/*else if (km.mouseaction && km.relative) {
	      assert (km.which_device != PRIMARY_MOUSE);
	      assert (is_mouse(km.which_device));
	      Pt &pt = get_GLOBAL_MOUSE_Pt(km.which_device);
	      clamp_to_display(pt);
	      my_mousefunc(km.which_device,km.button,km.action,km.glfw_modifiers);//pt.x,pt.y,0);
	    } */
        else {
	      assert (km.mousemotion && km.relative);
	      assert (km.which_device != PRIMARY_MOUSE);
	      assert (is_mouse(km.which_device));
	      Pt &pt = get_GLOBAL_MOUSE_Pt(km.which_device);
	      pt += Pt(RELATIVE_MOUSE_SCALE*km.x,-RELATIVE_MOUSE_SCALE*km.y);
	      clamp_to_display(pt);
	      my_motionfunc(km.which_device,pt.x,pt.y, km.glfw_modifiers);
	    }
	  //std::cout << "done LOAD KEYBOARD ACTIONS" << std::endl;
	  load_frame_counter = (load_frame_counter+1); // % MAX_KEYBOARD_AND_MOUSE_FILES;
	  //set_next_frame_to_be_written(mk_action_filename_template,frame_counter);
	  //frame_counter++;
          something_happened = true;
      }
      }
    }
  } /* SCOPE FOR istr */

  assert (load_frame_counter >= save_frame_counter);
  save_frame_counter = load_frame_counter;

  // ======================================================
  // NOW SAVE YOUR OWN ACTIONS
  { /* SCOPE FOR  ostr */
    if (GLOBAL_key_and_mouse_actions.size() != 0) {
      
      something_happened = true;

      std::string filename = keyboard_and_mouse_filename(mk_action_filename_template, save_frame_counter); // % MAX_KEYBOARD_AND_MOUSE_FILES);
      //std::cout << "try to SAVE KEYBOARD ACTIONS " << save_frame_counter << " " << filename << std::endl;
      std::ofstream ostr(filename.c_str());
      assert (ostr);
      ostr << "frame " << save_frame_counter << "\n";
      ostr << "num_key_and_mouse_actions " << GLOBAL_key_and_mouse_actions.size() << "\n";
      for (unsigned int i = 0; i < GLOBAL_key_and_mouse_actions.size(); i++) {
	ostr << GLOBAL_key_and_mouse_actions[i];
      } 
      //std::cout << "done SAVE KEYBOARD ACTIONS" << std::endl;
      save_frame_counter = (save_frame_counter+1); // % MAX_KEYBOARD_AND_MOUSE_FILES;
      set_next_frame_to_be_written(mk_action_filename_template,save_frame_counter);
    }

  } /* SCOPE FOR istr & ostr */
  global_mk_dirlock.Unlock();

  GLOBAL_key_and_mouse_actions.clear();

  if (something_happened) {
    //std::cout << "something" << std::endl;
    //exit(0);
  } else {
    //std::cout << "nothing" << std::endl;
  }

  return something_happened;
}

// =======================================================================
// =======================================================================
