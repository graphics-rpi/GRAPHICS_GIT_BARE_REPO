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

#include <list>
#include <vector>
#include <fstream>
#include <iostream>

#include "../../common/directory_locking.h"
#include "argparser.h"
#include "planar_calibration.h"
#include "tracker.h"
#include "../../../../remesher/src/vectors.h"
#include "colors.h"

#include "../../applications/paint/text.h"
#include "../../multi_cursor/key_and_mouse_logger.h"

#define SHIFT 200


#define DESIRED_BORDER 70

// note, the grid is +1 in each dimension
//##define DESIRED_X_CELLS 7
//#define DESIRED_X_CELLS 9
#define DESIRED_X_CELLS 3
#define DESIRED_Y_CELLS 3
//#define DESIRED_Y_CELLS 2

#define IR_STATE_DIRECTORY                   "../state/ir_tracking"
#define FOUND_IR_POINTS_FILENAME             "../state/ir_tracking/found_ir_points.txt"
#define MK_STATE_DIRECTORY                   "../state/mouse_and_keyboard"
#define MK_ACTION_FILENAME_TEMPLATE          "../state/mouse_and_keyboard/actions_XXX.txt"

#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"
// ADDED BY CHRIS STUETZLE
#define PLANAR_CALIBRATION_GNU_PLOT_INTENSITY_DATA_FILE_PREFIX "../state/ir_tracking/"

#define APPLICATIONS_STATE_DIRECTORY            "../state/applications/"
#define CALIBRATION_STATE_FILENAME              "../state/applications/calibration_state.txt"

int GLOBAL_laser_timer_count = 0;
#define GLOBAL_laser_min_points_valid 20
#define GLOBAL_laser_max_timer 30
bool GLOBAL_laser_collection = false;

extern std::ofstream *tracking_logfile;

int which_lap = 0;

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args;

// for collecting calibration data
int global_collect_index=0;

// for collecting & testing the calibration data
DirLock global_ir_dirlock(IR_STATE_DIRECTORY);
DirLock global_mk_dirlock(MK_STATE_DIRECTORY);
DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);
PlanarCalibration *global_calibration_data;

// for testing the calibration
PointTracker *global_point_tracker;
Colors global_colors;

// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================
void draw_grid();
void draw_tracked_points();
void draw_debug_points();
void draw_intensity_samples();
void draw_histogram(const IR_Interpolable_Data &tmp);

void keypress_function(int which_keyboard, unsigned char key, int x, int y, int glutmodifiers);
void specialkeypress_function(int which_keyboard, int key, int x, int y, int glutmodifiers)  { /* do nothing */ }
void mouseaction_function(int which_mouse, int button, int state, int x, int y, int glutmodifiers) { /* do nothing */ }
void mousemotion_function(int which_mouse, int x, int y, int glutmodifiers) { /* do nothing */ }

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


void clamp_to_display(Pt &pt) { 
  int x = pt.x;
  int y = pt.y;
  x = std::max(0,std::min(x,args->tiled_display.full_display_width));
  y = std::max(0,std::min(y,args->tiled_display.full_display_height));
  pt = Pt(x,y);
}


// ==========================================================================================
// DRAWING ROUTINES
// ==========================================================================================

