#include "../paint/gl_includes.h"


#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>
#include <cmath>

#include "../../common/directory_locking.h"
#include "../../../../remesher/src/vectors.h"

#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"

#include "argparser.h"
#include "../paint/button.h"
#include "../paint/path.h"
#include "../../common/Image.h"

#include "pong.h"
#include "../paint/text.h"

#define IR_STATE_DIRECTORY                "../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"

#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args;
DirLock global_dirlock(IR_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;
PointTracker *global_point_tracker;
PongGame *game;

Colors global_colors;
std::vector<Button> buttons;

double TILE_w;
double TILE_h;

// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

void draw_strokes(bool button_strokes);
void draw_buttons();

void initialize_buttons();
void check_for_button_press();
void check_for_button_motion();

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


// ==========================================================================================
// DRAWING ROUTINES
// ==========================================================================================

void draw() { 

  static GLfloat amb[] =  {0.4, 0.4, 0.4, 0.0};
  static GLfloat dif[] =  {1.0, 1.0, 1.0, 0.0};

  float s = 0.0;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  amb[3] = dif[3] = cos(s) / 2.0 + 0.5;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT2);
  glDisable(GL_LIGHT1);
  amb[3] = dif[3] = 0.5 - cos(s * .95) / 2.0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  args->tiled_display.ORTHO();
  //gluOrtho2D(0, args->width, 0, args->height);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  //std::sort(buttons.begin(),buttons.end());

  //  draw_grid();
  //draw_grid2();
  draw_strokes(true);
  draw_buttons();
  draw_strokes(false);
  
  game->draw();

  glDisable(GL_LINE_SMOOTH);
  //  glDisable(GL_BLEND);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);

  glutSwapBuffers();

}

void draw_buttons() {

  glLineWidth(1);
  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glVertex2d(buttons[0].getCentroid().x, buttons[0].getCentroid().y);
  glVertex2d(buttons[1].getCentroid().x, buttons[1].getCentroid().y);
  glColor3f(0, 1, 1);
  glVertex2d(buttons[2].getCentroid().x, buttons[2].getCentroid().y);
  glVertex2d(buttons[3].getCentroid().x, buttons[3].getCentroid().y);
  glEnd();

  for (unsigned int i = 0; i < buttons.size(); i++) {
    buttons[i].paint();

    Vec3f color = buttons[i].getColor();

    // do some custom drawing without messing with the button class
    glBegin(GL_QUADS);
    glColor3f(0,0,0);

    Pt p = buttons[i].getLowerLeftCorner(); //Position();
    glVertex2d(p.x + buttons[i].getWidth() * 0.05, p.y + buttons[i].getWidth() * 0.05);
    glVertex2d(p.x + buttons[i].getWidth() * 0.95, p.y + buttons[i].getWidth() * 0.05);
    glVertex2d(p.x + buttons[i].getWidth() * 0.95, p.y + buttons[i].getWidth() * 0.95);
    glVertex2d(p.x + buttons[i].getWidth() * 0.05, p.y + buttons[i].getWidth() * 0.95);

    if(buttons[i].isPressed()){
      glColor3f(color[0], color[1], color[2]);
      glVertex2d(p.x + buttons[i].getWidth() * 0.40, p.y + buttons[i].getWidth() * 0.40);
      glVertex2d(p.x + buttons[i].getWidth() * 0.60, p.y + buttons[i].getWidth() * 0.40);
      glVertex2d(p.x + buttons[i].getWidth() * 0.60, p.y + buttons[i].getWidth() * 0.60);
      glVertex2d(p.x + buttons[i].getWidth() * 0.40, p.y + buttons[i].getWidth() * 0.60);      
    }

    glEnd();

    if(!buttons[i].isPressed()){
      Pt cent = buttons[i].getCentroid();
      drawstring(cent.x, cent.y, "grab", color,
		 //color[0], color[1], color[2],
		 buttons[i].getWidth() * 0.8, buttons[i].getHeight() * 0.8);
    }
    
  }
}

void draw_strokes(bool button_strokes) {
  assert (global_point_tracker != NULL);

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    int id = (*global_point_tracker)[i].getID();
    bool pressing = false;
    // check if this tracker is attached to a button
    for (unsigned int j = 0; j < buttons.size(); j++) {
      if (buttons[j].isPressed() && buttons[j].PressedBy() == id) 
	pressing = true;
    }
    if (!pressing && button_strokes) continue;
    if (pressing && !button_strokes) continue;

    if (!(*global_point_tracker)[i].IsActive()) continue;

    // SET COLOR
    Vec3f color = global_colors.GetColor(id);
    glColor3f(color.x(),color.y(),color.z());

    // DRAW TRAIL
    glLineWidth(3);
    Path::draw_smooth_stroke((*global_point_tracker)[i].getPtTrail());
  }
}

