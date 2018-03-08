#include "../paint/gl_includes.h"

#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>

//Tyler: for Human Paintbrush
//added for sending UDP (OSC) messages to MaxMSP
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
//#include "PracticalSocket.h"


#include "../../common/directory_locking.h"
#include "../../../../remesher/src/vectors.h"

#include "text.h"
#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"

#include "paint_argparser.h"
#include "button.h"
#include "path.h"


//Tyler Sammann commented
//#include "Polygon.h"


#include "drawn_object.h"

#define IR_STATE_DIRECTORY                "../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"

#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

#define MAX_WIDTH 150
#define DEFAULT_WIDTH 50
#define POLYGON_DEFAULT_WIDTH 5

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

PaintArgParser *args;
DirLock global_dirlock(IR_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;
PointTracker *global_point_tracker;

std::vector<drawn_object*> lolwut;
std::vector<Path> global_finished_strokes;
//std::vector<NonConvexPolygon> global_finished_polygons;

std::map<int,int> selected_colors;

Colors global_colors;
std::map<int,int> stroke_widths;
std::set<int> global_button_strokes;
std::vector<Button> buttons;

//Tyler: testing for mouse
std::list<Pt> mouse_pts;

int global_current_color = 0;
int global_current_width = 5;

int global_random_button_id;
int global_up_button_id;
int global_down_button_id;
int global_undo_button_id;
int global_clear_button_id;

double sample_line_x;
double sample_line_top;
double sample_line_bottom;

Pt mouse_location;

enum {LINE,POLYGON};

int g_mode;
bool g_debug;

double linesize = 1;
bool isbig = false;
bool isSpline = true;
int drawn_strokes;

std::fstream fileToMax; 
std::fstream fileFromCamera;
std::fstream fileFromMax;
FILE *fromCamera;


// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

void draw_active_strokes();
void draw_finished_strokes();
//void draw_finished_polygons();

void draw_buttons();

void initialize_buttons();
void check_for_button_press();

void mouse_state(int, int, int, int);
void mouse_motion(int, int);

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
  
  if(args->human_paintbrush){
		glClearColor(1.0,1.0,1.0,1.0);
  }
  
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

  //gluOrtho2D(0, args->tiled_display.full_display_width, 0, args->tiled_display.full_display_height);
  args->tiled_display.ORTHO();
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();


	draw_finished_strokes();
	//draw_finished_polygons();
	if(!args->human_paintbrush){
		draw_buttons();
	}
	draw_active_strokes();

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
  for (unsigned int i = 0; i < buttons.size(); i++) {
		if(g_mode==POLYGON && i==(unsigned int)global_up_button_id)
			continue;
		else if(g_mode==POLYGON && i==(unsigned int)global_down_button_id)
			continue;
		else
			buttons[i].paint();
  }
  glColor3f(1,1,1);
  glLineWidth(global_current_width);
	double radius = global_current_width/2;
  glBegin(GL_QUADS);
  glVertex2d(sample_line_x-radius,sample_line_top);
  glVertex2d(sample_line_x-radius,sample_line_bottom);
  glVertex2d(sample_line_x+radius,sample_line_bottom);
  glVertex2d(sample_line_x+radius,sample_line_top);
  glEnd();
}

void draw_active_strokes() {
  assert (global_point_tracker != NULL);

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {

    if (!(*global_point_tracker)[i]->IsActive()) continue;

    // SET COLOR
    int id = (*global_point_tracker)[i]->getID();
    Vec3f color = global_colors.GetColor(id);
    glColor3f(color.x(),color.y(),color.z());

    // DRAW TRAIL
    glLineWidth(stroke_widths[id]);
    Path::draw_smooth_stroke((*global_point_tracker)[i]->getPtTrail(),stroke_widths[id]);

    //Pt pt = (*global_point_tracker)[i].getCurrentPosition();
    int laser = (*global_point_tracker)[i]->getWhichLaser();

    if (laser != -1) {
      assert (laser >= 0 && laser < MAX_LASERS);
      //      drawstring(pt.x,pt.y,laser_names[laser],color.x(),color.y(),color.z(),50,50,3.0);
    }
  }
}


void draw_finished_strokes() {
  for (unsigned int i = 0; i < global_finished_strokes.size(); i++) {
    global_finished_strokes[i].draw();
  }
}
//void draw_finished_polygons(){
//	for(unsigned int i = 0; i < global_finished_polygons.size(); i++)
//		global_finished_polygons[i].draw_polygon();
//}


