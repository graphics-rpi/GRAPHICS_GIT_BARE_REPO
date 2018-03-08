#ifndef _INTERACTION_H_
#define _INTERACTION_H_

#include "../applications/paint/gl_includes.h"
#include "../applications/paint/shader_manager.h"

#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string>

#include "../calibration/planar_interpolation_calibration/tracker.h"
#include "../calibration/planar_interpolation_calibration/tiled_display.h"
#include "../calibration/planar_interpolation_calibration/colors.h"


// --------------------
// SHOULD NOT BE IN THE GRAPH INTERACTION DIRECTORY...  
// SHOULD NOT USE DRAWN NODE
#include "action.h"
// --------------------

#include "../applications/paint/path.h"
#include "../applications/paint/button.h"
#include "multiMouse.h"
#include "laser.h"
#include "key_and_mouse_logger.h"
#include "cursor.h"


using std::string;
using std::cerr;
using std::endl;
using std::vector;

// Mouse button IDs
const int LEFT_CLICK = 0;
const int RIGHT_CLICK = 2;
// Needs to be changed if changed in setupMultiMice
const int PRIMARY_MOUSE_CURSOR_ID = 1000;

// Mouse double-click timeout in clock cycles
const unsigned int DOUBLE_CLICK_TIMEOUT = 250;
//const unsigned int DOUBLE_CLICK_TIMEOUT = 500;

// Number of frames before a laser is discontinuous or inactive
const unsigned int INACTIVE_THRESHOLD = 250;
const unsigned int DISCONTINUOUS_THRESHOLD = 250;

//Function Prototypes===========================================================================


#define IR_STATE_DIRECTORY "../state/ir_tracking"
#define MK_STATE_DIRECTORY "../state/mouse_and_keyboard"
#define MK_ACTION_FILENAME_TEMPLATE "../state/mouse_and_keyboard/actions_XXX.txt"
#define FOUND_IR_POINTS_FILENAME "../state/ir_tracking/found_ir_points.txt"
#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME "../state/ir_tracking/planar_calibration_intensity_data.txt"
#define default_laser_tail_length 30
#define PIXEL_MOTION_TOLERANCE 10

namespace CursorVBO{
    enum Enum {
        POSITION,
        COLOR
    };
}

namespace CursorVAO{
    enum Enum {
        OUTLINE,
        SHAPE
    };
}

class Interaction {
public:
  
  //Interaction(); 
  
  // this wasn't working for mapview (possibly related to library??)

    // Constants

  /*    const static string IR_STATE_DIRECTORY;
    const static string MK_STATE_DIRECTORY;
    const static string MK_ACTION_FILENAME_TEMPLATE;
    const static string FOUND_IR_POINTS_FILENAME;
    const static string PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME;
    const static string PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME;
    const static int default_laser_tail_length;
    // The pixel tolerance for laser detection
    const static unsigned int PIXEL_MOTION_TOLERANCE;
  */

    // LASER FUNCTIONS
    static void CheckForLaserActionInitializations();
    static void HandleLaserActions();

    // OpenGL Function to draw the cursors
    static void drawCursors( bool screen_space=false, const Vec3f &background_color = Vec3f(0,0,0), glm::mat4 projection = glm::mat4(1.0f) );
    static void drawMultiMice( bool screen_space, glm::mat4 projection );
    

    static void drawCursorTrails( bool screen_space, const Vec3f &background_color, double seconds, timeval cur );
    static void drawStrokes( bool );

    // Helper Functions (DO WE NEED THESE? SHOULD THEY GO HERE?)
    static void keyfunc_helper(int which_keyboard, int key, int scancode, int action, int glut_modifiers);
  //static void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers);
    static void mousefunc_helper(int which_mouse, int button, int action, int glut_modifiers);
    static void motionfunc_helper(int which_mouse, int x, int y); //, int glut_modifiers);
    static void scrollfunc_helper(int which_mouse, double x, double y);

    friend void clamp_to_display(Pt &pt);

    // Function to determine which call backs to use given the current state of the cursors
    static void determineCallBacks();

    // Accessors
    static Cursor* getCursor( int index ) { return cursorVec[ index ]; }
    
    static unsigned int getNumCursors( ) { return cursorVec.size(); }


    // A list of laser actions
    static std::map<int,Action> LActions;