void display(void) {
  HandleGLError("BEFORE DISPLAY");
  draw();
  HandleGLError("AFTER DISPLAY");
}



// ==========================================================================================
// TRACKING ROUTINES
// ==========================================================================================


void AddTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  global_colors.AssignRandomAvailableColor(id);
  global_colors.ReAssignColor(id,0);  // starts
}


void RemoveTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  Vec3f color = global_colors.GetColor(id);
  global_colors.RemoveId(id);


  for (unsigned int i = 0; i < buttons.size(); i++) {
    if (buttons[i].isPressed() && buttons[i].PressedBy() == id) {
      //button_pressed_by[i] = -1;
      buttons[i].release();
    }
  }

}


// ==========================================================================================
// IDLE
// ==========================================================================================

void idle(void) {

  // look for new IR data
  while (!global_dirlock.TryLock()) {
    usleep(1000);
  }
  std::vector<IR_Data_Point> raw_points;    
  bool success;
  { /* SCOPE FOR istr */
    std::ifstream istr(FOUND_IR_POINTS_FILENAME);
    assert (istr);
    success = PointTracker::ReadIRPointData(istr,raw_points);
  } /* SCOPE FOR istr */
  global_dirlock.Unlock();
  if (!success) {
    // new data not yet available
    usleep(1000);
    return;
  }
  global_point_tracker->ProcessPointData(raw_points);
  
  check_for_button_press();
  check_for_button_motion();

  // get buttonPositions
  std::vector<Vector2> positions;

  const int MAX_WALL_LEN = 300;
  for(int i = 0; i < 4; i += 2){
    Vector2 left(buttons[i].getCentroid().x, buttons[i].getCentroid().y);
    Vector2 right(buttons[i+1].getCentroid().x, buttons[i+1].getCentroid().y);
    Vector2 dir = right - left;
    double len = dir.magnitude();
    if(len == 0){
      continue;
    }
    //if(len <= MAX_WALL_LEN){
    //  positions.push_back(left);
    //  positions.push_back(right);
    //}
    //else{
    double d = 0.5 * (1 - MAX_WALL_LEN / len);
    positions.push_back(left + dir * d);
    positions.push_back(right - dir * d);
      // }
  }

  /*
  for(unsigned int i = 0; i < buttons.size(); i++){
    
    positions.push_back(Vector2(buttons[i].getCentroid().x,
				buttons[i].getCentroid().y));
  }
  */

  // update pong
  game->runFrame(positions, 1.0);

  //nudge_toward_grid();

  display();
  // ~ 60 fp
  usleep(1000);
  // ~ 30 fp
  //usleep(30000);
  //usleep(50000);
}


#define HOVER_THRESHOLD 10


void check_for_button_motion() {

  for (unsigned int j = 0; j < buttons.size(); j++) {
    if (!buttons[j].isPressed()) continue;
    int id = buttons[j].PressedBy();
    assert (id != -1);

    //global_colors.ReAssignColor(id,j+1); 

    Button &b2 = buttons[j];
    for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
      if (id != (*global_point_tracker)[i].getID()) continue;

      Pt p = (*global_point_tracker)[i].getCurrentPosition();
      Pt offset = buttons[j].getPressPosition();

      Pt q = Pt(p.x-offset.x,p.y-offset.y);

      if(j < 4){
	// clamp q to dimensions of pong court
	q.y = std::max(q.y, (double)game->getTopBound());
	q.y = std::min(q.y, (double)game->getBottomBound());
	q.x = std::max(q.x, (double) ((j < 2) ? game->getLeftBound() : game->getMidCourtX()));
	q.x = std::min(q.x, (double) ((j > 1) ? game->getRightBound() : game->getMidCourtX()));
      }

      q -= Pt(buttons[j].getWidth(), buttons[j].getHeight()) * 0.5;

      //b2.Move(q);
      b2.MoveWithMaxSpeed(q, 10);
    }
  }
}

