#include "../../common/OffScreenBuffer.hpp"

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
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
#include <utility>
#include "../../common/MersenneTwister.h"
#include "../../common/directory_locking.h"
#include "animals.h"


MTRand global_rand;
std::vector<Animal> global_animals;
Group *global_topgroup = NULL;
std::vector<DrawnNode*> global_graph;
std::vector<DrawnNode*> global_floor_tokens;

bool global_render_facts = false;

std::string global_pen_history_file;

std::string global_selected_animal = "none";
bool global_expand_on_click = true;

int global_click_counter = 10000;

std::string global_image_directory;
std::vector<std::string> global_image_filenames;

int global_save_frequency = 10;

bool global_toggle = true;

int global_width;
int global_height;

int global_last_x;
int global_last_y;
bool global_last_click_on_wall;

std::list<std::pair<double,double> > global_last_clicks;


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



GLfloat light0_ambient[] =
{0.2, 0.2, 0.2, 1.0};
GLfloat light0_diffuse[] =
{0.0, 0.0, 0.0, 1.0};
GLfloat light1_diffuse[] =
{1.0, 0.0, 0.0, 1.0};
GLfloat light1_position[] =
{1.0, 1.0, 1.0, 0.0};
GLfloat light2_diffuse[] =
{0.0, 1.0, 0.0, 1.0};
GLfloat light2_position[] =
{-1.0, -1.0, 1.0, 0.0};
float s = 0.0;
GLfloat angle1 = 0.0, angle2 = 0.0;



