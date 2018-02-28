// ====================================================================

#ifndef _GL_CANVAS_H_
#define _GL_CANVAS_H_

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <glui.h>
#include <string>

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#define GL_GLEXT_PROTOTYPES
#ifdef _WIN32
#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/glew.h>
#endif
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef _WIN32
#include "GL/wglew.h"
#endif
#endif


class MeshManager;
class Camera;
class Command;
class CommandQueue;

// ====================================================================

class GLCanvas {

public:

  static void initialize(MeshManager *_meshes); 
  static void redisplayCB(int);
  static void rerender_sceneCB(int);
  static void rerender_select_geometryCB(int);
  static void updateCounts();
  static void refresh();
  static void clear();

private:

  static void TakeAction();

  // Callback functions for mouse and keyboard events
  static void InitLight();
 public:
  static void display(void);
 private:
  static void reshape(int w, int h);
  static void mouse(int button, int state, int x, int y);
  static void motion(int x, int y);
  static void idle();
  static void keyboard_down(unsigned char key, int x, int y);

  static char currentKey();
  static void setKey(char key);
  static char whichKeyOnMousePress();
  static void setKeyOnPress();
  static void clearKeyOnRelease();

  static void setup_glui(int main_win);



  static void loadCB(int);
  static void cutThroughPlanesCB(int);
  static void cheatFakeMaterialsCB(int);

  static void saveRemeshCB(int);

  static void remeshCB(int);
  static void BadTrianglesStatusCB(int);
  static void eliminateBadTrianglesCB(int);
  static void subdivideCB(int);
  static void splitEdgesCB(int);
  static void flipEdgesCB(int);
  static void moveVerticesCB(int);
  static void fixSeamsCB(int);
  static void triangulateCB(int);
  static void cutEdgesCB(int);
  static void collapseEdgesCB(int);

  static void moveVerticesRandomlyCB(int);
  static void compressVerticesCB(int);
  static void assignPatchesAndZonesCB(int);
  static void seedPatchesCB(int);
  static void iteratePatchesCB(int);
  static void randomizePatchAndZoneColorsCB(int);
  static void assignZonesCB(int);

  static void seedCB(int);
  static void clusterCB(int);

  static void normalizeClusterWeightsCB(int);

  static void ClipCB(int);

  static void resetCameraCB(int);
  static void quitCB(int);
  static void nullCB(int) { }



  static void setCounts(int vertices, int triangles, int quads, int bad_elements, int patches);

  static void PaintDensity(int density, int x, int y);
  

  static GLUI_Panel* setup_rollout_helper(char *label, bool visible = false);
  static void setup_file_helper(GLUI_Panel *p, char *button, char *label, 
					  GLUI_String &var, GLUI_Update_CB CB);

  static void setup_button_helper(GLUI_Panel *p, char *button, GLUI_Update_CB CB);

  static void setup_spinner_helper_float(GLUI_Panel *p, char *label, float min_val, float max_val, float *val, GLUI_Update_CB CB = NULL);
  static void setup_spinner_helper_int(GLUI_Panel *p, char *label, int min_val, int max_val, int *val, GLUI_Update_CB CB = NULL);

  // fabrication 
  static void createTilesCB(int);
  static void unrollTilesCB(int);
  static void saveTilesCB(int);




  // ===================================
  // VARIABLES
  static MeshManager *meshes;
  static Camera *camera;
  static CommandQueue *command_queue;

  static int main_window;

  // State of the mouse cursor
  static int mouseButton;
  static int mouseX;
  static int mouseY;
  static bool shiftPressed;
  static bool controlPressed;
  static bool altPressed;
  
  static GLUI_StaticText *vertices_text;
  static GLUI_StaticText *triangles_text;
  static GLUI_StaticText *patches_text;
  static GLUI_StaticText *quads_text;
  static GLUI_StaticText *bad_elements_text;

  static GLUI *glui;
};

// ====================================================================

int HandleGLError(std::string);

#endif
