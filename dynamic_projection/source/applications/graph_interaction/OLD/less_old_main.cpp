// multi_surface_graph main

#include "../../common/OffScreenBuffer.hpp"

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <list>
#include <set>
#include <utility>
#include "../../common/directory_locking.h"
#include "../../common/Image.h"

#include "animals.h"
#include "argparser.h"

#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"
#include "../paint/button.h"
#include "../paint/path.h"
#include "../paint/text.h"
#include "action.h"

#define IR_STATE_DIRECTORY                "../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"
#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

#define APPLICATIONS_STATE_DIRECTORY            "../state/applications/"
#define ANIMAL_GRAPH_STATE_FILENAME             "../state/applications/animal_graph_state.txt"


//#define HOVER_THRESHOLD 10

ArgParser *args;
DirLock global_ir_dirlock(IR_STATE_DIRECTORY);
DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;
PointTracker *global_point_tracker;

Colors global_colors;

//std::vector<Button*> GLOBAL_buttons;

std::vector<DrawnNode*> GLOBAL_buttons;



std::map<int,Action> actions;

Group *global_topgroup = NULL;
std::vector<Thing*> global_things;

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

#define CIRCLE_RES 20


void draw_buttons() 
{
  //std::cout << "---------------------------------------------" << std::endl;
  //std::cout << "ENTER DRAW BUTTONS" << std::endl;

  HandleGLError("BEFORE draw_buttons()");

  std::sort(GLOBAL_buttons.begin(),GLOBAL_buttons.end(),button_pointer_sorter);
  int num_pressed = 0;
  for (unsigned int i = 0; i < GLOBAL_buttons.size(); i++) {
    //if (GLOBAL_buttons[i]->getDrawnNode() == NULL ||
    //	GLOBAL_buttons[i]->getDrawnNode()->isInvisible()) continue;
    if (GLOBAL_buttons[i]->isVisible()/*visible*/ == false) continue;
    GLOBAL_buttons[i]->paint();
    if (GLOBAL_buttons[i]->isPressed()) {
      num_pressed++;
    }
  }

  if (num_pressed > (int)actions.size()) {
    //std::cout << "CHECK ACTIONS " << num_pressed << " " << (int)actions.size() << std::endl;
    for (unsigned int i = 0; i < GLOBAL_buttons.size(); i++) {
      GLOBAL_buttons[i]->paint();
      if (GLOBAL_buttons[i]->isPressed()) {
	//std::cout << GLOBAL_buttons[i]->getText() << " pressed by: " << GLOBAL_buttons[i]->PressedBy() << std::endl;
      }
    }
  }
  assert (num_pressed <= (int)actions.size());
  

  HandleGLError("middle of draw_buttons()");

#if 0  
  for (std::map<int,Action>::iterator itr = actions.begin(); itr != actions.end(); itr++) {
    //int id = itr->first;
    Action &action = itr->second;

    /*    if (action.getNode() != NULL) {
    //std::cout << id << " is pressing " << action.getNode()->getName() << std::endl;
    } else {
    //std::cout << id << " is pressing nothing" << std::endl;
    } 
    */

    const Vec3f& c = action.getColor();
    double rad = action.getRadius();
    const Pt& cent = action.getCenter();
    glColor4f(c.r(),c.g(),c.b(),0.5);
    glBegin(GL_POLYGON);
    for (int j = 0; j < CIRCLE_RES; j++) {
      double angle = j*2*M_PI/CIRCLE_RES;
      Pt p = cent + Pt(rad*sin(angle),rad*cos(angle));
      glVertex3f(p.x,p.y,0);
    }
    glEnd();
  }
#endif
  //std::cout << "EXIT DRAW BUTTONS" << std::endl;

  HandleGLError("AFTER draw_buttons()");
}