void  drawstring(GLfloat x, GLfloat y, const char *text, double r, double g, double b, double my_extra_scale = 1.0) {

  const char * p;

  double length = 0;
  for (p = text; *p; p++) {
    length += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
  }

  double scale = 0.25 * my_extra_scale;
  length *= scale;
  
  double above = 119.05 * scale;
  double below = 33.33 * scale;
  double extra = 20 * scale;

  double diff = (above+below)/2.0;

  glLineWidth(1.0);
  glPointSize(1.0);

  //  glColor4f(0.25*r,0.25*g,0.25*b,0.25);
  glColor4f(r,g,b,0.25);
  glBegin(GL_QUADS);
  glVertex3f(x-length/2.0-extra,y-diff-extra,0);
  glVertex3f(x+length/2.0+extra,y-diff-extra,0);
  glVertex3f(x+length/2.0+extra,y+diff+extra,0);
  glVertex3f(x-length/2.0-extra,y+diff+extra,0);
  glEnd();

  glColor3f(r,g,b);
  glPushMatrix();
  glTranslatef(x-length/2.0, y-(above-below)/2.0, 0);
  glScalef(scale,scale,scale);
  
  for (p = text; *p; p++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
  glPopMatrix();
}



void drawgraph() {
  if (global_last_click_on_wall) {
    glColor3f(0,1,0);
    glPointSize(10);
    glBegin(GL_POINTS);
    glVertex3f(global_last_x,global_last_y,0);
    glEnd();

    glColor3f(0,1,0);
    glPointSize(3);
    glBegin(GL_POINTS);
    for (std::list<std::pair<double,double> >::iterator itr = global_last_clicks.begin(); itr != global_last_clicks.end(); itr++) {
      glVertex3f(itr->first,itr->second,0); //*global_width,itr->second*global_height,0);
      //std::cout << "draw";
    }
    //    std::cout << std::endl;
    glEnd();
  }

  for (unsigned int i = 0; i < global_graph.size(); i++) {
    DrawnNode *n = global_graph[i];
    double x = n->getX();
    double y = n->getY();
    double r = n->getR();
    double g = n->getG();
    double b = n->getB();
    glColor3f(r,g,b);
    drawstring(x*global_width, y*global_height, n->getName().c_str(),r,g,b,global_width/1400.0);
    //glPointSize(5);
    //glBegin(GL_POINTS);
    //glVertex2f(global_width*n->getX(),global_height*n->getY());
    //glEnd();
    glPointSize(1);
    glLineWidth(2.0);
    for (unsigned int j = i+1; j < global_graph.size(); j++) {
      DrawnNode *n2 = global_graph[j];
      double r2 = n2->getR();
      double g2 = n2->getG();
      double b2 = n2->getB();
      if (n->IsConnected(n2)) {
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3f(r,g,b);
	glVertex2f(global_width*n->getX(),global_height*n->getY());
	glColor3f(r2,g2,b2);
	glVertex2f(global_width*n2->getX(),global_height*n2->getY());
	glEnd();
      }
    }
  }
}


void drawcontrols() {
  if (!global_last_click_on_wall) {
    //    std::cout << "DRAW CLICK " << global_last_x << " " << global_last_y << std::endl;
    glColor3f(0,1,0);
    glPointSize(10);
    glBegin(GL_POINTS);
    glVertex3f(global_last_x,global_last_y,0);
    glEnd();

    glColor3f(0,1,0);
    glPointSize(3);
    glBegin(GL_POINTS);
    for (std::list<std::pair<double,double> >::iterator itr = global_last_clicks.begin(); itr != global_last_clicks.end(); itr++) {
      glVertex3f(itr->first,itr->second,0); //*global_width,itr->second*global_height,0);
      //glVertex3f(itr->first*global_width,itr->second*global_height,0);
    }
    glEnd();
  }
  for (unsigned int i = 0; i < global_floor_tokens.size(); i++) {
    DrawnNode *n = global_floor_tokens[i];
    double x = n->getX();
    double y = n->getY();
    double r = n->getR();
    double g = n->getG();
    double b = n->getB();
    glColor3f(r,g,b);
    drawstring(x*global_width, y*global_height, n->getName().c_str(),r,g,b,global_width/1400.0);
    //drawstring(x*global_width, y*global_height, n->getName().c_str(),r,g,b);
    //glPointSize(5);
    //glBegin(GL_POINTS);
    //glVertex2f(global_width*n->getX(),global_height*n->getY());
    //glEnd();
  }
}
  


void drawfacts(const Animal *animal) {
  glScalef(1.5,1.5,1.5);
  drawstring(0.25*global_width, 0.4*global_height, animal->getName().c_str(),1,1,1);

  glScalef(0.5,0.5,0.5);
  int num = animal->numLevelsOfClassification();
  for (int i = 0; i < num; i++) {
    int j = num-i;
    if (i == 0) {
      drawstring(0.5*global_width, (0.75-0.15*j)*global_height, animal->getClassificationLevel(i).c_str(),1,0,0);
    } else if (i == 1) {
      drawstring(0.5*global_width, (0.75-0.15*j)*global_height, animal->getClassificationLevel(i).c_str(),1,1,0);
    } else if (i == 2) {
      drawstring(0.5*global_width, (0.75-0.15*j)*global_height, animal->getClassificationLevel(i).c_str(),0,1,0);
    } else if (i == 3) {
      drawstring(0.5*global_width, (0.75-0.15*j)*global_height, animal->getClassificationLevel(i).c_str(),0,0,1);
    } else {
      assert(0);
    }
  }
}
  



void draw(bool draw_graph, bool draw_tokens, bool draw_facts, const Animal *animal) {
  static GLfloat amb[] =
  {0.4, 0.4, 0.4, 0.0};
  static GLfloat dif[] =
  {1.0, 1.0, 1.0, 0.0};

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

  gluOrtho2D(0, global_width, 0, global_height);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  /* Rotate text slightly to help show jaggies. */
  //  glRotatef(4, 0.0, 0.0, 1.0);

  if (draw_graph) {
    drawgraph();
  }
  if (draw_tokens) {
    drawcontrols();
  }
  if (draw_facts) {
    drawfacts(animal);
  }

  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  //drawstring(160, 100, "This text is not.");
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);

  glutSwapBuffers();

}

 
void save_to_file() {
  
  static DirLock *directory = NULL;
  if (directory == NULL) {
    directory = new DirLock(global_image_directory.c_str());
  }

  assert (directory != NULL);
  directory->Lock();

  OffScreenBuffer buffer(global_width, global_height, false);
  // select the off-screen buffer for rendering

  if (global_render_facts) {
    for (unsigned int i = 0; i < global_animals.size(); i++) {
      const Animal &animal = global_animals[i];

      // DRAW GRAPH
      buffer.Select();  
      draw(false,false,true,&animal);
      // save the color buffer
      std::string filename = "facts_images/"+animal.getName()+"_facts.ppm";
      buffer.WritePPM(filename.c_str());
      // unselect the off-screen buffer
      buffer.UnSelect();

    }


  } else {

    // DRAW GRAPH
    buffer.Select();  
    draw(true,false,false,NULL);
    // save the color buffer
    std::string filename = global_image_directory + "/back_wall.ppm";
    buffer.WritePPM(filename.c_str());
    // unselect the off-screen buffer
    buffer.UnSelect();
    
    // DRAW FLOOR TOKENS
    buffer.Select();  
    draw(false,true,false,NULL);
    // save the color buffer
    filename = global_image_directory + "/floor.ppm";
    buffer.WritePPM(filename.c_str());
    // unselect the off-screen buffer
    buffer.UnSelect();
    
    
    filename = global_image_directory + "/state.txt";
    {
      std::vector<std::string> tmp = global_image_filenames;
      
      std::ofstream ostr(filename.c_str());
      if (tmp.size() == 0) {
	tmp.push_back("cropped_images/"+global_selected_animal+".ppm");
	tmp.push_back("facts_images/"+global_selected_animal+".ppm");
	tmp.push_back("floor.ppm");
      tmp.push_back("back_wall.ppm");
      } else {
	std::string foo; 
	foo = "cp cropped_images/"+global_selected_animal+".ppm "+global_image_directory+"/"+global_image_filenames[0];
	system(foo.c_str());
	foo = "cp facts_images/"+global_selected_animal+"_facts.ppm "+global_image_directory+"/"+global_image_filenames[1];
	system(foo.c_str());

	foo = "mv "+global_image_directory+"/floor.ppm "+global_image_directory+"/"+global_image_filenames[2];
	system(foo.c_str());
	foo = "mv "+global_image_directory+"/back_wall.ppm "+global_image_directory+"/"+global_image_filenames[3];
	system(foo.c_str());


      }
      assert (tmp.size() == 4);
      
      ostr << "canvas_wall_0    " << tmp[0] << std::endl;
      ostr << "luan_wall_0      " << tmp[1] << std::endl;
      ostr << "floor_0_0        " << tmp[2] << std::endl;
      ostr << "empac_back_wall  " << tmp[3] << std::endl;
    }
  }  

  directory->Unlock();
  HandleGLError("main 5");

  if (global_render_facts) { exit(0); }
}