void display(void) {
  draw();
}


// ==========================================================================================
// TRACKING ROUTINES
// ==========================================================================================

void set_active_stroke_colors() {

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    const TrackedPoint *pt = (*global_point_tracker)[i];
    int id = pt->getID();
    int which_laser = pt->getWhichLaser();
    //std::cout << "which laser = " << which_laser << std::endl;
    std::map<int,int>::iterator itr = selected_colors.find(which_laser);
    if (itr != selected_colors.end()) {
      global_colors.ReAssignColor(id,itr->second);
    }
  }
}


void AddTrackedPoint(TrackedPoint *pt) {
  int id = pt->getID();
  //std::cout << "ADD " << pt.getID() << std::endl;
  //int global_current_color = 0;

  if (global_current_color == -1) {
    std::cout<<"Assigning random color\n";
    global_colors.AssignRandomAvailableColor(id);
    while (global_colors.GetColorIndex(id) > 15 ) {
      std::cout << "regenerate " << global_colors.GetColorIndex(id) << std::endl;
      global_colors.ReleaseColor(id);
      global_colors.AssignRandomAvailableColor(id);
    }
    std::cout << "good " << global_colors.GetColorIndex(id) << std::endl;
  } else {
    std::cout<<"Assigning correct color\n";
    global_colors.AssignColor(id,global_current_color);
  }
  stroke_widths[id] = global_current_width;
}


void RemoveTrackedPoint(TrackedPoint *pt) {
  int id = pt->getID();

  Vec3f color = global_colors.GetColor(id);

  if (global_button_strokes.find(id) != global_button_strokes.end()) {
    global_button_strokes.erase(global_button_strokes.find(id));
  } else {
		//if(g_mode==LINE)
			global_finished_strokes.push_back(Path(pt->getPtTrail(),color,stroke_widths[id]));
		//else
			//global_finished_polygons.push_back(NonConvexPolygon(pt.getPtTrail(),color));
  }

  stroke_widths.erase(stroke_widths.find(id));

  //std::cout << "ERASE " << pt.getID() << std::endl;
  global_colors.RemoveId(id);

}

// ==========================================================================================
// IDLE
// ==========================================================================================