void check_for_button_press() {

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    if (!(*global_point_tracker)[i].IsActive()) continue;
    int id = (*global_point_tracker)[i].getID();
    const std::list<Pt> &trail = (*global_point_tracker)[i].getPtTrail();      

    bool already = false;
    // check if this tracker is already attached to a button
    for (unsigned int j = 0; j < buttons.size(); j++) {
      if (buttons[j].isPressed() && buttons[j].PressedBy() == id) {
	already = true;
      }
    }
    if (already) continue;


    
    //for (unsigned int j = 0; j < buttons.size(); j++) {
    for (int j = int(buttons.size())-1; j >= 0; j--) {
      Button &b2 = buttons[j];
      if (b2.Hover(trail,HOVER_THRESHOLD)) {

	// don't steal from someone else!
	if (!buttons[j].isPressed()) { 
	  Pt pt = b2.Offset((*global_point_tracker)[i].getCurrentPosition());
	  global_colors.ReAssignColor(id,j+1); 
	  b2.press(global_colors.GetColor(id),id,pt);
	}

	// don't click more than one button!!!
	break;
      }
    }

  }
}



void reshape(int w, int h) {
  HandleGLError("BEFORE RESHAPE");

  args->tiled_display.reshape(w,h);
  //args->width = w;
  //args->height = h;
  glViewport(0,0,w,h);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, 1.0, 1.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,
	    0.0, 0.0, 0.0,     
	    0.0, 1.0, 0.); 
  glTranslatef(0.0, 0.6, -1.0);
  initialize_buttons();
  HandleGLError("AFTER RESHAPE");
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q') { 
    exit(0); 
  }
  else if(key == 'b' || key == ' '){
    game->keypress();
  }
  else if(key == 'r'){
    game->resetGame();
  }
  else {
    std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
  display();
  //  glutPostRedisplay();
}


// ===================================================================


void initialize_buttons() {

  buttons.clear();
  
  double button_width = args->tiled_display.full_display_width * 0.03;
  double button_height = button_width; //args->height * 0.05;

  buttons.push_back(Button(Pt(args->tiled_display.full_display_width * 0.2, args->tiled_display.full_display_height * 0.75),
			   button_width, button_height, Vec3f(1,0,0)));
  buttons.push_back(Button(Pt(args->tiled_display.full_display_width * 0.2, args->tiled_display.full_display_height * 0.2),
			   button_width, button_height, Vec3f(1,0,0)));

  buttons.push_back(Button(Pt(args->tiled_display.full_display_width * 0.75, args->tiled_display.full_display_height * 0.2),
			   button_width, button_height, Vec3f(0,1,1)));
  buttons.push_back(Button(Pt(args->tiled_display.full_display_width * 0.75, args->tiled_display.full_display_height * 0.75),
			   button_width, button_height, Vec3f(0,1,1)));
}





int  main(int argc, char **argv) {

  std::vector<Vec3f> colors;
  colors.push_back(Vec3f(1,1,1));
  colors.push_back(Vec3f(1,0.5,0));
  colors.push_back(Vec3f(1,0.5,0.5));
  colors.push_back(Vec3f(0,0.5,1));
  colors.push_back(Vec3f(0.5,0,1));

  global_colors = Colors(colors);

  args = new ArgParser(argc,argv);
  global_point_tracker = NULL;
  global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);

  //args->width = global_calibration_data->getWidth();
  //args->height = global_calibration_data->getHeight();

  args->tiled_display.set_from_calibration_data(global_calibration_data);

  game = new PongGame();
  //game->init(600, 900);
  game->init(args->tiled_display.full_display_width, args->tiled_display.full_display_height);

  std::cout << "pong initialized" << std::endl;

  // initialize things...
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);
  glutInitWindowPosition(20,20);
  glutCreateWindow("planar calibration");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);

  GLfloat light0_ambient[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat light0_diffuse[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat light1_diffuse[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat light1_position[] = {1.0, 1.0, 1.0, 0.0};
  GLfloat light2_diffuse[] = {0.0, 1.0, 0.0, 1.0};
  GLfloat light2_position[] = {-1.0, -1.0, 1.0, 0.0};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(2.0);
  
  HandleGLError("main 1");

  if (args->tiled_display.full_screen) {
    glutFullScreen();
  } 

  HandleGLError("BEFORE INITIALIZE BUTTONS");

  initialize_buttons();
  HandleGLError("AFTER INITIALIZE BUTTONS");
  std::cout << "FINISHED INITIALIZING THE BUTTONS" << std::endl;
  
  global_point_tracker = new PointTracker(global_calibration_data,
					  AddTrackedPoint,RemoveTrackedPoint, 20,0.5);
  
  glutMainLoop();

  return 0;
}