void display(void) {
  static int count = 0;
  count++;
  draw(global_toggle,!global_toggle,false,NULL);
  if (global_image_directory != "" && count%global_save_frequency == 0) {
    //    std::cout << "save to files" << std::endl;
    save_to_file();
  }
}

#define INCH_IN_METERS 0.0254

// the floor
#define EMPAC_SMALL_MINUS_X   (          4) * INCH_IN_METERS 
#define EMPAC_SMALL_POS_X     (15 * 12 + 4) * INCH_IN_METERS 
#define EMPAC_SMALL_MINUS_Z  -( 7 * 12 + 3) * INCH_IN_METERS 
#define EMPAC_SMALL_POS_Z     (13 * 12 + 1) * INCH_IN_METERS 

// distance from origin in meters (in the positive x direction)
#define EMPAC_BACK_WALL_DISTANCE 5.2578  
// (in the z direction)
// WIDTH = 21.5 FEET 
// HEIGHT = 18 FEET
#define EMPAC_BACK_WALL_LEFT_EDGE -(21.5/2.0 - 3) * 12 * INCH_IN_METERS
#define EMPAC_BACK_WALL_RIGHT_EDGE (21.5/2.0 + 3) * 12 * INCH_IN_METERS
// (in the y direction)
#define EMPAC_BACK_WALL_BOTTOM_EDGE 3 * INCH_IN_METERS
#define EMPAC_BACK_WALL_TOP_EDGE (3 + 18 * 12) * INCH_IN_METERS