void idle(void) {

	if(!args->human_paintbrush){
		// look for new IR data
		while (!global_dirlock.TryLock()) {
			usleep(1000);
		}
		std::vector<IR_Data_Point> raw_points;    
		bool success;
		{ /* SCOPE FOR istr */
			std::ifstream istr(FOUND_IR_POINTS_FILENAME);
			//assert (istr);
			success = PointTracker::ReadIRPointData(istr,raw_points)||(mouse_location.x >= 0 && mouse_location.y >= 0);
		} /* SCOPE FOR istr */
		global_dirlock.Unlock();
		if (!success) {
			// new data not yet available
			usleep(1000);
			return;
		}

		if(mouse_location.x >= 0 && mouse_location.y >= 0){
			std::vector<Pt> all_mouse_pts;
			Pt temp;
			temp.x = mouse_location.x;
			temp.y = args->tiled_display.full_display_height - mouse_location.y;
			all_mouse_pts.push_back(temp);
			global_point_tracker->ProcessPointData(raw_points,all_mouse_pts);
		}
		else{
			global_point_tracker->ProcessPointData(raw_points);
		}
	}
	
	else{
		//Tyler: idle function code for the human paintbrush project
		//Send Position data to text file, for MaxMSP application
		//std::vector<IR_Data_Point> raw_points;
		
		
		
		//using mouse input coordinates
		// if(mouse_location.x >= 0 && mouse_location.y >= 0 && 0){
		
			// if(isbig){
				// linesize += .2;
				// if(linesize > 30) isbig = false;
			// }else{
				// linesize = 1;
				// isbig = true;
			// }
			
			// fileFromCamera.open("../../../paint/PositionToPaint.txt");
			// std::cout << linesize << " ";
			// Pt temp;
			// temp.x = mouse_location.x;
			// temp.y = args->height - mouse_location.y;
			// fileFromCamera >> temp.x;
			// fileFromCamera >> temp.y;
			// fileFromCamera >> linesize;
			// fileFromCamera.close();
			
			// mouse_pts.push_back(temp);
			// if(mouse_pts.size() > 3 && isSpline ){
				// mouse_pts.pop_front();
			// }
			// std::cout << linesize << " " << temp.y << " " << temp.x << std::endl;
			
			// if(linesize < 0) linesize = 0;
			// if(linesize > 20) linesize = 20;
			
			// send (x,y) mouse coords to txt file for Max
			// fileToMax.open("PositionToMax.txt");
			// fileToMax << (int)temp.x << std::endl;
			// fileToMax << (int)temp.y << std::endl;
			// fileToMax.close();
			
			// draw the new stroke
			// Path strk = Path(mouse_pts,Vec3f(0.0,(linesize/60.0),(linesize/20.0)),linesize);
			// global_finished_strokes.push_back(strk);
			
		// }else{
			
			
			fileFromCamera.open("PositionToPaint.txt");
			if(!fileFromCamera.fail()){
				//std::cout << "couldn't open file" << std::endl;
			}
			Pt temp;
			std::string x, y, l;
			fileFromCamera >> x;
			temp.x = atoi(x.c_str());
			temp.x *= args->tiled_display.full_display_width / 600.0;
			fileFromCamera >> y;
			temp.y = atoi(y.c_str());
			temp.y *= args->tiled_display.full_display_height / 600.0;
			fileFromCamera >> l;
			linesize = atoi(l.c_str());
			fileFromCamera.close();
			
			if(temp.x == args->tiled_display.full_display_width || temp.x == 0 || temp.y == args->tiled_display.full_display_height || temp.y == 0){
				return;
			}
			
			// fromCamera = fopen("./PositionToPaint.txt", "r");
			// if (fromCamera == NULL){
				// perror ("Error opening file");
			// }
			// Pt temp;
			// char* x = new char[15];
			// char* y = new char[15];
			// char* l = new char[15];
			// fileFromCamera.getline(x,15);
			// temp.x = atoi(x);
			// fileFromCamera.getline(y,15);
			// temp.y = atoi(y);
			// fileFromCamera.getline(l,15);
			// linesize = atoi(l);
			// fgets (x, 15, fromCamera);
			// temp.x = atoi(x);
			// fgets (y, 15, fromCamera);
			// temp.y = atoi(y);
			// fgets (l, 15, fromCamera);
			// linesize = atoi(l);
			// fclose(fromCamera);
			
			//temp.x = temp.x * (600.0/1920.0);
			//temp.y = temp.y * (600.0/1200.0);
			fileFromMax.open("ColorToPaint.txt");
			std::string rstr, gstr, bstr;
			fileFromMax >> rstr;
			fileFromMax >> gstr;
			fileFromMax >> bstr;
			double r, g, b;
			r = atof(rstr.c_str());
			g = atof(gstr.c_str());
			b = atof(bstr.c_str());
			fileFromMax.close();
			
			mouse_pts.push_back(temp);
			if(mouse_pts.size() > 3 && isSpline ){
				mouse_pts.pop_front();
			}
			//std::cout << temp.x << " " << temp.y << " " << linesize << std::endl;
			
			if(linesize < 1) linesize = 1;
			if(linesize > 70) linesize = 70;

			usleep(3000);
			
			
			//draw the new stroke
			Path strk = Path(mouse_pts,Vec3f(((linesize*r)/20.0),((linesize*g)/20.0),((linesize*b)/20.0)),linesize);
			global_finished_strokes.push_back(strk);
			
			
		//}
	}
	
	if(!args->human_paintbrush){
	check_for_button_press();
  }

  set_active_stroke_colors();

  display();

  usleep(5000);
}

void release_all_color_buttons() {
  for (unsigned int j = 1; j <= 18; j++) {
    Button &b2 = buttons[j];
    b2.release();
  }
}


#define HOVER_THRESHOLD 10

void write_all_color_button_text() {
  for (int i = 0; i < 18; i++) {
    Button &b2 = buttons[i+1];
    std::string foo = "";
    for (std::map<int,int>::iterator itr = selected_colors.begin();
	 itr != selected_colors.end(); itr++) {
      if (itr->second == i) {
	if (foo != "") foo += " ";
	assert (itr->first >= 0 && itr->first < MAX_LASERS);
	foo += PlanarCalibration::laser_names[itr->first];
      }
    }
    b2.clearText();
    b2.addText(foo);    
  }
}