void draw() { 

  static GLfloat amb[] =  {0.4, 0.4, 0.4, 0.0};
  static GLfloat dif[] =  {1.0, 1.0, 1.0, 0.0};

  float s = 0.0;

  if (args->visualize_intensity) {
    glClearColor(1.0,1.0,1.0,1.0);
  } else {
    glClearColor(0.0,0.0,0.0,0.0);
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

  gluOrtho2D(0, args->tiled_display.my_width, 0, args->tiled_display.my_height);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if (/*args->test_data || */args->collect_geometry || args->collect_intensity) {
    if (global_calibration_data != NULL) {
      draw_grid();
    }
  }
  
  
  if (args->all_black) {
    // JUST BLACK!
  } else if (args->test_data) {
    //draw_debug_points();

    if (args->visualize_intensity) {
      //assert (0);


      draw_intensity_samples();

    }

    draw_tracked_points();


  } else if (args->collect_geometry) {
    if (global_calibration_data != NULL) {
      if (GLOBAL_laser_collection == true) {
	glColor3f(0,0,1);
      } else {
	glColor3f(0,1,0);
      }
      glPointSize(30);
      glBegin(GL_POINTS);

      int i,j;
      global_calibration_data->index_to_grid(global_collect_index,i,j);

      Pt p = global_calibration_data->calibration_point(IntPt(i,j)); 
      glVertex2f(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom); 
      glEnd();
    } 
  } else if (args->collect_intensity) {
    if (global_calibration_data != NULL) {
      

      for (int offset = 0; offset < MAX_LASERS; offset++)  {
	
	int index = (global_collect_index + offset) % (global_calibration_data->getGridX() * global_calibration_data->getGridY());
	int i,j;
	global_calibration_data->index_to_grid(index,i,j);      
	
	Pt p = global_calibration_data->calibration_point(IntPt(i,j));
	
	if (GLOBAL_laser_collection == true) {
	  drawstring(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom,PlanarCalibration::laser_names[offset].c_str(),Vec3f(0,0,1),100,100);
	} else {
	  drawstring(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom,PlanarCalibration::laser_names[offset].c_str(),Vec3f(0,1,0),100,100);
	}
      }
    } 
  }
  
  //  glDisable(GL_LINE_SMOOTH);
  //  glDisable(GL_BLEND);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);

  glutSwapBuffers();

}


void draw_tracked_points() {

  assert (global_point_tracker != NULL);

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {

    if (!(*global_point_tracker)[i]->IsActive()) continue;

    // SET COLOR
    int id = (*global_point_tracker)[i]->getID();
    Vec3f color = global_colors.GetColor(id);

    int laser = (*global_point_tracker)[i]->getWhichLaser();
    if (laser != -1) {
      color = PlanarCalibration::fixed_laser_colors[laser];
      //std::cout << "laser " << laser+1 << " color: " << color.x() << " " << color.y() << " " << color.z() << std::endl;
    }

    if (!args->visualize_intensity) {        
      // DRAW TRIANGLE
      const IntTri& tri = (*global_point_tracker)[i]->getCurrentTriangle();
      glBegin(GL_TRIANGLES);
      Pt a = global_calibration_data->calibration_point(tri.pts[0]);
      Pt b = global_calibration_data->calibration_point(tri.pts[1]);
      Pt c = global_calibration_data->calibration_point(tri.pts[2]);
      glColor3f(color.x()*0.5,color.y()*0.5,color.z()*0.5);
      glVertex3f(a.x-args->tiled_display.my_left,a.y-args->tiled_display.my_bottom,0);
      glVertex3f(b.x-args->tiled_display.my_left,b.y-args->tiled_display.my_bottom,0);
      glVertex3f(c.x-args->tiled_display.my_left,c.y-args->tiled_display.my_bottom,0);
      glEnd();
    }
    
    // DRAW POINT
    glColor3f(color.x(),color.y(),color.z());

    const Pt& pt = (*global_point_tracker)[i]->getCurrentPosition();
    if (laser == -1) {
      drawstring(pt.x-args->tiled_display.my_left,pt.y-args->tiled_display.my_bottom,"-1",color,100,100);
    } else {
      assert (laser >= 0 && laser < MAX_LASERS);
      drawstring(pt.x-args->tiled_display.my_left,pt.y-args->tiled_display.my_bottom,PlanarCalibration::laser_names[laser].c_str(),color,100,100);
    }

    if (args->visualize_intensity) {
      //double b,r;
      IR_Interpolable_Data interpolable_data;
      unsigned long ip;

      (*global_point_tracker)[i]->getIntensity(interpolable_data,ip);

      glLineWidth(10);
      draw_histogram(interpolable_data);

    }
    

    // DRAW TRAIL
    glLineWidth(3);
    glBegin(GL_LINES);
    const std::list<Pt> &trail = (*global_point_tracker)[i]->getPtTrail();
    std::list<Pt>::const_iterator itr = trail.begin();
    while (itr != trail.end()) {
      double x = itr->x;
      double y = itr->y;
      itr++;
      if (itr == trail.end()) break;
      glVertex2f(x-args->tiled_display.my_left,y-args->tiled_display.my_bottom); //      glVertex2f(x,y);
      x = itr->x;
      y = itr->y;
      glVertex2f(x-args->tiled_display.my_left,y-args->tiled_display.my_bottom); //      glVertex2f(x,y);
    }
    glEnd();
    
  }
}



/*
void draw_debug_points() {

  std::vector<std::vector<double> > std::vector<std::vector<double> >(1,std::vector<double>(MAX_LASERS,-1)); //intensity_differences(MAX_LASERS);
  glPointSize(5);
  glBegin(GL_POINTS);

#if 0
  for (double b = 0.1; b < 4.2; b += 0.02) {
    for (double r = 0.1; r < 3; r += 0.02) {
#else
  for (double b = 0.1; b < 4.2; b += 0.03) {
    for (double r = 0.1; r < 3; r += 0.03) {
#endif

      unsigned long ip = global_calibration_data->all_my_data[0].ip;

      Vec3f color = Vec3f(0.1,0.1,0.1); 
      global_calibration_data->MatchToLaserProfiles(b,r,ip,intensity_differences,false);

      int laser = -1;
      double best = -1;

      for (int j = 0; j < MAX_LASERS; j++) {
	if (intensity_differences[j] > 0 
	    && intensity_differences[j] < 1.5 
	    && (best < 0 || best > intensity_differences[j])) {
	  laser = j;
	  best = intensity_differences[j];
	}
      }



      if (laser != -1) {
	color = fixed_laser_colors[laser];
      }

      glColor3f(color.x(),color.y(),color.z());
      
      double b2 = b;
      double r2 = r;

      global_calibration_data->overall_scale(b2,r2);
      glVertex3f(b2*args->width,r2*args->height,0);      
 
    }
  }
  glEnd();
}
*/


void draw_histogram(const IR_Interpolable_Data &tmp) {

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);

  int height = 1000; //args->tiled_display.full_display_height;
  //height = 768;
  int width = 1600; //args->tiled_display.full_display_width;
  // width = 1024;
  
  int border = 10;

  glBegin(GL_LINE_STRIP);
  int numbins = tmp.numBins();
  for (int i = 0; i < numbins; i++) {
    float b = tmp.getHistogramIntensity(i);
    if (b == -1) continue;
    //std::cout << "b : " << b << std::endl;
    assert (b >= 0 && b <= 255);
    glVertex3f(border + i * (width-2*border) / double (numbins),
	       border + b * (height-2*border) / 255.0,0);
  }
  glEnd();

}