bool pen_click(const std::string &surface, double x, double y, double z, double &u, double &v) {
  global_click_counter = 0;
  if (surface == "floor") {
    if (fabs(y) > INCH_IN_METERS) return false;
    u = (z - EMPAC_SMALL_MINUS_Z) / double(EMPAC_SMALL_POS_Z - EMPAC_SMALL_MINUS_Z);
    v = (x - EMPAC_SMALL_MINUS_X) / double(EMPAC_SMALL_POS_X - EMPAC_SMALL_MINUS_X);
    //std::cout << "floor click" << u << " " << v << std::endl;
    if (u >= 0.0 && u < 1.0 && v >= 0.0 && v < 1.0) {
      return true;
    } else {
      return false;
    }
  } else if (surface == "wall") {
    if (fabs(x-EMPAC_BACK_WALL_DISTANCE) > INCH_IN_METERS) return false;
    u = (z - EMPAC_BACK_WALL_LEFT_EDGE) / double(EMPAC_BACK_WALL_RIGHT_EDGE - EMPAC_BACK_WALL_LEFT_EDGE);
    v = (y - EMPAC_SMALL_MINUS_X) / double(EMPAC_SMALL_POS_X - EMPAC_SMALL_MINUS_X);

    v = v - 0.03;

    //std::cout << "wall click" << u << " " << v << std::endl;
    if (u >= 0.0 && u < 1.0 && v >= 0.0 && v < 1.0) {
      return true;
    } else {
      return false;
    }
  } else {
    assert (0);
  }
}



void read_pen_history(void) {


  if (global_pen_history_file == "") return;
  std::ifstream istr(global_pen_history_file.c_str());

  int frame;
  std::string surface;
  double x;
  double y;
  double z;

  static int last_frame = -1;
  static std::string last_surface = "none";


  //  static std::list<std::pair<double,double> > clicks;
  //static double avg_u = 0;
  //static double avg_v = 0;
  //static int count = 0;


  while (istr >> frame >> surface >> x >> y >> z) {
    if (frame <= last_frame) continue;
    last_frame = frame;

    //    std::cout << "new frame "; fflush(stdout);


    if (last_surface != "none" && last_surface != surface) {
      std::cout << " MOUSE RELEASE" << global_last_clicks.size() << std::endl;
      if (global_last_clicks.size() >= 5) {
	assert (global_last_clicks.size() == 5);
	double u = 0;
	double v = 0;
	//	global_last_clicks.clear();
	for (std::list<std::pair<double,double> >::iterator itr = global_last_clicks.begin(); itr != global_last_clicks.end(); itr++) {
	  u += itr->first; ///float(global_last_clicks.size());
	  v += itr->second; /// float(global_last_clicks.size());
	}
	//	global_last_clicks.push_back(std::make_pair(u,v));
	global_last_x = u/float(global_last_clicks.size());
	global_last_y = v/float(global_last_clicks.size());
	
	std::cout << "click " << surface << std::endl;

	if (last_surface == "wall") {
	  std::cout << "CLICK POSITION WALL " << u << " " << v << std::endl;
	  global_last_click_on_wall = true;
	  ClickGraphExpand(global_graph,global_topgroup,global_last_x/float(global_width),global_last_y/float(global_height),global_floor_tokens);
	  
	} else if (last_surface == "floor") {
	  std::cout << "CLICK POSITION FLOOR " << u << " " << v << std::endl;
	  global_last_click_on_wall = false;	  
	  ClickToken(global_floor_tokens,global_graph,global_topgroup,global_last_x/float(global_width),global_last_y/float(global_height));
	} else {
	  assert (0);
	}
      }
    }
    
    if (last_surface != surface && surface != "none") {
      std::cout << "MOUSE PRESS!" << std::endl;
      
      global_last_clicks.clear();
      double u,v;
      bool success = pen_click(surface,x,y,z,u,v);
      if (success) {
	global_last_clicks.push_back(std::make_pair(u*global_width,v*global_height));
	assert (global_last_clicks.size() == 1); //> 5) global_last_clicks.pop_front();
      }
      last_surface = surface;

      if (last_surface == "wall") {
	std::cout << "CLICK POSITION WALL " << u << " " << v << std::endl;
	global_last_click_on_wall = true;
      } else if (last_surface == "floor") {
	std::cout << "CLICK POSITION FLOOR " << u << " " << v << std::endl;
	global_last_click_on_wall = false;	  
      } else {
	assert (0);
      }

    }

    if (surface != "none" && last_surface == surface) {
      double u,v;
      bool success = pen_click(surface,x,y,z,u,v);
      if (success) {
	global_last_clicks.push_back(std::make_pair(u*global_width,v*global_height));
	if (global_last_clicks.size() > 5) global_last_clicks.pop_front();
      }

      //global_last_clicks.clear();
      for (std::list<std::pair<double,double> >::iterator itr = global_last_clicks.begin(); itr != global_last_clicks.end(); itr++) {
	u += itr->first; ///float(global_last_clicks.size());
	v += itr->second; /// float(global_last_clicks.size());
      }
      global_last_x = u/float(global_last_clicks.size());
      global_last_y = v/float(global_last_clicks.size());
      //global_last_x = u*global_width/float(global_last_clicks.size());
      //global_last_y = (1-v)*global_height/float(global_last_clicks.size());
      //global_last_x = u;//*global_width;
      //global_last_y = (1-v);//*global_height;
    }

    //    std::cout << "set surface " << surface << std::endl;
    last_surface = surface;
    
  }
  
}