    //TYLER
    // Public setup methods
    static void setupCursors(TiledDisplay *td, void (*add)(TrackedPoint *pt)=NULL, void (*remove)(TrackedPoint *pt)=NULL);
    static void invertCursorYAxis(){ invert_cursor_y = true; }

    // Registration functions to register app functions as gesture call backs
    static void registerMove( void (*func)(Cursor *c) ) { moveFunc = func; }
    static void registerSelect( void (*func)(Cursor *c) ) { selectFunc = func; }
    static void registerExpand( void (*func)(Cursor *c) ) { expandFunc = func; }
    static void registerSimplify( void (*func)(Cursor *c) ) { simplifyFunc = func; }
    static void registerGroup( void (*func)(Cursor *c) ) { groupFunc = func; }
    static void registerUngroup( void (*func)(Cursor *c) ) { ungroupFunc = func; }
    static void registerChange( void (*func)(Cursor *c) ) { changeFunc = func; }
    static void registerRelease( void (*func)(Cursor *c) ) { releaseFunc = func; }
    static void registerLock( void (*func)(Cursor *c) ) { lockFunc = func; }
    static void registerZoom( void (*func)(Cursor *c) ) { zoomFunc = func; }

    // Constant values for gesture types
    static const unsigned int MOVE = 1;
    static const unsigned int SELECT = 2;
    static const unsigned int EXPAND = 3;
    static const unsigned int SIMPLIFY = 4;
    static const unsigned int GROUP = 5;
    static const unsigned int UNGROUP = 6;
    static const unsigned int CHANGE = 7;
    static const unsigned int RELEASE = 8;
    static const unsigned int LOCK = 9;
    static const unsigned int ZOOM = 10;


    // List of colors
    static Colors global_colors;

  static double interface_scale;
  static Pt interface_offset;

private:
    // Vector of all the cursors in the system
    static std::vector<Cursor*> cursorVec;

    //Tyler (for audio interface)
    static bool invert_cursor_y;


    // Track the IR Points for the lasers
    static PointTracker *global_point_tracker;
    static PlanarCalibration *global_calibration_data;
    static TiledDisplay *tiled_display;


    // LASER FUNCTIONALITY
    //    static int laserIDs[ NUM_LASERS ];
    static std::vector<Laser*> laserVec;
    static void ReadAndProcessLaserData();
    //    static unsigned int numFramesOff[ NUM_LASERS ];

    // Callback functions
    static void dealWithKeyboardAndMice();
    static void dealWithLasers();

    // MULTI-MOUSE FUNCTIONALITY
    static std::vector<MultiMouse*> multiMouseVec;
    static int decodeMultiMice(int mouseId);
    static int getGestureFromAction( KeyOrMouseAction* );


    // Private setup methods
    static void setupMultiMice();
    static void setupLasers(void (*add)(TrackedPoint *pt), void (*remove)(TrackedPoint *pt));
    static void setupColors();

    // GENERAL CURSOR FUNCTIONS
    static void updateCursorPosition( int cursorIndex, KeyOrMouseAction* km );
    static void updateMainMouseCursorPosition( Pt p );

    // Directory Locks
    static DirLock global_ir_dirlock;
    static DirLock global_mk_dirlock;

    // CHRIS STUETZLE
    // Callback functions: One for each possible gesture, assigned in the application
    static void (*moveFunc)(Cursor *c);
    static void (*selectFunc)(Cursor *c);
    static void (*expandFunc)(Cursor *c);
    static void (*simplifyFunc)(Cursor *c);
    static void (*groupFunc)(Cursor *c);
    static void (*ungroupFunc)(Cursor *c);
    static void (*changeFunc)(Cursor *c);
    static void (*releaseFunc)(Cursor *c);
    static void (*lockFunc)(Cursor *c);
    static void (*zoomFunc)(Cursor *c);

    // OPENGL 3.0 Functionality
    static GLuint m_vao[2];
    static GLuint m_vbo[2];
    static GLuint m_outline_vbo[2];

};


void RemoveTrackedPoint(TrackedPoint *pt );
void AddTrackedPoint(TrackedPoint *pt );

void error_callback(int error, const char* description);
std::string WhichGLError(GLenum &error);
int HandleGLError(const std::string &message="", bool ignore=false);


#endif