void check_for_button_press() {

  timeval now;
  gettimeofday(&now,NULL);

  // clear any buttons that are on timer:
  Button &undo_button = buttons[global_undo_button_id];
  undo_button.release_if_time_expired(now);
  Button &clear_button = buttons[global_clear_button_id];
  clear_button.release_if_time_expired(now);
  Button &up_button = buttons[global_up_button_id];
  up_button.release_if_time_expired(now);
  Button &down_button = buttons[global_down_button_id];
  down_button.release_if_time_expired(now);

  // check to see if any tracked points are in the button zone long enough
  Button &button_zone = buttons[0];
  
  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    if (!(*global_point_tracker)[i]->IsActive()) continue;

    Pt p = (*global_point_tracker)[i]->getCurrentPosition();

    // check to see if the point is currently in button zone...
    if (button_zone.PointInside(p)) {

      int id = (*global_point_tracker)[i]->getID();
      int which_laser = (*global_point_tracker)[i]->getWhichLaser();

      const std::list<Pt> &trail = (*global_point_tracker)[i]->getPtTrail();      
      if (button_zone.Hover(trail,HOVER_THRESHOLD)) {
	//std::cout << "HOVER" << std::endl;
	global_colors.ReAssignColor(id,0);  // white
	(*global_point_tracker)[i]->setLaserTailLength((int)(HOVER_THRESHOLD*1.1));
	stroke_widths[id] = 1;
	global_button_strokes.insert(id);
      }

      

      // check to see if a color button is pressed
      for (unsigned int j = 0; j < 18; j++) {
	Button &b2 = buttons[j+1];
	if (b2.Hover(trail,HOVER_THRESHOLD)) {
	  release_all_color_buttons();
	  global_current_color = j;
	  Button &random_button = buttons[global_random_button_id];
	  random_button.release();
	  if (which_laser != -1) {
	    assert(which_laser >= 0 && which_laser < MAX_LASERS);
	    selected_colors[which_laser] = j;
	  }
	  write_all_color_button_text();
	  	  b2.press();
	  break;
	}
      } 
      Button &random_button = buttons[global_random_button_id];
      if (random_button.Hover(trail,HOVER_THRESHOLD)) {
	release_all_color_buttons();
	//random_button.press();
	global_current_color = -1;

	if (which_laser != -1) {
	  //std::cout << "want to clear " << which_laser << std::endl;
	  std::map<int,int>::iterator itr = selected_colors.find(which_laser);
	  if (itr != selected_colors.end()) {
	    selected_colors.erase(itr);
	  } else {
	    //std::cout << " not selected" << std::endl;
	  }
	  write_all_color_button_text();
	}
      }

      if (g_mode==LINE && down_button.Hover(trail,HOVER_THRESHOLD)) {
	if (down_button.isPressed() == false) {
	  down_button.press(200);
	  global_current_width--;
	  if (global_current_width < 1) global_current_width = 1;
	  std::cout << "down width = " << global_current_width << std::endl;
	}
      }

      if (g_mode==LINE && up_button.Hover(trail,HOVER_THRESHOLD)) {
				if (up_button.isPressed() == false) {
					up_button.press(200);
					global_current_width+=2;
					//GLint width_range[2];
					//GLint *foo = &width_range[0];
					//glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE,foo);
					if (global_current_width > MAX_WIDTH) global_current_width = MAX_WIDTH;
					std::cout << "up width = " << global_current_width << std::endl;

				}
      }

      if (undo_button.Hover(trail,HOVER_THRESHOLD)) {
	if (!undo_button.isPressed()) {
	  undo_button.press(500);
	  if (global_finished_strokes.size() > 0)
	    global_finished_strokes.pop_back();
	}
      }

      if (clear_button.Hover(trail,HOVER_THRESHOLD)) {
	clear_button.press(500);
	global_finished_strokes.clear();
	//global_finished_polygons.clear();
      }

    }    
  }
}


