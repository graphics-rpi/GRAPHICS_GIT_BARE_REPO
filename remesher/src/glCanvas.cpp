#include "OffScreenBuffer.hpp"   // this needs to be first or compile fails on unix... something weird about dependencies :(

// Included files for OpenGL Rendering
/*
  #ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
  #else
  #ifdef _WIN32
  #include <Windows.h>
  #else
  #include <unistd.h>
  #endif
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
  #endif
*/

#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <fstream>

#include <unistd.h>

#include "glCanvas.h"
#include "argparser.h"
#include "camera.h"
#include "mesh.h"
#include "meshmanager.h"
#include "render.h"
#include "meshio.h"
#include "remesh.h"
#include "image.h"

void main2(void);

bool GLOBAL_SNAP=false;

#include "command.h"

// =======================
// GLCanvas variables

int GLCanvas::mouseButton;
int GLCanvas::mouseX;
int GLCanvas::mouseY;
MeshManager* GLCanvas::meshes;
Camera* GLCanvas::camera;
int GLCanvas::main_window;

GLUI* GLCanvas::glui;
GLUI_StaticText* GLCanvas::vertices_text;
GLUI_StaticText* GLCanvas::triangles_text;
GLUI_StaticText* GLCanvas::patches_text;
GLUI_StaticText* GLCanvas::quads_text;
GLUI_StaticText* GLCanvas::bad_elements_text;
CommandQueue *GLCanvas::command_queue;

bool GLCanvas::controlPressed = false;
bool GLCanvas::shiftPressed = false;
bool GLCanvas::altPressed = false;


// =======================

static int main_loop_started = 0;
void DoOnQuit(MeshManager *meshes);
bool IncrementFileName(std::string &tmp2); // defined in main.cpp

#ifdef LINUX
#define SPEED_FACTOR 1
#else
#define SPEED_FACTOR 1
#endif

// ========================================================
// ========================================================


int HandleGLError(std::string foo) {
  GLenum error;
  int i = 0;
  while ((error = glGetError()) != GL_NO_ERROR) {
    printf ("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
    std::cout << foo << std::endl;
    if (error != GL_INVALID_OPERATION) i++;
  }
  if (i == 0) return 1;
  return 0;
}

// ========================================================
// ========================================================

void GLCanvas::InitLight() {
  // Set the last component of the position to 0 to indicate
  // a directional light source
  assert(HandleGLError("init light"));

  Vec3f d = camera->getDir();
  //std::cout << d.x() << " " << d.y() << " " << d.z() << std::endl;

  int mode;
  glGetIntegerv(GL_MATRIX_MODE,&mode);
  //assert (mode == GL_MODELVIEW);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  //  GLfloat position[4] = { 0, 100, 0, 0 };
  //  GLfloat position[4] = { -d.x(), -d.y(), -d.z(), 0}; //0, 10 ,0,1};
  GLfloat position[4];
  position[0] = -d.x();
  position[1] = -d.y();
  position[2] = -d.z();
  position[3] = 0.f;
  //GLfloat position[4] = { 100,100,150, 0};

  GLfloat diffuse[4] = { 0.9,0.9,0.9,1};
  GLfloat specular[4] = { 0.1, 0.1, 0.1, 1.0 };
  GLfloat ambient[4] = { 0.1, 0.1, 0.1, 1.0 };
  if (meshes->args->transparency) {
    diffuse[0]  = 0.2; diffuse[1]  = 0.2; diffuse[2]   = 0.2;
    specular[0] = 1; specular[1] = 1;  specular[2] = 1;
    ambient[0]  = 0.6; ambient[1]  = 0.6;  ambient[2] = 0.6;
  }
  GLfloat zero[4] = {0,0,0,0};
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glLightfv(GL_LIGHT0, GL_AMBIENT, zero);
  glEnable(GL_LIGHT0);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  GLfloat spec_mat[4] = {1,1,1,1};
  float glexponent = 30;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_mat);

  int vis_mode = meshes->args->render_vis_mode;
  if (vis_mode == 0) {
    // white/magenta
    float back_color[] = { 1,0,0.5,1};
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, back_color);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, back_color);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE); // call before enabling GL_COLOR_MATERIAL
    glEnable(GL_COLOR_MATERIAL);
  } else {
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // call before enabling GL_COLOR_MATERIAL
  }

  glEnable(GL_COLOR_MATERIAL);
  glPopMatrix();
}