void idle(void) {
  read_pen_history();

  angle1 = (GLfloat) fmod(angle1 + 0.8, 360.0);
  angle2 = (GLfloat) fmod(angle2 + 1.1, 360.0);
  s += 0.05;
  AdjustGraph(global_graph);
  display();
  glutPostRedisplay();
  
  global_click_counter++;
  if (global_click_counter > 25) {
    global_last_x = -20;
    global_last_y = -20;
    global_last_clicks.clear();
  }

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
    exit(0); 
  } else if (key == 'r') {
    RandomizeGraph(global_graph); 
  } else if (key == 'e') {
    ExpandAll(global_graph,global_topgroup); 
  } else if (key == 'c') {
    CollapseAll(global_graph); 
  } else if (key == 't') {
    //std::cout << "TOGGLE " << global_toggle << std::endl;
    global_toggle = !global_toggle;
  } else {
    std::cout << "unknown key" << std::endl;
  }
  display();
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
  global_click_counter = 0;
  if (state == GLUT_DOWN) {
    global_last_x = x;
    global_last_y = global_height-y;

    //std::cout << "click " << surface << std::endl;

    if (global_toggle) {
      std::cout << "click group " << x << " " << y << std::endl;
      ClickGraphExpand(global_graph,global_topgroup,x/double(global_width),(1-y/double(global_height)),global_floor_tokens);
      global_last_click_on_wall = true;
    } else {
      std::cout << "click token " << x << " " << y << std::endl;
      ClickToken(global_floor_tokens,global_graph,global_topgroup,x/double(global_width),(1-y/double(global_height)));
      global_last_click_on_wall = false;
    }
  }
}


// ===================================================================

int  main(int argc, char **argv) {

  std::string animals_filename;

  global_width = 1400;
  global_height = 1050;

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == std::string("-i")) {
      i++; assert (i < argc);
      animals_filename = argv[i];
    } else if (std::string(argv[i]) == std::string("-dimensions")) {
      i++; assert (i < argc);
      global_width = atoi(argv[i]);
      i++; assert (i < argc);
      global_height = atoi(argv[i]);
    } else if (std::string(argv[i]) == std::string("-image_directory")) {
      i++; assert (i < argc);
      global_image_directory = argv[i];
    } else if (std::string(argv[i]) == std::string("-image_filenames")) {
      i++; assert (i < argc);
      global_image_filenames.push_back(argv[i]);
      i++; assert (i < argc);
      global_image_filenames.push_back(argv[i]);
      i++; assert (i < argc);
      global_image_filenames.push_back(argv[i]);
      i++; assert (i < argc);
      global_image_filenames.push_back(argv[i]);
    } else if (std::string(argv[i]) == std::string("-pen_history")) {
      i++; assert (i < argc);
      global_pen_history_file = argv[i];
    } else if (std::string(argv[i]) == std::string("-render_facts")) {
      global_render_facts = true;
    } else {
      assert (0);
    }
  }

  LoadAnimalData(animals_filename,global_animals,global_topgroup);

  MakeGraph(global_topgroup, global_graph);
  MakeFloorTokens(global_animals, global_floor_tokens);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(global_width,global_height);
  glutInitWindowPosition(20,20);
  glutCreateWindow("blender");
  glutDisplayFunc(display);
  //glutIdleFunc(idle);
  glutVisibilityFunc(visible);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  /*
  glNewList(1, GL_COMPILE);  / * create ico display list * /
  glutSolidIcosahedron();
  glEndList();
  */

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(2.0);

  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
  /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
  glTranslatef(0.0, 0.6, -1.0);
  
  HandleGLError("main 1");

  glutMainLoop();

  return 0;             /* ANSI C requires main to return int. */
}