void reshape(int w, int h) {
  //w=1024;
  //h=768;
  //args->width = w;
  //args->height = h;

  args->tiled_display.reshape(w,h);

  glViewport(0,0,w,h);

  glMatrixMode(GL_PROJECTION);
  args->tiled_display.ORTHO2();
  //gluPerspective(40.0, 1.0, 1.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
  //gluLookAt(0.0, 0.0, 5.0,
  //	    0.0, 0.0, 0.0,     
  //	    0.0, 1.0, 0.); 
  //glTranslatef(0.0, 0.6, -1.0);

  //  initialize_buttons();
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 'q')
    exit(0);
	else if (key=='p'||key=='P'){
		g_mode = POLYGON;
		global_current_width = POLYGON_DEFAULT_WIDTH;
	}
	else if (key=='l'||key=='L')
		g_mode= LINE;
	else if (key=='c'||key=='C'){
		global_finished_strokes.clear();
		//global_finished_polygons.clear();
	}
	else if (key=='d'||key=='D')
		g_debug = !g_debug;

	else
    std::cout << "WARNING unknown key: '" << key << "'" << std::endl;

  display();
  //  glutPostRedisplay();
}

// ===================================================================
// Mouse Functions
// ===================================================================

void mouse_state(int button, int state, int x, int y){
	if(button != GLUT_LEFT_BUTTON)
		return;
	
	if(state==GLUT_DOWN){
		mouse_location.x = x;
		mouse_location.y = y;
	}
	else if (state==GLUT_UP){
		mouse_location.x = -1;
		mouse_location.y = -1;
	}
}

void mouse_motion(int x, int y){
	if(mouse_location.x != -1 && mouse_location.y != -1){
		mouse_location.y = y;
		mouse_location.x = x;
	}
}
// ===================================================================

#define SPACING 15

void initialize_buttons() {

  std::vector<Vec3f> colors;
  colors.push_back(Vec3f(1,1,1));  // 0
  colors.push_back(Vec3f(1,0.5,0.5));
  colors.push_back(Vec3f(1,0,0));
  colors.push_back(Vec3f(1,0.5,0));
  colors.push_back(Vec3f(1,1,0));
  colors.push_back(Vec3f(0,1,0));
  colors.push_back(Vec3f(0,0.4,0));
  colors.push_back(Vec3f(0,1,0.5));
  colors.push_back(Vec3f(0,1,1));
  colors.push_back(Vec3f(0,0.5,1));
  colors.push_back(Vec3f(0,0,1)); // 10
  colors.push_back(Vec3f(0.5,0,1));
  colors.push_back(Vec3f(1,0,1));
  colors.push_back(Vec3f(1,0,0.5));
  colors.push_back(Vec3f(0.5,0.4,0.3));
  colors.push_back(Vec3f(0.3,0.2,0.1));
  colors.push_back(Vec3f(0.3,0.3,0.3));
  colors.push_back(Vec3f(0.1,0.1,0.1));

  global_colors = Colors(colors);
  buttons.clear();
  
  double left = args->tiled_display.full_display_width*0.8;
  double w = args->tiled_display.full_display_width-left;

  buttons.push_back(Button(Pt(left,0),w, args->tiled_display.full_display_height,Vec3f(0,0,0)));
  
  left = left + SPACING;
  double full_w = w-2*SPACING;
  double half_w = (full_w-SPACING)/2;

  double third_w = (full_w-2*SPACING)/3;

  double h = (args->tiled_display.full_display_height-SPACING) / 9.0 - SPACING;
  double top = args->tiled_display.full_display_height - SPACING - h;

  buttons.push_back(Button(Pt(left,top),third_w,h,colors[0])); 
  buttons.push_back(Button(Pt(left+third_w+SPACING,top),third_w,h,colors[1])); 
  buttons.push_back(Button(Pt(left+2*third_w+2*SPACING,top),third_w,h,colors[2]));  top -= h+SPACING;

  buttons.push_back(Button(Pt(left,top),third_w,h,colors[3])); 
  buttons.push_back(Button(Pt(left+third_w+SPACING,top),third_w,h,colors[4])); 
  buttons.push_back(Button(Pt(left+2*third_w+2*SPACING,top),third_w,h,colors[5]));  top -= h+SPACING;

  buttons.push_back(Button(Pt(left,top),third_w,h,colors[6])); 
  buttons.push_back(Button(Pt(left+third_w+SPACING,top),third_w,h,colors[7])); 
  buttons.push_back(Button(Pt(left+2*third_w+2*SPACING,top),third_w,h,colors[8]));  top -= h+SPACING;

  buttons.push_back(Button(Pt(left,top),third_w,h,colors[9])); 
  buttons.push_back(Button(Pt(left+third_w+SPACING,top),third_w,h,colors[10])); 
  buttons.push_back(Button(Pt(left+2*third_w+2*SPACING,top),third_w,h,colors[11]));  top -= h+SPACING;

  buttons.push_back(Button(Pt(left,top),third_w,h,colors[12])); 
  buttons.push_back(Button(Pt(left+third_w+SPACING,top),third_w,h,colors[13])); 
  buttons.push_back(Button(Pt(left+2*third_w+2*SPACING,top),third_w,h,colors[14]));  top -= h+SPACING;

  buttons.push_back(Button(Pt(left,top),third_w,h,colors[15])); 
  buttons.push_back(Button(Pt(left+third_w+SPACING,top),third_w,h,colors[16])); 
  buttons.push_back(Button(Pt(left+2*third_w+2*SPACING,top),third_w,h,colors[17]));  top -= h+SPACING;


  buttons.push_back(Button(Pt(left,top),full_w,h,Vec3f(0.5,0.5,0.5),"RANDOM"));  top -= h+SPACING;
  global_random_button_id = buttons.size()-1;
  //Button &random_button = buttons[global_random_button_id];
  //random_button.press();
  global_current_color = -1;

  sample_line_x = left + half_w + 0.5*SPACING;
  sample_line_top = top+h;
  sample_line_bottom = top;
	buttons.push_back(Button(Pt(left,top),half_w/2-SPACING,h,Vec3f(0.5,0.5,0.5),"down")); 
	//buttons.push_back(Button(Pt(0,0),half_w-SPACING,h,Vec3f(0.5,0.5,0.5),"down")); 
	global_down_button_id = buttons.size()-1;
	buttons.push_back(Button(Pt(left+half_w+half_w/2+2*SPACING,top),half_w/2-SPACING,h,Vec3f(0.5,0.5,0.5),"up"));  top -= h+SPACING;
	global_up_button_id = buttons.size()-1;



  buttons.push_back(Button(Pt(left,top),half_w,h,Vec3f(0.5,0.5,0.5),"UNDO")); 
  global_undo_button_id = buttons.size()-1;

  buttons.push_back(Button(Pt(left+half_w+SPACING,top),half_w,h,Vec3f(0.5,0.5,0.5),"CLEAR"));
  global_clear_button_id = buttons.size()-1;

}