void draw_intensity_samples() {
  
  int grid_x = global_calibration_data->getGridX();
  int grid_y = global_calibration_data->getGridY();



  for (unsigned int c = 0; c < global_calibration_data->all_my_data.size(); c++) {
    //    unsigned long ip = global_calibration_data->all_my_data[c].ip;

    for (int laser = 0; laser < MAX_LASERS; laser++) {

      Vec3f color = PlanarCalibration::fixed_laser_colors[laser];
      color = color*0.25 + Vec3f(0.75,0.75,0.75);
      glColor3f(color.r(),color.g(),color.b());
      

      // draw all the samples first
      glLineWidth(3);
      for (int index = 0; index < grid_x*grid_y; index++) {
	if (!global_calibration_data->all_my_data[c].intensity_data[index][laser].valid()) {
	  continue;
	}
	IR_Interpolable_Data tmp = global_calibration_data->all_my_data[c].intensity_data[index][laser].get_average();
	assert (tmp.valid());
	//global_calibration_data->NormalizeIntensityData(index,laser,tmp); 
	draw_histogram(tmp);
      }

      // draw the average 
      glLineWidth(2);


      color = PlanarCalibration::fixed_laser_colors[laser];
      glColor3f(color.r(),color.g(),color.b());

      IR_Interpolable_Data tmp = global_calibration_data->all_my_data[c].laser_interpolable_data[laser];
      if (tmp.valid()) {
	draw_histogram(tmp);
      }
    }
  }
}

 
void draw_grid() {
  assert (global_calibration_data != NULL);
  glColor3f(1,1,1);
  glLineWidth(1);
  glBegin(GL_LINES);
  int max_i = global_calibration_data->getGridX();
  int max_j = global_calibration_data->getGridY();
  for (int i = 0; i < max_i; i++) {
    for (int j = 0; j < max_j; j++) {
      if (i+1 < max_i) {
	Pt p = global_calibration_data->calibration_point(IntPt(i,j));
	glVertex2f(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom); 
	//glVertex2f(p.x,p.y); 
	p = global_calibration_data->calibration_point(IntPt(i+1,j));
	glVertex2f(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom); 
	//glVertex2f(p.x,p.y); 
      }
      if (j+1 < max_j) {
	Pt p = global_calibration_data->calibration_point(IntPt(i,j));
	glVertex2f(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom); 
	//glVertex2f(p.x,p.y); 
	p = global_calibration_data->calibration_point(IntPt(i,j+1));
	glVertex2f(p.x-args->tiled_display.my_left,p.y-args->tiled_display.my_bottom); 
	//glVertex2f(p.x,p.y); 
      }
    }
  }
  glEnd();
}