void draw_strokes(bool button_strokes) {
  assert (global_point_tracker != NULL);

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    int id = (*global_point_tracker)[i].getID();
    bool pressing = false;
    // check if this tracker is attached to a button
    for (unsigned int j = 0; j < GLOBAL_buttons.size(); j++) {
      if (GLOBAL_buttons[j]->isPressed() && GLOBAL_buttons[j]->PressedBy() == id)
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


void drawgraph() {

  glLineWidth(2);
  glBegin(GL_LINES);

  for (unsigned int i = 0; i < global_things.size(); i++) {
    DrawnNode *n = global_things[i]->getDrawnNode();
    if (n == NULL || !n->isVisible()) continue;
    const Pt &p = n->getPosition();
//std::cerr << "Drawing node " << i << std::endl;
//std::cerr << p.x << "  " << p.y << std::endl;
    double r = n->getR();
    double g = n->getG();
    double b = n->getB();
    for (unsigned int j = i+1; j < global_things.size(); j++) {
      DrawnNode *n2 = global_things[j]->getDrawnNode();
      if (n2 == NULL || !n2->isVisible()) continue;
      const Pt &p2 = n2->getPosition();
      double r2 = n2->getR();
      double g2 = n2->getG();
      double b2 = n2->getB();
      if (n->IsConnected(n2)) {
     	glColor3f(r,g,b);
	    glVertex2f(p.x,p.y);
	    glColor3f(r2,g2,b2);
	    glVertex2f(p2.x,p2.y);
      }
    }
  }
  glEnd();
}



void draw() {

  HandleGLError("BEFORE draw()");
  
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

  HandleGLError("BEFORE drawgraph()");

  drawgraph();

  draw_strokes(true);
  draw_buttons();
  draw_strokes(false);

  HandleGLError("AFTER drawstrokes");

  glDisable(GL_LINE_SMOOTH);
  //glDisable(GL_BLEND);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);

  glutSwapBuffers();

  HandleGLError("AFTER draw()");
}


void display(void) {
  HandleGLError("BEFORE DISPLAY");
  draw();
  HandleGLError("AFTER DISPLAY");
}

void check_for_actions() {
  // For each laser, check for associated actions.
  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    if (!(*global_point_tracker)[i].IsActive()) continue;
    int id = (*global_point_tracker)[i].getID();
    std::map<int,Action>::iterator itr = actions.find(id);
    if (itr == actions.end()) continue;
    Action &action = itr->second;
    DrawnNode *n = action.getNode();
    if (n == NULL) continue;
    //Button *b = n; //n->getButton();
    DrawnNode *b = n;
    assert (b->isPressed());
    assert (b->PressedBy() == id);
    if (action.is_drag()) {
      Pt p = (*global_point_tracker)[i].getCurrentPosition();
      Pt offset = b->getPressPosition();
      Pt q = Pt(p.x-offset.x,p.y-offset.y);
      b->Move(q);
    } else if (action.is_expand()) {
      if (action.getActionCount() < 2) continue;
      b->release();
      action.setNode(NULL);
      Expand2(global_things,global_topgroup,n);
      (*global_point_tracker)[i].clear_history();
    } else {
      assert (action.is_collapse());
      if (action.getActionCount() < 5) continue;
      b->release();
      action.setNode(NULL);
      Collapse2(global_things,global_topgroup,n);
      (*global_point_tracker)[i].clear_history();
    }
  }
}




// ================================================================================


#define PIXEL_MOTION_TOLERANCE 10

void check_for_action_initiation() {
  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    if (!(*global_point_tracker)[i].IsActive()) continue;
    int id = (*global_point_tracker)[i].getID();

    const std::list<Pt> &trail = (*global_point_tracker)[i].getPtTrail();

    // identify current action (if any)
    std::map<int,Action>::iterator action_itr = actions.find(id);
    Action *current_action = NULL;
    if (action_itr!=actions.end()) {
      assert (id == action_itr->first);
      current_action = &action_itr->second;
    }

    // if we are already dragging, don't start a new action
    if (current_action != NULL &&
	current_action->is_drag() &&
	current_action->getNode() != NULL) continue;

    // classify the current trail
    double radius;
    Pt center;
    action_classification ac = Classify(trail,PIXEL_MOTION_TOLERANCE, radius,center);

    // prepare the new action
    Action *new_action = NULL;
    Action stupid; // to get around problem with "taking address of temporary"
    if (ac == EARLY_TRACKING || ac == OTHER_CLASSIFICATION) {
      global_colors.ReAssignColor(id,0);
    } else if (ac == STATIONARY) {
      // prepare for drag
      global_colors.ReAssignColor(id,2); // (pen color green)
      stupid = Action::Hover(radius,center);
      new_action = &stupid;
    } else if (ac == HORIZONTAL_Z_STRIKE ||
	     ac == VERTICAL_Z_STRIKE ||
	     ac == Z_STRIKE) {
      global_colors.ReAssignColor(id,1); // (pen color red)
      stupid = Action::ZStrike(radius,center);
      new_action = &stupid;
    } else {
      assert (ac == CLOCKWISE_CIRCLE ||
	      ac == COUNTERCLOCKWISE_CIRCLE);
      global_colors.ReAssignColor(id,3); // (pen color blue)
      stupid = Action::Circle(radius,center);
      new_action = &stupid;
    }

    DrawnNode *new_affected = NULL;
    if (new_action != NULL) {
      // find the button affected by this action
      double best_dist = radius;
      for (unsigned int j = 0; j < global_things.size(); j++) {
		DrawnNode *n2 = global_things[j]->getDrawnNode();
		if (n2 == NULL || !n2->isVisible()) continue;
		//Button *b2 = n2->getButton();
		    DrawnNode *b2 = n2;
		    double my_dist = b2->DistanceFrom(center);
		if (my_dist < best_dist) {
		  // don't steal from someone else!
		  if (!b2->isPressed() || b2->PressedBy() == id) {
			new_affected = n2;
			best_dist = my_dist;
		  }
		}
      }
    }
    
    if (current_action == NULL && new_action == NULL) {
      //std::cout << "nothing before or now" << std::endl;
    } else if (current_action != NULL && new_action == NULL) {
      //std::cout << "action ceased" << std::endl;

      if (current_action->getNode() != NULL) {
	current_action->getNode()->/*getButton()->*/release();
      }

      actions.erase(action_itr);
    } else if (current_action == NULL && new_action != NULL) {

      //std::cout << "action initiated" << std::endl;
      if (new_affected != NULL) {
	//std::cout << "action initiated by " << id << " " << new_affected->getName() << std::endl;
	new_action->setNode(new_affected);
	assert (new_affected/*->getButton()*/ != NULL);

	Pt pt = new_affected/*->getButton()*/->Offset(center);
	new_affected/*->getButton()*/->press(global_colors.GetColor(id),id,pt);

      }
      actions[id] = *new_action;
    } else {
      assert (current_action != NULL && new_action != NULL);

      if (current_action->same_action_type(*new_action)) {
	if (current_action->getNode() == new_affected) {
	  new_action->setNode(new_affected);
	  //std::cout << "action ongoing" << std::endl;
	  if (new_affected != NULL) {
	    //std::cout << "continuing to select " << new_affected->getName() << std::endl;
	    new_action->advanceActionCount(*current_action);
	    //std::cout << "      action count " << new_action->getActionCount() << std::endl;
	    if (new_action->getActionCount() > 15) {
	      std::cout << "ACTION OVERDUE: " << new_affected->getName() << std::endl;
	    }
	  }
	  actions[id] = *new_action;
	} else if (current_action->getNode() == NULL) {
	  //std::cout << "node now selected : " << id << " " << new_affected->getName() << std::endl;
	  new_action->setNode(new_affected);
	  assert (new_affected/*->getButton()*/ != NULL);
	  Pt pt = new_affected/*->getButton()*/->Offset(center);
	  new_affected/*->getButton()*/->press(global_colors.GetColor(id),id,pt);
	  actions[id] = *new_action;
	} else if (new_affected == NULL) {
	  //std::cout << "node released: " << current_action->getNode()->getName() << std::endl;
	  current_action->getNode()/*->getButton()*/->release();
	  actions[id] = *new_action;
	} else {
	  //std::cout << "node change  " << id << " " << current_action->getNode()->getName() << "->" << new_affected->getName() << std::endl;
	  current_action->getNode()/*->getButton()*/->release();
	  new_action->setNode(new_affected);
	  assert (new_affected/*->getButton()*/ != NULL);
	  Pt pt = new_affected/*->getButton()*/->Offset(center);
	  new_affected/*->getButton()*/->press(global_colors.GetColor(id),id,pt);
	  actions[id] = *new_action;
	}
      } else {
	if (current_action->getNode() != NULL) {
	  current_action->getNode()/*->getButton()*/->release();
	}
	new_action->setNode(new_affected);
	if (new_affected != NULL) {
	  assert (new_affected/*->getButton()*/ != NULL);
	  Pt pt = new_affected/*->getButton()*/->Offset(center);
	  new_affected/*->getButton()*/->press(global_colors.GetColor(id),id,pt);
	}
	actions[id] = *new_action;
      }
    }
  }
}