int main(int argc, char **argv) {

	

  args = new PaintArgParser(argc,argv);
  global_point_tracker = NULL;

  global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);

  /*
    if(!args->human_paintbrush){
		args->width = global_calibration_data->getWidth();
		args->height = global_calibration_data->getHeight();
	}
  */

  args->tiled_display.set_from_calibration_data(global_calibration_data);

  // initialize things...
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);
	
	if(!args->human_paintbrush){
		glutInitWindowPosition(20,20);
	}else{
		glutInitWindowPosition(1100-300,475-300);
	}
  glutCreateWindow("planar calibration");

  initialize_buttons();

	mouse_location.x = -1;
	mouse_location.y = -1;

	std::list<Pt> test_points;
	//test_points.push_back(Pt(100,100));
	//test_points.push_back(Pt(100,500));
	//test_points.push_back(Pt(300,400));
	//test_points.push_back(Pt(500,500));
	//test_points.push_back(Pt(500,100));
	//NonConvexPolygon p;
	//p.make_test();
	////global_finished_polygons.push_back(NonConvexPolygon(test_points));
	//global_finished_polygons.push_back(p);

	//set the default draw mode to line
	g_mode = LINE;

  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse_state);
	glutMotionFunc(mouse_motion);

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
  //  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(2.0);

  //glEnable(GL_POLYGON_SMOOTH);
  
  HandleGLError("main 1");

  if (args->tiled_display.full_screen) {
    glutFullScreen();
  } 
  
  //reshape(args->width,args->height);
  
  global_point_tracker = new PointTracker(global_calibration_data,
					  AddTrackedPoint,RemoveTrackedPoint,-1,0.5, false);

	/*NonConvexPolygon p;
	p.add_point(Pt(100,100));
	p.add_point(Pt(100,400));
	p.add_point(Pt(400,400));
	p.add_point(Pt(400,100));
	p.set_color(Vec3f(1.0,0,0));
	global_finished_polygons.push_back(p);
	global_finished_strokes.push_back(Path(test_stroke,Vec3f(0,0,255),5));*/
	
	global_current_width = DEFAULT_WIDTH;
  

  glutMainLoop();

  return 0;
}