void display(void) {
  draw();
}


// ==========================================================================================
// COLLECT DATA ROUTINES
// ==========================================================================================

int ClosestLaser(double x, double y, unsigned long ip) {
  int index = global_calibration_data->ClosestGridPoint(x,y,ip);
  
  if (index == -1) return -1;

  int num_grid_points = global_calibration_data->getGridX()*global_calibration_data->getGridY();

  assert (index >= 0 && index < num_grid_points);

  int offset = (index-global_collect_index+2*num_grid_points)%num_grid_points;

  assert (offset >= 0 && offset < MAX_LASERS);

  //std::cout << "offset! " << offset << std::endl;

  //std::cout << "LASER " << offset+1 << "\n";
  
  return offset;
}
 
class SUMMER {
public:
  unsigned long ip;
  double sum_x;
  double sum_y;
  int count;
};
 

void collect_calibration_data_point() {
  while (!global_ir_dirlock.TryLock()) {
    usleep(1000);
  }

  assert (GLOBAL_laser_collection == true);

  //===================================================
  // READ THE IR DATA
  std::vector<IR_Data_Point> raw_points;    
  bool success;
  { /* SCOPE FOR istr */
    std::ifstream istr(FOUND_IR_POINTS_FILENAME);
    assert (istr);
    success = PointTracker::ReadIRPointData(istr,raw_points);
  } /* SCOPE FOR istr */
  global_ir_dirlock.Unlock();
  if (!success) {
    //std::cout << "ERROR: no new IR data available" << std::endl;
    usleep(500);
    return;
  }

  //===================================================
  // READ THE IR DATA
  
  bool done = false;
  
  GLOBAL_laser_timer_count++;
  if (args->collect_geometry) {
    static std::vector<SUMMER> summers;
    // process the points for this frame
    if (raw_points.size() > 0) {
      for (unsigned int A = 0; A < raw_points.size(); A++) {
	for (unsigned int A2 = 0; A2 < A; A2++) {
	  if (raw_points[A].ip() == raw_points[A2].ip()) break;
	}
	int which_summer = -1;
	for (unsigned int S = 0; S < summers.size(); S++) {
	  if (summers[S].ip == raw_points[A].ip()) which_summer = S;
	}
	if (which_summer == -1) {
	  which_summer = summers.size();
	  summers.push_back(SUMMER());
	  summers[which_summer].ip = raw_points[A].ip();
	}
	summers[which_summer].sum_x += raw_points[A].x();
	summers[which_summer].sum_y += raw_points[A].y();
	summers[which_summer].count++;
      }
    }

    if (GLOBAL_laser_timer_count == GLOBAL_laser_max_timer) {
      for (unsigned int S = 0; S < summers.size(); S++) {
	if (summers[S].count >= GLOBAL_laser_min_points_valid) {
	  summers[S].sum_x /= double(summers[S].count);
	  summers[S].sum_y /= double(summers[S].count);
	  std::cout << "set " << summers[S].count << " " << summers[S].sum_x << " " << summers[S].sum_y << " " << std::endl;
	  global_calibration_data->setGeometryData(global_collect_index,Pt(summers[S].sum_x,summers[S].sum_y),summers[S].ip);
	} 
      }
      GLOBAL_laser_collection = false;
      summers.clear();
      done = true;
    }

  } else {
    assert (args->collect_intensity);
      
    for (unsigned int i = 0; i < raw_points.size(); i++) {
      int which_laser = ClosestLaser(raw_points[i].x(),raw_points[i].y(),raw_points[i].ip());
      if (which_laser == -1) { continue; }
      
      int whichindex = (global_collect_index + which_laser) % 
	(global_calibration_data->getGridX() * global_calibration_data->getGridY());
      
      global_calibration_data->addIntensityData(whichindex,which_laser,
 						//raw_points[i].brightness(),
						//,raw_points[i].radius,
						raw_points[i].ip(),
						raw_points[i].interpolable_data());
    }
    
    if (GLOBAL_laser_timer_count == GLOBAL_laser_max_timer) {
      GLOBAL_laser_collection = false;
      done = true;
    }
  }

  if (done) {
    if (!args->tiled_display.is_tiled_display || args->tiled_display.is_master) {
      global_collect_index++;
      if (global_collect_index == global_calibration_data->getGridX() * global_calibration_data->getGridY()) {
	if (which_lap > 0) {
	  global_collect_index = 0;
	  which_lap--;	
	  std::cout << "INCR WHICH LAP" << std::endl;
	} else {
	  std::cout << "ALL DONE!" << std::endl;
	  if (args->collect_geometry) {
	    std::cout << " save geometry " << std::endl;
	    global_calibration_data->write_geometry_to_file(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME);
	    global_calibration_data->write_intensity_to_file(PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);
	    // CHRIS STUETZLE
	    //    global_calibration_data->writeOutGNUPlotFile_DiffLasers(PLANAR_CALIBRATION_GNU_PLOT_INTENSITY_DATA_FILE_PREFIX);
	  } else {
	    assert (args->collect_intensity);
	    global_calibration_data->write_intensity_to_file(PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);	
	    // CHRIS STUETZLE
	    //  global_calibration_data->writeOutGNUPlotDataFiles(PLANAR_CALIBRATION_GNU_PLOT_INTENSITY_DATA_FILE_PREFIX);
	  }
	  exit(0);
	}
      }
    }
  }
}