// TRACKING ROUTINES
// ==========================================================================================


void AddTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  // everybody starts with the color white!
  global_colors.AssignColor(id,0);
  //global_colors.AssignRandomAvailableColor(id);
}


void RemoveTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  Vec3f color = global_colors.GetColor(id);
  global_colors.RemoveId(id);

  std::map<int,Action>::iterator itr = actions.find(id);
  if (itr!=actions.end()) {
    actions.erase(itr);
  }


  for (unsigned int i = 0; i < global_things.size(); i++) {
    DrawnNode *n = global_things[i]->getDrawnNode();
    if (n == NULL || !n->isVisible()) continue;
    //    Button *b = n->getButton();
    DrawnNode *b = n;
    if (b->isPressed() && b->PressedBy() == id) {
      // keep the button still for 2 seconds
      b->release(2000);
    }
  }
}

void idle(void) {

  if (args->lasers) {
    // look for new IR data
    while (!global_ir_dirlock.TryLock()) {
      usleep(1000);
    }
    std::vector<IR_Data_Point> raw_points;
    bool success;
    { /* SCOPE FOR istr */
      std::ifstream istr(FOUND_IR_POINTS_FILENAME);
      assert (istr);
      success = PointTracker::ReadIRPointData(istr,raw_points);
    } /* SCOPE FOR istr */
    global_ir_dirlock.Unlock();
    if (!success) {
      // new data not yet available
      usleep(1000);
      return;
    }
    global_point_tracker->ProcessPointData(raw_points);

    check_for_action_initiation();
    check_for_actions();
  }

  if (!args->tiled_display.is_tiled_display) {
    AdjustGraph(global_things);
  } else {
    if (args->tiled_display.is_master) {
      AdjustGraph(global_things);
      SaveGraphState(global_things,ANIMAL_GRAPH_STATE_FILENAME);
    } else {
      LoadGraphState(global_things,ANIMAL_GRAPH_STATE_FILENAME);
    }
  }

  display();
  //  glutPostRedisplay();

  //  usleep(25000);
  usleep(1000);
}