void GLCanvas::TakeAction() {
  //printf ("TAKE ACTION\n");
  //  command_queue->Print();
  assert(HandleGLError("takeaction1"));
  assert (!command_queue->Empty());
  Command *c = command_queue->Next();
  //c->Print();
  assert (c->getTimer() > 0);
  c->decrTimer();
  assert(HandleGLError("takeaction2"));
  //c->Print()
  switch (c->getType()) {
  case COMMAND_QUIT: {
    DoOnQuit(meshes);
    exit(0);
    break; }
  case COMMAND_LOAD: {
    meshes->Clear();
    GLOBAL_DESIRED_COUNT = -1;

    std::cout << "RESETTING ELEMENT_ID!" << std::endl;
    Element::currentID = 1;

    MeshIO::Load(meshes, meshes->args->load_file);
    if (main_loop_started) rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_CUT_THROUGH_PLANES: {
    ReMesh::CutThroughPlanes(meshes);
    updateCounts();
    rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_CHEAT_FAKE_MATERIALS: {
    ReMesh::CheatFakeMaterials(meshes);
    rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_BAD_TRIANGLES_STATUS: {
    ReMesh::BadTrianglesStatus(meshes);
    break; }
  case COMMAND_ELIMINATE_BAD_TRIANGLES: {
    while(ReMesh::EliminateBadTriangles(meshes)) {
      rerender_select_geometryCB(0);
    }
    rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_SUBDIVIDE: {
    ReMesh::LoopSubdivision(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_SPLIT_EDGES: {
    ReMesh::SplitEdges(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_FLIP_EDGES: {
    ReMesh::FlipEdges(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_MOVE_VERTICES: {
    ReMesh::MoveVertices(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_FIX_SEAMS: {
    ReMesh::FixSeams(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_TRIANGULATE: {
    ReMesh::Triangulate(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_MOVE_VERTICES_RANDOMLY: {
    ReMesh::MoveVerticesRandomly(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_COMPRESS_VERTICES: {
    ReMesh::CompressVertices(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }

  case COMMAND_SEED_PATCHES: {
    meshes->getMesh()->SeedPatches(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_ITERATE_PATCHES: {
    meshes->getMesh()->IteratePatches(meshes,1); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_RANDOMIZE_PATCH_COLORS: {
    meshes->getMesh()->RandomizePatchColors(meshes);
    rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_ASSIGN_ZONES: {
    meshes->getMesh()->AssignZones(meshes);
    rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break;}
  case COMMAND_RANDOMIZE_ZONE_COLORS: {
    meshes->getMesh()->RandomizeZoneColors(meshes);
    rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break;}

  case COMMAND_CUT_EDGES: {
    ReMesh::CutEdges(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_COLLAPSE_EDGES: {
    ReMesh::CollapseEdges(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }
  case COMMAND_SAVE_REMESH: {
    MeshIO::Save(meshes,meshes->getMesh(), meshes->args->save_file);
    assert (c->getTimer() == 0);
    break; }

  case COMMAND_EVALUATE: {
    ReMesh::Evaluate(meshes); rerender_select_geometryCB(0);
    assert (c->getTimer() == 0);
    break; }

  default: {
    printf ("WHOOPS UNKNOWN COMMAND CANT TAKE ACTION\n");
    assert(0);
    break; }
  }
  if (c->getTimer() == 0) {
    command_queue->Remove();
  }
  assert(HandleGLError("takeaction3"));
  updateCounts();
  c = NULL;
}

void GLCanvas::display() {

  OffScreenBuffer osb(meshes->args->width2,meshes->args->height2,false); //true); //false);
  if (meshes->args->make_arrangement_images || GLOBAL_SNAP) {
    osb.Select();
  }

  // FIX?
  assert (glutGetWindow() == main_window);
  //  if (glutGetWindow() != main_window) glutSetWindow(main_window);
  assert(HandleGLError("begin_display"));
  // Clear the display buffer
  if (meshes->args->white_background == 0) { glClearColor(0,0,0,1); }
  else { glClearColor(1,1,1,1); }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  InitLight();

  // Set the camera parameters
  if (meshes->args->glcam_camera == NULL || !meshes->args->use_glcam_camera) {
    glMatrixMode(GL_PROJECTION);
    camera->glInit(meshes->args->width2,meshes->args->height2);
    camera->glPlaceCamera();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotated(-90.0,1,0,0);
    glScaled(1.02,1.02,1.02);
  } else {
    // render from glcam camera calibration
    meshes->args->glcam_camera->setOpenGLCamera();
  }

  // draw objects
#if 1
  BoundingBox *bbox = meshes->getMesh()->getBoundingBox();
  Vec3f center = Vec3f(0,0,0);
  double scale = 0.8;
  if (bbox != NULL) {
    bbox->getCenter(center);
    scale *= 1 / bbox->maxDim();
  }
  glScalef(scale,scale,scale);
  glTranslatef(-center.x(),-center.y(),-center.z());
#endif

  glEnable(GL_LIGHTING);
  if (meshes->args->clip_enabled) {
    BoundingBox *bbox = meshes->getMesh()->getBoundingBox();
    Vec3f min, max;
    bbox->Get(min,max);
    float min_v = min4(-1,min.x(),min.y(),min.z());
    float max_v = max4(1,max.x(),max.y(),max.z());
    GLdouble eqn0[4] = { meshes->args->clip_x,
                         meshes->args->clip_y,
                         meshes->args->clip_z,
                         meshes->args->min_clip*min_v };
    GLdouble eqn1[4] = { -meshes->args->clip_x,
                         -meshes->args->clip_y,
                         -meshes->args->clip_z,
                         meshes->args->max_clip*max_v };
    glClipPlane(GL_CLIP_PLANE0,eqn0);
    glClipPlane(GL_CLIP_PLANE1,eqn1);
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
  } else {
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
  }

  Render::RenderAll(meshes);

  if (meshes->args->make_arrangement_images || GLOBAL_SNAP) {
    assert (meshes->args->camera_image_file != "");
    std::cout << "WRITING CAMERA IMAGE " << meshes->args->camera_image_file << std::endl;
    osb.WritePPM(meshes->args->camera_image_file.c_str());
    osb.UnSelect();
  } else {
    // Swap the back buffer with the front buffer to display the scene
    glutSwapBuffers();
    if (!command_queue->Empty()) {
      TakeAction();
    }
  }
  assert(HandleGLError("end_display"));
}


void GLCanvas::refresh() {
  glui->refresh();
  glui->sync_live();

}


// ========================================================
// Callback function for window resize
// ========================================================

void GLCanvas::reshape(int w, int h) {
  // Set the OpenGL viewport to fill the entire window
  int tx,ty,tw,th;
  GLUI_Master.get_viewport_area(&tx,&ty,&tw,&th);
  glViewport(tx,ty,tw,th);

  meshes->args->width2 = tw-tx;
  meshes->args->height2 = th-ty;
  //std::cout << "va " << tx << " " << ty << " " << tw << " " << th << std::endl;
  //meshes->args->height = th-ty;
  //meshes->args->width = tw-tx;

  //std::cout << "  w " << meshes->args->width << "h " << meshes->args->height << std::endl;

  // Set the camera parameters to reflect the changes
  glMatrixMode(GL_PROJECTION);
  camera->glInit(meshes->args->width2,meshes->args->height2);
  camera->glPlaceCamera();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotated(-90.0,1,0,0);
}


// ========================================================
// Callback function for mouse click or release
// ========================================================

void GLCanvas::mouse(int button, int state, int x, int y) {
  // state 0 = press, 1 = release
  // Save the current state of the mouse.  This will be
  // used by the 'motion' function

  if (state == 0) {
    setKeyOnPress();
    //printf ("MOUSE PRESS\n");
  }

  char c = whichKeyOnMousePress();
  //printf ("MOUSE KEY %c\n", c);

  switch(c) {

  case '1': case '2':
  case '3': case '4':
  case '5': case '6':
  case '7': case '8':
  case '9':
    char tmp[2]; tmp[0] = c; tmp[1] = '\0';
    //    PaintDensity(atoi(tmp),x,y);
    mouseButton = button;
    mouseX = x;
    mouseY = y;
    break;
  default:
    mouseButton = button;
    mouseX = x;
    mouseY = y;
  }

  if (state == 1) {
    //printf (" MOUSE RELEASE'%c'\n",c);
    clearKeyOnRelease();
  }

  shiftPressed = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
  controlPressed = (glutGetModifiers() & GLUT_ACTIVE_CTRL) != 0;
  altPressed = (glutGetModifiers() & GLUT_ACTIVE_ALT) != 0;
  //  controlPressed = glutGetModifiers() & GLUT_ACTIVE_CTRL;
}

// ========================================================
// Callback function for mouse drag
// ========================================================

void GLCanvas::motion(int x, int y) {

  char c = whichKeyOnMousePress();
  //printf ("m%c",c);fflush(stdout);

  switch(c) {

  case '1': case '2':
  case '3': case '4':
  case '5': case '6':
  case '7': case '8':
  case '9':
    char tmp[2]; tmp[0] = c; tmp[1] = '\0';
    //    PaintDensity(atoi(tmp),x,y);
    break;
  default:



    // Control or Shift or Alt pressed = zoom
    // (don't move the camera, just change the angle or image size)
    if (controlPressed || shiftPressed || altPressed) {
      camera->zoomCamera(mouseY-y);
    }
    // Left button = rotation
    // (rotate camera around the up and horizontal vectors)
    else if (mouseButton == GLUT_LEFT_BUTTON) {
      camera->rotateCamera(0.005*(mouseX-x), 0.005*(mouseY-y));
    }
    // Middle button = translation
    // (move camera perpendicular to the direction vector)
    else if (mouseButton == GLUT_MIDDLE_BUTTON) {
      camera->truckCamera(mouseX-x, y-mouseY);
    }
    // Right button = dolly or zoom
    // (move camera along the direction vector)
    else if (mouseButton == GLUT_RIGHT_BUTTON) {
      camera->dollyCamera(mouseY-y);
    }
    mouseX = x;
    mouseY = y;


    /*
    // Left button = rotation
    // (rotate camera around the up and horizontal vectors)
    if (mouseButton == GLUT_LEFT_BUTTON) {
    camera->rotateCamera(0.005*(mouseX-x), 0.005*(mouseY-y));
    mouseX = x;
    mouseY = y;
    }
    // Middle button = translation
    // (move camera perpendicular to the direction vector)
    else if (mouseButton == GLUT_MIDDLE_BUTTON) {
    camera->truckCamera(mouseX-x, y-mouseY);
    mouseX = x;
    mouseY = y;
    }
    // Right button = dolly or zoom
    // (move camera along the direction vector)
    else if (mouseButton == GLUT_RIGHT_BUTTON) {
    if (controlPressed || shiftPressed || altPressed) {
    camera->zoomCamera(mouseY-y);
    } else {
    camera->dollyCamera(mouseY-y);
    }
    mouseX = x;
    mouseY = y;
    }
    */
  }

  // Redraw the scene with the new camera parameters
  glutPostWindowRedisplay(main_window);
}

// ========================================================
// Callback function for keyboard events
// ========================================================

void GLCanvas::keyboard_down(unsigned char key, int x, int y) {

  //printf ("KEY DOWN %c\n",key);
  switch (key) {
  case 'p':  case 'P':
    GLOBAL_SNAP=true;
    display();
    GLOBAL_SNAP=false;
    break;
  case 'q':  case 'Q':
    exit(0);
    break;
  case 'w':  case 'W':
    meshes->args->glui = !meshes->args->glui;
    if (!meshes->args->glui)
      glui->hide();
    else
      glui->show();
    break;
  case 'c':  case 'C':
    meshes->args->use_glcam_camera = ! meshes->args->use_glcam_camera;
    meshes->args->rerender_scene_geometry = 1;
    break;
  case '1': case '2':
  case '3': case '4':
  case '5': case '6':
  case '7': case '8':
  case '9':
    setKey(key);
    break;
  default:
    printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
  }
}

// ========================================================
// ========================================================


// ========================================================
// ========================================================

void GLCanvas::idle() {
  static int here = 0;

  if (meshes->args->rerender_scene_geometry) {
    glutPostWindowRedisplay(main_window);
  }
  assert(HandleGLError("idle"));

  if (meshes->args->offline_viewer && here == 0) {
    here = 1;
    std::cout << "offline viewer" << std::endl;
    remeshCB(0);
    saveRemeshCB(0);
    quitCB(0);
    meshes->args->offline = 0;
  }
  if (!command_queue->Empty()) {
    TakeAction();
  } else {
#ifdef _WIN32
    Sleep(1);
#else
    usleep (1000);
#endif
  }

  static clock_t start_time = clock();
  static int frames = 0;

  //static int last_time = time(0);
  //int this_time = time(0);
  //std::cout << "times " << last_time << " " << this_time << " " << CLOCKS_PER_SEC << std::endl;

  //  double difference = double(this_time - last_time)/CLOCKS_PER_SEC;
  //std::cout << "difference " << difference << std::endl;
  if (meshes->args->run_continuously) {
#ifdef _WIN32
    Sleep(10);
#else
    usleep(10000);
#endif
    //&& last_time != this_time) { //true && (difference > 1 || difference < 0)) {
    //std::cout << " think about a reload" << std::endl;
    frames++;
    clock_t end_time = clock();
    double total_time = (end_time - start_time) / double(CLOCKS_PER_SEC);
    double fps = double(frames) / total_time;
    std::cout << "FPS = " << fps << std::endl;

    bool incr_success = true;
    if (meshes->args->increment_filename) {
      incr_success = IncrementFileName(meshes->args->load_file);
    }
    /*
      if (tmp.substr(tmp.size()-5,5) == ".wall") {
      string number = tmp.substr(tmp.size()-11,6);
      //std::cout << "TEST" << number << std::endl;
      int num = atoi(number.c_str());
      //std::cout << "NUM " << num << std::endl;
      num++;
      stringstream wall_filename;
      wall_filename << tmp.substr(0,tmp.size()-11) << setw(6) << setfill('0') << num << ".wall";
      tmp  = wall_filename.str();
      } else {
      assert (tmp.substr(tmp.size()-4,4) == ".led");
      string number = tmp.substr(tmp.size()-10,6);
      //std::cout << "TEST" << number << std::endl;
      int num = atoi(number.c_str());
      //std::cout << "NUM " << num << std::endl;
      num++;
      stringstream wall_filename;
      wall_filename << tmp.substr(0,tmp.size()-10) << setw(6) << setfill('0') << num << ".led";
      tmp  = wall_filename.str();
      }
    */

    //std::cout << "NEW " << tmp << std::endl;
    //    meshes->args->load_file = "tmp";
    //MeshIO::Load(meshes, meshes->args->load_file);
    //exit(0);
    //ifstream test(meshes->args->load_file.c_str());
    if (incr_success) {
      loadCB(0);
      ReMesh::CutThroughPlanes(meshes);
    } else {
      meshes->args->run_continuously = false;
    }
    //last_time = this_time;
  }

}

// ========================================================
// Initialize all appropriate OpenGL variables, set
// callback functions, and start the main event loop.
// This function will not return but can be terminated
// by calling 'exit(0)'
// ========================================================

void GLCanvas::clear() {
  meshes = NULL; //someone else will delete this
  delete camera;
  camera = NULL;
  delete command_queue;
  command_queue = NULL;

  // I don't know that this does anything...
  delete glui;
  glui = NULL;

}

void GLCanvas::initialize(MeshManager *_meshes) {
  meshes = NULL;
  meshes = _meshes;

  command_queue = new CommandQueue();

  // Set window parameters
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  glutInitWindowSize(meshes->args->width2,meshes->args->height2);
  glutInitWindowPosition(meshes->args->pos_x,meshes->args->pos_y);
  main_window = glutCreateWindow("Remesh Viewer");
  assert(HandleGLError("initialize_start"));

  // Initialize callback functions
  glutMotionFunc(motion);
  glutDisplayFunc(display);
  setup_glui(main_window);
  if (!meshes->args->glui)
    glui->hide();

  // Initialize glui callback functions
  GLUI_Master.set_glutMouseFunc(mouse);
  GLUI_Master.set_glutReshapeFunc(reshape);
  GLUI_Master.set_glutKeyboardFunc(keyboard_down);
  GLUI_Master.set_glutIdleFunc(idle);

  glEnable(GL_LIGHTING);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  // Enter the main rendering loop
  main_loop_started = 1;
  //loadCB(0);
  //TakeAction();
  resetCameraCB(0);
  glutPostWindowRedisplay(main_window);

  assert(HandleGLError("initialize_done"));

  //  glutMainLoop();
}

// ========================================================
// ========================================================

void GLCanvas::quitCB(int) { command_queue->Add(COMMAND_QUIT); }

void GLCanvas::updateCounts() {
  //  std::cout << "updateCounts()" << std::endl;
  int vertices = meshes->getMesh()->numVertices();
  int triangles = meshes->getMesh()->numTriangles();
  int patches = meshes->getMesh()->numPatches();
  int quads = meshes->getMesh()->numQuads();
  int bad_elements = meshes->getMesh()->numBadElements();
  setCounts(vertices,triangles,quads,bad_elements,patches);
  redisplayCB(0);
}

void GLCanvas::redisplayCB(int) {
  //  updateCounts();
  if (main_loop_started)
    glutPostWindowRedisplay(main_window);
}

void GLCanvas::rerender_sceneCB(int) {
  updateCounts();
  meshes->args->rerender_scene_geometry = 1;
}

void GLCanvas::rerender_select_geometryCB(int) {
  updateCounts();
  meshes->args->rerender_scene_geometry = 1;
  meshes->args->rerender_select_geometry = 1;
}

void GLCanvas::loadCB(int) {
  command_queue->Add(COMMAND_LOAD);
}

void GLCanvas::cutThroughPlanesCB(int) {
  command_queue->Add(COMMAND_CUT_THROUGH_PLANES);
}

void GLCanvas::cheatFakeMaterialsCB(int) {
  command_queue->Add(COMMAND_CHEAT_FAKE_MATERIALS);
}

void GLCanvas::saveRemeshCB(int) { command_queue->Add(COMMAND_SAVE_REMESH); }

void GLCanvas::resetCameraCB(int) {
  delete camera;

  double x,y,z;
  double angle;

  if (meshes->args->make_arrangement_images || meshes->args->render_mode == 2) {
    x = 0;
    y = 0.001;
    z = -1.7;
    angle = 51;
  } else {
    angle = 90.0;
#if 0
    x = 0; //-0.5;
    y = 0.001; //1.5;//0; //1.5; //0.9;
    z = -1.7; //-4.0; //1.8;
    if (meshes->getWalls()->empac) {
      x *=12*2;
      y *=12*2;
      z *=12*2;
    }
#else
    x = 0;
    y = 1.5;
    z = -1.0;
#endif

  }
  /*
    int count = meshes->args->projectors.size();
    for (int i = 0; i < count; i++) {
    double x,y,z;
    meshes->args->projectors[i].getCenter(x,y,z);
    }
  */

  Vec3f camera_position = Vec3f(x,y,z);
  Vec3f point_of_interest = Vec3f(0,0,0);
  Vec3f up = Vec3f(0,0,-1);


  camera = new PerspectiveCamera(camera_position, point_of_interest, up, angle * M_PI/180.0);
  //camera = new PerspectiveCamera(camera_position, point_of_interest, up, 70 * M_PI/180.0);
  if (main_loop_started)
    glutPostWindowRedisplay(main_window);
}


// REMESHING
void GLCanvas::remeshCB(int) {
  command_queue->Add(COMMAND_ELIMINATE_BAD_TRIANGLES);

  /*
    command_queue->Add(COMMAND_EVALUATE);
    command_queue->Add(COMMAND_SPLIT_EDGES);
    command_queue->Add(COMMAND_FLIP_EDGES);
    command_queue->Add(COMMAND_COLLAPSE_EDGES);
    command_queue->Add(COMMAND_MOVE_VERTICES);
  */

  for (int i = 0; i < 10; i++) {
    command_queue->Add(COMMAND_EVALUATE);
    command_queue->Add(COMMAND_SPLIT_EDGES);
    command_queue->Add(COMMAND_FLIP_EDGES);
    command_queue->Add(COMMAND_MOVE_VERTICES);
    command_queue->Add(COMMAND_COLLAPSE_EDGES);
  }

  /*
    command_queue->Add(COMMAND_MOVE_VERTICES);

    command_queue->Add(COMMAND_EVALUATE);
    command_queue->Add(COMMAND_SPLIT_EDGES);
    command_queue->Add(COMMAND_FLIP_EDGES);
    command_queue->Add(COMMAND_COLLAPSE_EDGES);

    command_queue->Add(COMMAND_MOVE_VERTICES);
  */
}

void GLCanvas::BadTrianglesStatusCB(int) {
  command_queue->Add(COMMAND_BAD_TRIANGLES_STATUS);
}

void GLCanvas::eliminateBadTrianglesCB(int) { command_queue->Add(COMMAND_EVALUATE); command_queue->Add(COMMAND_ELIMINATE_BAD_TRIANGLES); }
void GLCanvas::subdivideCB(int) { command_queue->Add(COMMAND_EVALUATE); command_queue->Add(COMMAND_SUBDIVIDE); }
void GLCanvas::splitEdgesCB(int) { command_queue->Add(COMMAND_EVALUATE); command_queue->Add(COMMAND_SPLIT_EDGES); }
void GLCanvas::flipEdgesCB(int) { command_queue->Add(COMMAND_EVALUATE); command_queue->Add(COMMAND_FLIP_EDGES); }
void GLCanvas::moveVerticesCB(int) { command_queue->Add(COMMAND_EVALUATE); command_queue->Add(COMMAND_MOVE_VERTICES); }
void GLCanvas::collapseEdgesCB(int) { command_queue->Add(COMMAND_EVALUATE); command_queue->Add(COMMAND_COLLAPSE_EDGES); }

void GLCanvas::fixSeamsCB(int) { command_queue->Add(COMMAND_FIX_SEAMS); }
void GLCanvas::triangulateCB(int) { command_queue->Add(COMMAND_TRIANGULATE); }
void GLCanvas::moveVerticesRandomlyCB(int) { command_queue->Add(COMMAND_MOVE_VERTICES_RANDOMLY); }
void GLCanvas::compressVerticesCB(int) { command_queue->Add(COMMAND_COMPRESS_VERTICES); }


void GLCanvas::assignPatchesAndZonesCB(int) {
  command_queue->Add(COMMAND_SEED_PATCHES);
  command_queue->Add(COMMAND_RANDOMIZE_PATCH_COLORS);
  for (int i = 0; i < 20; i++) {
    command_queue->Add(COMMAND_ITERATE_PATCHES);
  }
  command_queue->Add(COMMAND_ASSIGN_ZONES);
  command_queue->Add(COMMAND_RANDOMIZE_ZONE_COLORS);
}
void GLCanvas::seedPatchesCB(int) {
  command_queue->Add(COMMAND_SEED_PATCHES);
  command_queue->Add(COMMAND_RANDOMIZE_PATCH_COLORS); }
void GLCanvas::randomizePatchAndZoneColorsCB(int) {
  command_queue->Add(COMMAND_RANDOMIZE_PATCH_COLORS);
  command_queue->Add(COMMAND_RANDOMIZE_ZONE_COLORS); }
void GLCanvas::iteratePatchesCB(int) { command_queue->Add(COMMAND_ITERATE_PATCHES); }
void GLCanvas::assignZonesCB(int) { command_queue->Add(COMMAND_ASSIGN_ZONES); }


void GLCanvas::cutEdgesCB(int) { command_queue->Add(COMMAND_CUT_EDGES); }
void GLCanvas::setCounts(int vertices, int triangles, int quads, int bad_elements, int patches) {
  char tmp[100];
  sprintf (tmp, "%d vertices", vertices);
  vertices_text->set_text(tmp);
  sprintf (tmp, "%d triangles", triangles);
  triangles_text->set_text(tmp);
  sprintf (tmp, "%d quads", quads);
  quads_text->set_text(tmp);
  sprintf (tmp, "%d bad elements", bad_elements);
  bad_elements_text->set_text(tmp);
  sprintf (tmp, "%d patches", patches);
  patches_text->set_text(tmp);
}

void GLCanvas::ClipCB(int) {

  static float prev_min = -1;

  float tmp = square(meshes->args->clip_x) + square(meshes->args->clip_y);
  if (tmp > 1) {
    tmp = sqrt(1-square(meshes->args->clip_x));
    if (meshes->args->clip_y >= 0)
      meshes->args->clip_y = tmp;
    else
      meshes->args->clip_y = -tmp;
    meshes->args->clip_z = 0;
  } else {
    tmp = sqrt(1-tmp);
    if (meshes->args->clip_z >= 0)
      meshes->args->clip_z = tmp;
    else
      meshes->args->clip_z = -tmp;
  }
  tmp = square(meshes->args->clip_x) + square(meshes->args->clip_y) + square(meshes->args->clip_z);
  assert (fabs(tmp - 1) < 0.0001);

  if (meshes->args->min_clip > meshes->args->max_clip-0.001) {
    if (prev_min-meshes->args->min_clip > 0.0001) {
      meshes->args->max_clip = meshes->args->min_clip+0.001;
    } else {
      meshes->args->min_clip = meshes->args->max_clip-0.001;
    }
  }
  assert (meshes->args->min_clip < meshes->args->max_clip);

  prev_min = meshes->args->min_clip;

  glui->refresh();
  glui->sync_live();

  redisplayCB(0);

}



GLUI_Panel* GLCanvas::setup_rollout_helper(char *label, bool visible) {
  GLUI_Panel *answer = glui->add_rollout(label,visible);
  answer->set_alignment(GLUI_ALIGN_LEFT);
  answer->set_w(400);
  return answer;
}

void GLCanvas::setup_button_helper(GLUI_Panel *p, char *button, GLUI_Update_CB CB) {
  GLUI_Button *b = glui->add_button_to_panel(p,button,0,CB);
  b->set_w(100);
  b->set_alignment(GLUI_ALIGN_LEFT);
}

void GLCanvas::setup_spinner_helper_float(GLUI_Panel *p, char *label, float min_val, float max_val, float *val, GLUI_Update_CB CB) {
  GLUI_Spinner *s =
    glui->add_spinner_to_panel(p,label,GLUI_SPINNER_FLOAT, val, -1, CB);
  s->set_float_limits(min_val,max_val,GLUI_LIMIT_CLAMP);
  s->set_speed(SPEED_FACTOR);
}

void GLCanvas::setup_spinner_helper_int(GLUI_Panel *p, char *label, int min_val, int max_val, int *val, GLUI_Update_CB CB) {
  GLUI_Spinner *s =
    glui->add_spinner_to_panel(p,label,GLUI_SPINNER_INT, val, -1, CB);
  s->set_int_limits(min_val,max_val,GLUI_LIMIT_CLAMP);
  s->set_speed(SPEED_FACTOR);
}

void GLCanvas::setup_file_helper(GLUI_Panel *p, char *button, char *label, GLUI_String &var, GLUI_Update_CB CB) {
  GLUI_Panel *p2 = glui->add_panel_to_panel(p,(char*)"",false);
  setup_button_helper(p2,button,CB);
  glui->add_column_to_panel(p2,false);
  //GLUI_EditText *edittext = glui->add_edittext_to_panel(p2,label,GLUI_EDITTEXT_TEXT, var,0,CB);

  std::cerr << "ERROR: CANNOT SETUP FILE GUI FOR: " << label << std::endl;

  //GLUI_EditText *edittext =   glui->add_edittext_to_panel(p2,label,var);//,0,CB);
  //  edittext->set_w(250);
}

// PARAMETERS & USER INTERFACE
void GLCanvas::setup_glui(int main_window) {

  glui = GLUI_Master.create_glui((char*)"Remesher Controls", 0,0,0);
  glui->set_main_gfx_window(main_window);

  // ---------------------------------------------------------
  // INPUT & CAMERA
  GLUI_Panel *load_panel = setup_rollout_helper((char*)"Input",true);
  setup_file_helper(load_panel,(char*)"LOAD",(char*)"input",meshes->args->load_file, loadCB);
  setup_button_helper(load_panel,(char*)"CUT PLANES", cutThroughPlanesCB);
  setup_button_helper(load_panel,(char*)"Cheat fake materials", cheatFakeMaterialsCB);

  //glui->add_checkbox_to_panel(load_panel,(char*)"flip triangles while loading",&meshes->args->load_flip_triangles,0,NULL);
  glui->add_checkbox_to_panel(load_panel,(char*)"flip triangles while loading",&meshes->args->load_flip_triangles); //,0,NULL);
  glui->add_checkbox_to_panel(load_panel,(char*)"flip y up while loading",&meshes->args->load_flip_y_up); //,0,NULL);

  //setup_spinner_helper_float(load_panel,(char*)"enclosed threshhold",0.0,1.0,&meshes->args->enclosed_threshhold);

  GLUI_Panel *load_panel2 = glui->add_panel_to_panel(load_panel,(char*)"",false);
  setup_button_helper(load_panel2,(char*)"Reset Camera",resetCameraCB);
  glui->add_column_to_panel(load_panel2,false);
  setup_button_helper(load_panel2,(char*)"Quit",quitCB);

  // ---------------------------------------------------------
  // REMESHING
  GLUI_Panel *remeshing_panel = setup_rollout_helper((char*)"Remeshing",true);
  GLUI_Panel *remeshing_panel2 = glui->add_panel_to_panel(remeshing_panel,(char*)"",false);
  setup_button_helper(remeshing_panel2,(char*)"REMESH",remeshCB);

  setup_button_helper(remeshing_panel2,(char*)"bad status",BadTrianglesStatusCB);

  setup_spinner_helper_int(remeshing_panel2,(char*)"target triangles",1,1000000,&meshes->args->desired_tri_count);
  setup_spinner_helper_int(remeshing_panel2,(char*)"target patches",1,1000000,&meshes->args->desired_patch_count);
  setup_spinner_helper_float(remeshing_panel2,(char*)"remesh_normal_tolerance",0.0,1.0,&meshes->args->remesh_normal_tolerance);
  //setup_spinner_helper_float(remeshing_panel2,(char*)"cut_normal_tolerance",0.0,1.0,&meshes->args->cut_normal_tolerance);
  glui->add_checkbox_to_panel(remeshing_panel2,(char*)"~preserve volume",&meshes->args->preserve_volume);
  glui->add_checkbox_to_panel(remeshing_panel2,(char*)"~equal edges & area",&meshes->args->equal_edge_and_area);

  vertices_text = glui->add_statictext_to_panel(remeshing_panel2,(char*)"0 vertices");
  triangles_text = glui->add_statictext_to_panel(remeshing_panel2,(char*)"0 triangles");
  quads_text = glui->add_statictext_to_panel(remeshing_panel2,(char*)"0 quads");
  bad_elements_text = glui->add_statictext_to_panel(remeshing_panel2,(char*)"0 bad elements");
  patches_text = glui->add_statictext_to_panel(remeshing_panel2,(char*)"0 patches");

  glui->add_column_to_panel(remeshing_panel2,false);
  setup_button_helper(remeshing_panel2,(char*)"eliminate bad",eliminateBadTrianglesCB);
  setup_button_helper(remeshing_panel2,(char*)"subdivide",subdivideCB);
  setup_button_helper(remeshing_panel2,(char*)"split edges",splitEdgesCB);
  setup_button_helper(remeshing_panel2,(char*)"collapse edges",collapseEdgesCB);
  setup_button_helper(remeshing_panel2,(char*)"flip edges",flipEdgesCB);
  setup_button_helper(remeshing_panel2,(char*)"move vertices",moveVerticesCB);
  setup_button_helper(remeshing_panel2,(char*)"fix seams",fixSeamsCB);
  setup_button_helper(remeshing_panel2,(char*)"triangulate",triangulateCB);
  setup_button_helper(remeshing_panel2,(char*)"move vertices randomly",moveVerticesRandomlyCB);
  setup_button_helper(remeshing_panel2,(char*)"compress vertex list",compressVerticesCB);
  glui->add_column_to_panel(remeshing_panel2,false);
  setup_button_helper(remeshing_panel2,(char*)"seed patches",seedPatchesCB);
  setup_button_helper(remeshing_panel2,(char*)"iterate patches",iteratePatchesCB);
  setup_button_helper(remeshing_panel2,(char*)"assign patches",assignPatchesAndZonesCB);
  setup_button_helper(remeshing_panel2,(char*)"assign zones",assignZonesCB);
  setup_button_helper(remeshing_panel2,(char*)"randomize patch & zone colors",randomizePatchAndZoneColorsCB);
  //setup_button_helper(remeshing_panel2,(char*)"cut edges",cutEdgesCB);

  //  glui->add_checkbox_to_panel(remeshing_panel,(char*)"save as quads",&meshes->args->save_as_quads);
  //  glui->add_checkbox_to_panel(remeshing_panel,(char*)"save polarized",&meshes->args->save_polarized);

  setup_file_helper(remeshing_panel,(char*)"SAVE",(char*)"remesh",meshes->args->save_file, saveRemeshCB);



  // ---------------------------------------------------------
  // VISUALIZATION OPTIONS
  GLUI_Panel *visualization_panel = setup_rollout_helper((char*)"Visualization Options",true);

  GLUI_RadioGroup *render_mode = glui->add_radiogroup_to_panel
    (visualization_panel,&meshes->args->render_mode,1,rerender_sceneCB);
  glui->add_radiobutton_to_group(render_mode,(char*)"no fill triangles");  //0
  glui->add_radiobutton_to_group(render_mode,(char*)"triangles");  //1
  glui->add_radiobutton_to_group(render_mode,(char*)"arrangement % enclosed"); //2
  glui->add_radiobutton_to_group(render_mode,(char*)"arrangement type"); //3
  glui->add_radiobutton_to_group(render_mode,(char*)"arrangement fingerprints"); //4
  glui->add_radiobutton_to_group(render_mode,(char*)"arrangement num middles"); //5
  glui->add_radiobutton_to_group(render_mode,(char*)"floor plan"); //6
  glui->add_radiobutton_to_group(render_mode,(char*)"2D walls"); //7
  glui->add_radiobutton_to_group(render_mode,(char*)"3D model"); //8

  glui->add_checkbox_to_panel(visualization_panel,(char*)"point sampled enclosure",&meshes->args->point_sampled_enclosure,0,rerender_sceneCB);

  //  glui->add_separator_to_panel(visualization_panel);

  //glui->add_checkbox_to_panel(visualization_panel,(char*)"Interpolated Normals",&meshes->args->render_normals,0,rerender_sceneCB);
  //glui->add_checkbox_to_panel(visualization_panel,(char*)"Cracks",&meshes->args->render_cracks,0,rerender_sceneCB);
  //glui->add_checkbox_to_panel(visualization_panel,(char*)"Transparency",&meshes->args->transparency,0,rerender_sceneCB);


#if 0
  setup_spinner_helper_float(visualization_panel,(char*)"clip x:",-1,1,&meshes->args->clip_x, ClipCB);
  setup_spinner_helper_float(visualization_panel,(char*)"clip y:",-1,1,&meshes->args->clip_y, ClipCB);
  setup_spinner_helper_float(visualization_panel,(char*)"clip z:",-1,1,&meshes->args->clip_z, ClipCB);

  setup_spinner_helper_float(visualization_panel,(char*)"min clip:",-1.1,1.0,&meshes->args->min_clip, ClipCB);
  setup_spinner_helper_float(visualization_panel,(char*)"max clip:",-1.0,1.1,&meshes->args->max_clip, ClipCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Enable Clipping",&meshes->args->clip_enabled,0,redisplayCB);
#endif

  glui->add_column_to_panel(visualization_panel,false);

  GLUI_RadioGroup *render_vis_mode = glui->add_radiogroup_to_panel
    (visualization_panel,&meshes->args->render_vis_mode,1,rerender_sceneCB);
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"White (front) / Magenta (back)"); // 0
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Patches");                        // 1
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Zones");                          // 2
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Diffuse Material");               // 3
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Physical Material");              // 4
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Material by Index");              // 5
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Glass (blue)");                   // 6
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Invisible Sensors (cyan)");       // 7
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Opaque Sensors (orange)");        // 8
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Exterior Shading (yellow-green)");// 9
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Texture Coordinates");            // 10
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Normals");                        // 11
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Projector Visibility");           // 12
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Blending Weights Sum");           // 13
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Blending Weights");               // 14
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Blending Distance");              // 15
  glui->add_radiobutton_to_group(render_vis_mode,(char*)"Blending Weights w/ Distance");   // 16
  //  glui->add_separator_to_panel(visualization_panel);

  if (meshes->args->projector_names.size() > 0)
    glui->add_column_to_panel(visualization_panel,false);

  GLUI_RadioGroup *render_which_projector = glui->add_radiogroup_to_panel
    (visualization_panel,&meshes->args->render_which_projector,1,rerender_sceneCB);
  for (unsigned int p = 0; p < meshes->args->projector_names.size(); p++) {
    std::string tmp = meshes->args->projector_names[p];
    glui->add_radiobutton_to_group(render_which_projector,tmp.c_str());
  }

  glui->add_column_to_panel(visualization_panel,false);

  glui->add_checkbox_to_panel(visualization_panel,(char*)"Triangle Vertices",&meshes->args->render_triangle_vertices,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Triangle Edges",&meshes->args->render_triangle_edges,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Boundaries (red)",&meshes->args->render_cluster_boundaries,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Non-Manifold (yellow)",&meshes->args->render_non_manifold_edges,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Creases (magenta)",&meshes->args->render_crease_edges,0,rerender_sceneCB);

  glui->add_checkbox_to_panel(visualization_panel,(char*)"Zero Area Triangles (green)",&meshes->args->render_zero_area_triangles,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Bad Normal Triangles (cyan)",&meshes->args->render_bad_normal_triangles,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Bad Neighbor Triangles (blue)",&meshes->args->render_bad_neighbor_triangles,0,rerender_sceneCB);

  glui->add_checkbox_to_panel(visualization_panel,(char*)"Cull Backfacing Triangles",&meshes->args->render_cull_back_facing,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Cull Ceiling Triangles",&meshes->args->render_cull_ceiling,0,rerender_sceneCB);

  glui->add_checkbox_to_panel(visualization_panel,(char*)"Visibility Planes",&meshes->args->render_visibility_planes,0,rerender_sceneCB);

  glui->add_checkbox_to_panel(visualization_panel,(char*)"Draw Walls",&meshes->args->render_walls,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Draw Wall Chains",&meshes->args->render_wall_chains,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"Draw Ground Plane",&meshes->args->ground_plane,0,rerender_sceneCB);

  glui->add_separator_to_panel(visualization_panel);

  glui->add_checkbox_to_panel(visualization_panel,(char*)"PROJECTION elements only",&meshes->args->render_PROJECTION_elements,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"FILLIN elements only",&meshes->args->render_FILLIN_elements,0,rerender_sceneCB);
  glui->add_checkbox_to_panel(visualization_panel,(char*)"EXTRA elements only",&meshes->args->render_EXTRA_elements,0,rerender_sceneCB);

  // ---------------------------------------------------------
  assert(HandleGLError("setup glui"));


}

// ========================================================
// ========================================================


//#include <sys/time.h>

char LAST_KEY = '\0';
char KEY_ON_CLICK = '\0';
timeval LAST_TIME;


char GLCanvas::currentKey() {
  /*timeval tv ;
    gettimeofday(&tv,NULL);
    timeval diff;
    if (LAST_TIME.tv_usec <= tv.tv_usec) {
    diff.tv_usec = tv.tv_usec - LAST_TIME.tv_usec;
    diff.tv_sec  = tv.tv_sec  - LAST_TIME.tv_sec;
    } else {
    diff.tv_usec = tv.tv_usec - LAST_TIME.tv_usec + 1000000;
    diff.tv_sec  = tv.tv_sec  - LAST_TIME.tv_sec - 1;
    }


    //printf ("elapsedclock:  %d  %d\n",diff.tv_sec,diff.tv_usec);
    LAST_TIME = tv;

    if (diff.tv_sec > 0 ||
    diff.tv_usec > 300000) {
    return '\0';
    }*/
  return LAST_KEY;
}


void GLCanvas::setKey(char key) {
  //gettimeofday(&LAST_TIME,NULL);
  LAST_KEY = key;
}

char GLCanvas::whichKeyOnMousePress() {
  return KEY_ON_CLICK;
}

void GLCanvas::setKeyOnPress() {
  KEY_ON_CLICK = currentKey();
}
void GLCanvas::clearKeyOnRelease() {
  KEY_ON_CLICK = '\0';
}