// ==========================================================================================
// TEST CALIBRATION ROUTINES
// ==========================================================================================

void AddTrackedPoint(TrackedPoint* pt) {
  int id = pt->getID();
  //std::cout << "ADD " << pt.getID() << std::endl;
  global_colors.AssignRandomAvailableColor(id);
}


void RemoveTrackedPoint(TrackedPoint* pt) {
  int id = pt->getID();
  //std::cout << "ERASE " << pt.getID() << std::endl;
  global_colors.RemoveId(id);
}



// ==========================================================================================
// IDLE
// ==========================================================================================

void SaveState() {
  //std::cout << "TRY TO SAVE" << std::endl;
  static unsigned int frame_counter = 0;
  frame_counter++;
  while (!global_app_dirlock.TryLock()) { usleep(1000); }
  { /* SCOPE FOR ostr */
    std::ofstream ostr(CALIBRATION_STATE_FILENAME);
    assert (ostr);
    ostr << "frame " << frame_counter << "\n";
    ostr << "global_collect_index " << global_collect_index << "\n";
    ostr << "global_laser_collection " << GLOBAL_laser_collection << "\n";
  } /* SCOPE FOR ostr */
  global_app_dirlock.Unlock();
  // std::cout << "SAVED" << std::endl;  
}

void LoadState() {
  //std::cout << "TRY TO LOAD" << std::endl;
  static unsigned int last_frame_counter = 0;
  while (!global_app_dirlock.TryLock()) { usleep(1000); }
  bool success;
  { /* SCOPE FOR istr */
    std::ifstream istr(CALIBRATION_STATE_FILENAME);
    if (!istr) { success = false; }
    else {    
      std::string token;
      unsigned int this_frame;
      istr >> token >> this_frame;
      assert (token == "frame");
      if (this_frame == last_frame_counter) {
	success = false;
      } else {
	if (this_frame < last_frame_counter) {
	  std::cout << "whoops, must have started with a bad frame " << this_frame << " vs. " << last_frame_counter << std::endl;
	}
	last_frame_counter = this_frame;
	istr >> token >> global_collect_index;
	assert (token == "global_collect_index");
	istr >> token >> GLOBAL_laser_collection;
	assert (token == "global_laser_collection");
	success = true;
      }
    }
  } /* SCOPE FOR istr */
  global_app_dirlock.Unlock();
  if (success) {
    //std::cout << "LOADED" << std::endl;  
  } else {
    //std::cout << "LOAD FAIL" << std::endl;  
  }
  //usleep(100000);
}