void visible(int vis) {
  if (vis == GLUT_VISIBLE)
    glutIdleFunc(idle);
  else
    glutIdleFunc(NULL);
}


void keyboard(unsigned char key, int x, int y) {
  if (key == 'q') {
    std::cout << "exiting normally" << std::endl;
    exit(0);
  } else if (key == 'e') {
    DrawnNode* top = SearchForDrawnNode(global_things,"animals");
    assert (top != NULL);
    Expand2(global_things,global_topgroup,top);
  } else if (key == 'c') {
    DrawnNode* top = SearchForDrawnNode(global_things,"animals");
    assert (top != NULL);
    Collapse2(global_things,global_topgroup,top);
  } else if (key == 'r') {
    RandomizeGraph(global_things);
  } else {
    std::cout << "unknown key '" << key << "'" << std::endl;
  }
  display();
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {

  /*
  if (y == 0) {
    std::cout << "y is 0" << std::endl;
  }
  if (y == 1) {
    std::cout << "y is 1" << std::endl;
  }

  if (y == args->height-1) {
    std::cout << "y is args->height-1" << std::endl;
  }
  if (y == args->height) {
    std::cout << "y is args->height" << std::endl;
  }
  */
  y = args->tiled_display.full_display_height-y-1;

// CHRIS STUETZLE: Do we need these?
  static int global_last_x;
  static int global_last_y;

  bool shiftPressed = glutGetModifiers() & GLUT_ACTIVE_SHIFT;


  if (state == GLUT_DOWN) {
    global_last_x = x;
    global_last_y = y;

    //    std::cout << "global_last_x " << x <<  " " << y << std::endl;

    if (!shiftPressed) {
      ClickGraphExpandOrCollapse(global_things,global_topgroup,Pt(x,y),true);
    } else {
      ClickGraphExpandOrCollapse(global_things,global_topgroup,Pt(x,y),false);
    }
  }

}

void reshape(int w, int h) {
  HandleGLError("BEFORE RESHAPE");

  args->tiled_display.reshape(w,h);

  //if (w == args->width && h == args->height && args->full_screen) {
    //std::cout << "RESHAPE skip" << w << " " << h << std::endl;
    //return;
  // }

  //std::cout << "RESHAPE" << w << " " << h << std::endl;
  //  args->width = w;
  //args->height = h;
  glViewport(0,0,w,h);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, 1.0, 1.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,
	    0.0, 0.0, 0.0,
	    0.0, 1.0, 0.);
  glTranslatef(0.0, 0.6, -1.0);

  //std::cout << "leaving RESHAPE" << std::endl;
  HandleGLError("AFTER RESHAPE");
}


// ===================================================================

int main(int argc, char **argv) {

//std::cout << "CHECKING!" << std::endl;

  args = new ArgParser(argc,argv);
  global_point_tracker = NULL;

  global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);

  args->tiled_display.set_from_calibration_data(global_calibration_data);

  /*#if 0
  args->width = 800;
  args->height = 600;
#else
  args->width = global_calibration_data->getWidth();
  args->height = global_calibration_data->getHeight();
#endif
  */

  std::vector<Vec3f> colors;
  colors.push_back(Vec3f(1,1,1));  // 0 = white
  colors.push_back(Vec3f(1,0,0));  // 1 = red
  colors.push_back(Vec3f(0,1,0));  // 2 = green
  colors.push_back(Vec3f(0,0,1));  // 3 = blue
  global_colors = Colors(colors);



  // initialize things...
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);
  glutInitWindowPosition(20,20);
  glutCreateWindow("multi-surface graph");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutVisibilityFunc(visible);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);

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

  if (args->tiled_display.full_screen) {
    glutFullScreen();
  }
  LoadAnimalData(args->animals_filename,global_topgroup);
  MakeGraph(global_topgroup, global_things);

  // about 1 sec of data;
  int default_laser_tail_length = 30;

  global_point_tracker = new PointTracker(global_calibration_data,
					  AddTrackedPoint,RemoveTrackedPoint,default_laser_tail_length,0.5);
  HandleGLError("main");
  glutMainLoop();

  return 0;             /* ANSI C requires main to return int. */
}