void idle(void) {

  load_and_save_key_and_mouse_data(global_mk_dirlock,MK_ACTION_FILENAME_TEMPLATE,
				   keypress_function,
				   specialkeypress_function,
				   mouseaction_function,
				   mousemotion_function);

  // do nothing for all_black
  if (args->all_black) return;

  if (!args->tiled_display.is_tiled_display) {
    if (GLOBAL_laser_collection == true) {
      collect_calibration_data_point();
    }    
  } else if (args->tiled_display.is_master) {
    SaveState();
    if (GLOBAL_laser_collection == true) {
      collect_calibration_data_point();
    }    
  } else {
    // do nothing if not master
    LoadState();
  }


  if (global_calibration_data == NULL) return;

  // TEST DATA
  if (args->test_data) {
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
  }

  display();
  // ~ 60 fp
  usleep(500);
  // ~ 30 fp
  //usleep(30000);
  //usleep(50000);
}


void reshape(int w, int h) {

  //h *=2;

  //  w *= 0.5;

  std::cout << "reshape " << w << " " << h << std::endl;
  args->tiled_display.reshape(w,h);
  //my_width = w;
  //args->tiled_display.my_height = h;
  //if (!args->tiled_display.is_tiled_display) {
  //args->tiled_display.full_display_width = args->tiled_display.my_width;
  //args->tiled_display.full_display_height = args->tiled_display.my_height;
  //}

  glViewport(0,0,w,h);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, 1.0, 1.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,
	    0.0, 0.0, 0.0,     
	    0.0, 1.0, 0.); 
  glTranslatef(0.0, 0.6, -1.0);

  
  // Create new calibration data
  if (args->collect_geometry) {
    //if (args->tiled_display.full_display_width != w ||
    //	args->tiled_display.full_display_height != h) {
      std::cout << "MAKE NEW CAL DATA" << std::endl;
      delete global_calibration_data;

    

      assert (DESIRED_BORDER >= 1);
      assert (DESIRED_X_CELLS >= 1);
      assert (DESIRED_Y_CELLS >= 1);

      assert ((DESIRED_X_CELLS+1)*(DESIRED_Y_CELLS+1) >= MAX_LASERS);

      int spacing_x = (args->tiled_display.full_display_width-2*DESIRED_BORDER) / int(DESIRED_X_CELLS);
      int spacing_y = (args->tiled_display.full_display_height-2*DESIRED_BORDER) / int(DESIRED_Y_CELLS);

      assert (spacing_x > 1);
      assert (spacing_y > 1);
      
      int border_x = (args->tiled_display.full_display_width-(DESIRED_X_CELLS*spacing_x)) / 2;
      int border_y = (args->tiled_display.full_display_height-(DESIRED_Y_CELLS*spacing_y)) / 2;

      assert (border_x >= DESIRED_BORDER);
      assert (border_y >= DESIRED_BORDER);

      global_calibration_data = 
	new PlanarCalibration(args->tiled_display.full_display_width,args->tiled_display.full_display_height, //w,h,
			      border_x, border_y,
			      spacing_x, spacing_y);
      //}
  }
}

// this will get called eventually :)
void keypress_function(int /*which_keyboard*/, unsigned char key, int x, int y, int /*glut_modifiers*/) {
  if (args->tiled_display.is_tiled_display &&
      !args->tiled_display.is_master) {
    // non master should ignore all input
    return;
  }
  
  if (key == 'q') { 
    exit(0); 
  } else if (key == 'b' || key == ' ') {

    
    if (args->collect_geometry || args->collect_intensity) {
  
      if (GLOBAL_laser_collection == true) {
	std::cout << "ERROR! didn't finish collecting last point" << std::endl;
      } else {
	GLOBAL_laser_timer_count = 0;
	GLOBAL_laser_collection = true;
	//GLOBAL_frame_count = GLOBAL_frame_max_count;
      }
    }
  } else {
    std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
  display();
  glutPostRedisplay();
}


// the wrapper function
void keyboard(unsigned char key, int x, int y) {
  log_keypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
}



// ===================================================================

int  main(int argc, char **argv) {

  args = new ArgParser(argc,argv);
  global_point_tracker = NULL;

  if (args->tracking_logfile != "") {
    tracking_logfile = new std::ofstream(args->tracking_logfile.c_str());    
  }
  
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
  //printf("main 1\n");
  if (args->tiled_display.full_screen) {
    glutFullScreen();
 // 3840x1080, 16bit pixel depth, 60Hz refresh rate
    //glutGameModeString( "640x480:16@60" );
     //HandleGLError("main 2");
    //printf("main 2\n");
    // start fullscreen game mode
    //glutEnterGameMode();
  HandleGLError("main 3");
  // printf("main 3\n");
  }

  //  display();
  //reshape(args->width,args->height);

  if (args->test_data) {
    global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,
						    PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);
    global_point_tracker = new PointTracker(global_calibration_data,
					    AddTrackedPoint,RemoveTrackedPoint,100,0.5);


    //std::cout << "num cameras " << global_calibration_data->all_my_data.size() << std::endl;
  }

  if (args->collect_intensity) {
    //which_lap = 1;    
    if (args->clear_all_intensities) {
      global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,
						      ""); 
    } else { 
      global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,
						      PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);

      for (unsigned int l = 0; l < args->clear_laser_intensity.size(); l++) {
	global_calibration_data->clear_laser_intensity(args->clear_laser_intensity[l]);
      }

      global_calibration_data->write_intensity_to_file(PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);	
// CHRIS STUETZLE
//      global_calibration_data->writeOutGNUPlotDataFiles(PLANAR_CALIBRATION_GNU_PLOT_INTENSITY_DATA_FILE_PREFIX);
    }

  }
  
  glutMainLoop();

  return 0;
}
