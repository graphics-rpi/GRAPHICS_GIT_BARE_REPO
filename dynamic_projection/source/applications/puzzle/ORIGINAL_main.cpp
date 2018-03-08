#include "../../../dynamic_projection/source/applications/paint/gl_includes.h"


#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <ctime>
#include <iomanip>

#include "../../common/directory_locking.h"
#include "../../../../remesher/src/vectors.h"

#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"

// Include the Interaction class
#include "../../multi_mouse/interaction.h"

#include "argparser.h"
#include "../paint/button.h"
#include "../paint/path.h"
#include "../paint/text.h"
#include "../../common/Image.h"
#include "PuzzlePiece.h"
#include "../../multi_mouse/key_and_mouse_logger.h"

#define __NO_SOUND__

#ifdef __APPLE__
#define __NO_SOUND__
#endif

#ifndef __NO_SOUND__
#include "../../common/Sound/SoundHandler.h"
#endif

#include "score.h"

std::map<int,Score> all_scores;

bool completed = false;


//int GLOBAL_total_moves = 0;
int GLOBAL_total_time = 0;
int GLOBAL_fastest_time;
int GLOBAL_fewest_moves;
int GLOBAL_num_people;




//#define IR_STATE_DIRECTORY                "../state/ir_tracking"
//#define FOUND_IR_POINTS_FILENAME          "../state/ir_tracking/found_ir_points.txt"
//#define PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_geometry_data.txt"
//#define PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME  "../state/ir_tracking/planar_calibration_intensity_data.txt"

//#define MK_STATE_DIRECTORY                   "../state/mouse_and_keyboard"
//#define MK_ACTION_FILENAME_TEMPLATE          "../state/mouse_and_keyboard/actions_XXX.txt"

#define APPLICATIONS_STATE_DIRECTORY            "../state/applications"
#define PUZZLE_STATE_FILENAME             "../state/applications/puzzle_state.txt"
#define PUZZLE_HIGH_SCORES_FILENAME             "../state/applications/puzzle_high_scores.txt"

#define BORDER_THICKNESS 25

int SPACING;
int X_BORDER;
int Y_BORDER;

int SNAP_GRID = 3;

double TILE_w;
double TILE_h;

// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args;
DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);

// Change these to use ClickableObjects
Colors global_colors;
std::vector<PuzzlePiece*> piece_data;
std::vector<PuzzlePiece*> piece_pointers;

// Don't know what this is...
std::vector<std::pair<std::list<Pt>,Vec3f> > GLOBAL_strokes;

// Change this to all be stored within Cursor ... multi-mice
Pt mouse_location;
std::vector<Pt> positions;
std::vector<Pt> correctpositions;

#ifndef __NO_SOUND__
SoundHandler soundcontroller = SoundHandler();
#endif
// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

void prepare_strokes();
void draw_strokes(); 
void draw_grid();
void draw_scores();
void draw_cursors();
void draw_background_buttons();
void draw_pressed_buttons();

void initialize_buttons();
//void check_for_button_press();
//void check_for_button_motion();
void check_for_correctness();
void nudge_toward_grid();

void LoadHighScores(const std::string &high_scores_filename,
                    const std::string &image_filename,
                    int cols, int rows,
                    int &fastest_time, int &fewest_moves, int &num_people);
  

void AppendHighScore(const std::string &high_scores_filename,
                     const std::string &image_filename,
                     int cols, int rows,
                     int time, const std::map<int,Score> &all_scores);


void TryToPressButton(int id, double x, double y);
void TryToReleaseButton(int id, double x, double y);
void TryToMoveButton(int id, double x, double y);

void keyfunc(unsigned char key, int x, int y);
void specialkeyfunc(int key, int x, int y);		
void mousefunc(int button, int state, int x, int y);
void motionfunc(int x, int y);

void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers);
void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers);		 
void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers);
void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers);


/*
// CHRIS STUETZLE 
// No longer needed here, it's in interaction.cpp
void clamp_to_display(Pt &pt) { 
  int x = pt.x;
  int y = pt.y;
  x = std::max(0,std::min(x,args->tiled_display.full_display_width));
  y = std::max(0,std::min(y,args->tiled_display.full_display_height));
  pt = Pt(x,y);
}
*/


#ifndef __NO_SOUND__
void initsound(SoundHandler* Controller);
#endif

void SavePuzzleState(const std::vector<PuzzlePiece*> &piece_data, const std::string &state_filename);
void LoadPuzzleState(std::vector<PuzzlePiece*> &piece_data, const std::string &state_filename);

void check_for_correctness();

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

#ifndef __NO_SOUND__
void initsound(SoundHandler* Controller){

	Controller->AddSource("../../common/Sound/wavdata/gameover.wav");
	Controller->AddSource("../../common/Sound/wavdata/FancyPants.wav");
	Controller->AddSource("../../common/Sound/wavdata/guitar.wav");

}
#endif

// ==========================================================================================
// DRAWING ROUTINES
// ==========================================================================================

bool piece_sorter(const PuzzlePiece *p1, const PuzzlePiece *p2) { 
  //std::cout << "PIECE SORTER" << std::endl;
  assert (p1 != NULL);
  assert (p2 != NULL);
  //std::cout << "PIECE SORTER2" << std::endl;
  //std::cout << *p1 << std::endl;
  //std::cout << *p2 << std::endl;
  
  return (*p1) < (*p2); }

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

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	std::sort(piece_pointers.begin(),piece_pointers.end(),piece_sorter);


	draw_grid();

	draw_background_buttons();
	
	if (!args->tiled_display.is_tiled_display ||
	    (args->tiled_display.is_tiled_display && args->tiled_display.is_master)) {
	  prepare_strokes();
	}
	draw_strokes();

	draw_pressed_buttons();


	//draw_cursors();


	draw_scores();

	glDisable(GL_LINE_SMOOTH);
	//  glDisable(GL_BLEND);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);


	glutSwapBuffers();

}

void draw_cursors() {
  Vec3f color;
  glPointSize(20);
  glBegin(GL_POINTS);
  color = global_colors.GetColor(PRIMARY_MOUSE);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,args->tiled_display.full_display_height-GLOBAL_PRIMARY_MOUSE_POS.y);
  //glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,GLOBAL_PRIMARY_MOUSE_POS.y);

  color = global_colors.GetColor(MOUSE_2);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_2_POS.x,args->tiled_display.full_display_height-GLOBAL_MOUSE_2_POS.y);
  color = global_colors.GetColor(MOUSE_3);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_3_POS.x,args->tiled_display.full_display_height-GLOBAL_MOUSE_3_POS.y);
  color = global_colors.GetColor(MOUSE_4);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_4_POS.x,args->tiled_display.full_display_height-GLOBAL_MOUSE_4_POS.y);

  color = global_colors.GetColor(MOUSE_5);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_5_POS.x,args->tiled_display.full_display_height-GLOBAL_MOUSE_5_POS.y);
  color = global_colors.GetColor(MOUSE_6);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_6_POS.x,args->tiled_display.full_display_height-GLOBAL_MOUSE_6_POS.y);
  glEnd();

}

void draw_grid() {
  
  int SPACING = 1;
  
  Pt pt,pt2;

  glColor3f(0.15,0.15,0.15);
  glBegin(GL_QUADS);
  for (int i = 0; i < args->cols; i++) {
    for (int j = 1; j <= args->rows; j++) {
      glVertex2f(X_BORDER+i*(TILE_w+SPACING),       SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j)*(TILE_h+SPACING)));
      glVertex2f(X_BORDER+i*(TILE_w+SPACING)+TILE_w,SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j)*(TILE_h+SPACING)));
      glVertex2f(X_BORDER+i*(TILE_w+SPACING)+TILE_w,SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j+0)*(TILE_h+SPACING))+TILE_h);
      glVertex2f(X_BORDER+i*(TILE_w+SPACING),       SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j+0)*(TILE_h+SPACING))+TILE_h);
    }
  }
  glEnd();
  
}


void make_nice_time(std::stringstream &ss, int t) {
  int sec = int(t) % 60;
  int min = int(t) / 60;
  ss << min << ":" << std::setw(2) << std::setfill('0') << sec;
}


std::string intToString(int a) { 
  if (a == 0) return " ";
  std::stringstream tmp;
  tmp << a;
  return tmp.str();
}


void pretty_colors_move_quality(const std::string &name, int good, int neutral, int bad, int which, int total_count) {
  std::stringstream tmp;
  int w = args->tiled_display.full_display_width;
  int h = args->tiled_display.full_display_height;


  std::string a = name;
  std::string b = " "+intToString(good);
  std::string c = " "+intToString(neutral);
  std::string d = " "+intToString(bad);

  //  double aw = drawstring_relative_width(a.c_str());
  //double bw = drawstring_relative_width(b.c_str());
  //double cw = drawstring_relative_width(c.c_str());
  //double dw = drawstring_relative_width(d.c_str());
  //double sumw = 1.2*(aw + bw + cw + dw);

  //  std::cout << "widths " << aw << " " << bw << " " << cw << " " << dw << std::endl;


  //  double horiz = (0.5+which) / double(total_count);
  //
  //double width = (which + 0.5) / double(total_count*1.1); 
  //double height = 0.05*h;


  std::vector<std::string> texts;
  texts.push_back(a);
  texts.push_back(b);
  texts.push_back(c);
  texts.push_back(d);
  std::vector<Vec3f> colors;
  colors.push_back(Vec3f(1,1,1));
  colors.push_back(Vec3f(0,1,0));
  colors.push_back(Vec3f(1,1,0));
  colors.push_back(Vec3f(1,0,0));

  drawstrings((0.5+which)*w/double(total_count), 0.05*h, texts,colors, 1/(total_count*1.1)*w, 0.05*h);

  //  double width = 1 / double(total_count*1.1); 
  //  double height = 0.05*h;


}




void draw_scores() {

  // COLLECT THE CURRENT TIME
  static time_t start = time(NULL);
  static time_t last = time(NULL);
  double diff;
  double diffstart = difftime(last,start);
  if (!completed) {
    time_t current = time(NULL);
    diff = difftime(current,last);
    diffstart = difftime(current,start);
    if (diff >= 0.1) {
      last = current;
    }
  }

  // helper variables
  std::stringstream tmp;
  int w = args->tiled_display.full_display_width;
  int h = args->tiled_display.full_display_height;


  // DISPLAY BEST TIME
  if (GLOBAL_fastest_time > 0) {
    tmp << "best: ";
    make_nice_time(tmp,GLOBAL_fastest_time);
    tmp << " in " << GLOBAL_fewest_moves << " moves";
  } else {
    tmp << "no previous score";
  }
  drawstring(0.25*w, 0.95*h, tmp.str().c_str(), Vec3f(1,1,1), 0.4*w,0.05*h);


  int current_total_moves = 0;
  for (std::map<int,Score>::iterator itr = all_scores.begin(); itr != all_scores.end(); itr++) {
    current_total_moves += itr->second.getGood() + itr->second.getNeutral() + itr->second.getBad();
  }


  // DISPLAY CURRENT TIME
  tmp.str("");
  GLOBAL_total_time = int(diffstart);
  tmp << "current: ";
  make_nice_time(tmp,diffstart);
  tmp << " in " << current_total_moves << " moves";
  drawstring(0.75*w, 0.95*h, tmp.str().c_str(), Vec3f(1,1,1), 0.4*w,0.05*h);


  int count = 0;
  int num_players = all_scores.size();
  for (std::map<int,Score>::iterator itr = all_scores.begin(); itr != all_scores.end(); itr++) {
    tmp.str("");
    if (itr->first == 201) {
      tmp << "mouse:";
    } else {
      tmp << itr->first << ":";
    }
    pretty_colors_move_quality(tmp.str(), itr->second.getGood(), itr->second.getNeutral(), itr->second.getBad(), count, num_players); 
    count++;
  }
  

}



void draw_background_buttons() {
  for (unsigned int i = 0; i < piece_pointers.size(); i++) {
    if (piece_pointers[i]->hasBorder()) continue;
    piece_pointers[i]->paint();
  }
}


void draw_pressed_buttons() {
  for (unsigned int i = 0; i < piece_pointers.size(); i++) {
    if (!piece_pointers[i]->hasBorder()) continue;
    piece_pointers[i]->paint();
  }
}


void prepare_strokes() { //bool button_strokes) {

  assert (Interaction::getGlobalPointTracker() != NULL);
  GLOBAL_strokes.clear();
    
  for (unsigned int i = 0; i < Interaction::getGlobalPointTracker()->size(); i++) {
    int id = (*Interaction::getGlobalPointTracker())[i].getID();
//    bool pressing = false;
    // check if this tracker is attached to a button
//    for (unsigned int j = 0; j < piece_pointers.size(); j++) {
//      if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == id)
//        pressing = true;
//    }
    
//    Pt pt = (*Interaction::getGlobalPointTracker())[i].getCurrentPosition();
    int laser = (*Interaction::getGlobalPointTracker())[i].getWhichLaser();
    Vec3f color = global_colors.GetColor(id);

    if (laser != -1) {
      assert (laser >= 0 && laser < MAX_LASERS);
      color = fixed_laser_colors[laser];
      for (unsigned int j = 0; j < piece_pointers.size(); j++) {
	Button *b2 = piece_pointers[j];
	if (b2->isPressed() && b2->PressedBy() == id) {	  
	  b2->clearBorders();
	  b2->addBorder(BorderInfo(color,BORDER_THICKNESS,1));
	  //std::cout << "set border info" << std::endl;
	}
	//b2.setBorderColor(color);
      }
      //drawstring(pt.x,pt.y,laser_names[laser],color.x(),color.y(),color.z(),50,50,3.0);
    }
    
    //    if (!pressing && button_strokes) continue;
    //if (pressing && !button_strokes) continue;
    
    if (!(*Interaction::getGlobalPointTracker())[i].IsActive()) continue;
 
    GLOBAL_strokes.push_back(std::make_pair((*Interaction::getGlobalPointTracker())[i].getPtTrail(),color));    
  }
}

void draw_strokes() {//bool button_strokes) {

  for (unsigned int i = 0; i < GLOBAL_strokes.size(); i++) {
    const Vec3f &color = GLOBAL_strokes[i].second;
    // SET COLOR
    glColor3f(color.x(),color.y(),color.z());
    const std::list<Pt> &trail = GLOBAL_strokes[i].first;
    // DRAW TRAIL
    glLineWidth(3);
    Path::draw_smooth_stroke(trail); //(*Interaction::getGlobalPointTracker())[i].getPtTrail());
    
  }
}


void display(void) {
  HandleGLError("BEFORE DISPLAY");
  draw();
  HandleGLError("AFTER DISPLAY");
}


void ReleasePiece(int i, int id) {
  PuzzlePiece *p = piece_pointers[i];
  assert (p->isPressed());
  //  std::cout << "pressed by " << id << " " << p->PressedBy() << std::endl;
  assert (p->PressedBy() == id);


  int which_cursor = -1;
  for (unsigned int i = 0; i < Interaction::getGlobalPointTracker()->size(); i++) {
    int tmp_id = (*Interaction::getGlobalPointTracker())[i].getID();
    if (id != tmp_id) {
      continue;
    }
    which_cursor = (*Interaction::getGlobalPointTracker())[i].getWhichLaser()+1;
    std::cout << "FOUND " << which_cursor  << std::endl;
  }

  if (which_cursor == -1) which_cursor = id;
  std::cout << "which cursor " << which_cursor << std::endl;




  //piece_pointers[i]->release();

  piece_pointers[i]->release();

  //  GLOBAL_total_moves++;
  if (!completed) {
    piece_pointers[i]->OnRelease(all_scores[which_cursor]);
  }

}

// ==========================================================================================
// TRACKING ROUTINES
// ==========================================================================================

// EVENTUALLY, MOVE THIS FUNCTIONALITY MORE INTO INTERACTION
void AddTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  global_colors.AssignRandomAvailableColor(id);
}


void RemoveTrackedPoint(const TrackedPoint &pt) {
  int id = pt.getID();
  Vec3f color = global_colors.GetColor(id);
  global_colors.RemoveId(id);
  
  
  for (unsigned int i = 0; i < piece_pointers.size(); i++) {
    if (piece_pointers[i]->isPressed() && piece_pointers[i]->PressedBy() == id) {
      //button_pressed_by[i] = -1;
      ReleasePiece(i,id);
      
    }
  }
  
}






// ==========================================================================================
// IDLE
// ==========================================================================================

void idle(void) {

// CHRIS STUETZLE
// INTERACTION STUFF GOES HERE... MOST OF THIS CODE WAS MOVED TO INTERACTION

/*
  //  std::cout << "IDLE" << std::endl;
  bool success = false;

  if (args->tiled_display.is_tiled_display) {
    success = load_and_save_key_and_mouse_data(global_mk_dirlock,MK_ACTION_FILENAME_TEMPLATE,
                                               keyfunc_helper,
                                               specialkeyfunc_helper,
                                               mousefunc_helper,
                                               motionfunc_helper);
  }
  
  if (success) {
    exit(0);
  }
*/
  if (!args->tiled_display.is_tiled_display ||
      (args->tiled_display.is_tiled_display && args->tiled_display.is_master)) {


/* 
    // THIS PROCESS IS RESPONSIBLE FOR DEALING WITH THE LASERS
    // look for new IR data
    while (!global_ir_dirlock.TryLock()) {
      usleep(1000);
    }
    std::vector<IR_Data_Point> raw_points;


    //std::cout << "before" << success << std::endl;

    { // SCOPE FOR istr 
      std::ifstream istr(Interaction::FOUND_IR_POINTS_FILENAME.c_str());
      //if (!istr) { 
      //	args->lasers_enabled = false; 
      //	success = true;
      //}
      //else { 
      if (istr) {
        assert (istr);
        success = PointTracker::ReadIRPointData(istr,raw_points) || (mouse_location.x >= 0 && mouse_location.y >= 0);
      } else {
        //std::cerr << "CANNOT OPEN LASER TRACKER FILE: " << FOUND_IR_POINTS_FILENAME << std::endl;
      }
      //}
    } // SCOPE FOR istr 

    //std::cout << "RAW POINTS" << success << std::endl;

    global_ir_dirlock.Unlock();
    if (!success) {// && !GLOBAL_KEY_OR_MOUSE_PRESS) {
      // new data not yet available
      usleep(1000);
      //GLOBAL_KEY_OR_MOUSE_PRESS = true;
      //return;
    }
*/

    //    GLOBAL_KEY_OR_MOUSE_PRESS = false;

    //std::cout << "RAW POINTS2" << std::endl;

    
//    if(mouse_location.x != -1 && mouse_location.y != -1){
//      Pt temp_pt;
//      temp_pt.y = args->tiled_display.full_display_height - mouse_location.y;
//      temp_pt.x = mouse_location.x;
    
//      std::vector<Pt> mouse_pts;
//      mouse_pts.push_back(temp_pt);
//      Interaction::getGlobalPointTracker()->ProcessPointData(raw_points /*,mouse_pts*/ );
//  }
//  else{

//  }
    
    
    // CHRIS STUETZLE - This should be moved to within Interaction
    // THIS HAS BEEN MOVED INTO DetectAndProcessGestures
    // Interaction::ProcessPointData( raw_points );
  
    //    if(completed){
    //nudge_toward_grid();
    //return;
    //}

    // IS THIS WHERE WE FIGURE OUT WHAT TO DO WITH THE PIECE? ALL CURSORS?
    //check_for_button_press();
    //check_for_button_motion();
//    Interaction::DetectAndProcessGestures();

    // DOES THIS "SNAP" TO THE GRID?
//    nudge_toward_grid();
    // FINISH THE PUZZLE?
//    check_for_correctness();

    // std::cout << "RAW POINTS3" << std::endl;

    if (args->tiled_display.is_tiled_display && args->tiled_display.is_master) {      
      SavePuzzleState(piece_data, PUZZLE_STATE_FILENAME);
    } // end of if
  } // end of if
  else {
    // THIS PROCESS IS NOT RESPONSIBLE FOR LASERS, JUST FOR VISUALIZING
    assert (args->tiled_display.is_tiled_display && !args->tiled_display.is_master);
    LoadPuzzleState(piece_data, PUZZLE_STATE_FILENAME);
  } // end of else
  
  //std::cout << "redisplay" << std::endl;
  display();
  usleep(1000);
   
}


#define HOVER_THRESHOLD 10

/* 
// MOVED TO LASER
void check_for_button_motion() {

	for (unsigned int j = 0; j < piece_pointers.size(); j++) {
		if (!piece_pointers[j]->isPressed()) continue;
		int id = piece_pointers[j]->PressedBy();
		assert (id != -1);

		Button *b2 = piece_pointers[j];
		for (unsigned int i = 0; i < Interaction::getGlobalPointTracker()->size(); i++) {
			if (id != (*Interaction::getGlobalPointTracker())[i].getID()) continue;

			Pt p = (*Interaction::getGlobalPointTracker())[i].getCurrentPosition();
			Pt offset = piece_pointers[j]->getPressPosition();

			Pt q = Pt(p.x-offset.x,p.y-offset.y);
			//b2->MoveWithMaxSpeed(q,20);
			b2->Move(q);
		}
	}
}
*/


Pt SnappedToGrid(const Pt &current) {

  double di = (current.x-X_BORDER) / (TILE_w+SPACING) ;
  double dj = (SPACING + args->tiled_display.full_display_height - current.y - Y_BORDER) / (TILE_h + SPACING) - 1 ;
  // make sure it doesn't move too far off the board
  if (di < -1.0) di = -1.0;
  if (dj < -1.0) dj = -1.0;
  if (di > args->cols) di = args->cols;
  if (dj > args->rows) dj = args->rows;
  
  double i2 = int(SNAP_GRID*(di+5)+0.5) / double(SNAP_GRID) - 5;
  double j2 = int(SNAP_GRID*(dj+5)+0.5) / double(SNAP_GRID) - 5;
  
  double closest_x = X_BORDER+i2*(TILE_w+SPACING);
  double closest_y = SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j2+1)*(TILE_h+SPACING));
  return Pt(closest_x,closest_y);
}



void nudge_toward_grid() {
  for (unsigned int j1 = 0; j1 < piece_pointers.size(); j1++) {
    if (piece_pointers[j1]->isPressed()) continue;
    
    Button *b = piece_pointers[j1];
    
    double b_width = b->getWidth();
    double b_height = b->getHeight();
    
    const Pt &current = b->getCentroid() - Pt(b_width/2.0,b_height/2.0); 
    
    Pt snapped = SnappedToGrid(current);
    
    b->Move(Pt(snapped.x+b_width/2.0,snapped.y+b_height/2.0));
    
  }
}



void TryToPressButton(int id, double x, double y) {

  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;
  
  y = args->tiled_display.full_display_height-y;

  //  std::cout << "TRY TO PRESS " << id << std::endl;

  // check if this tracker is already attached to a button
  for (unsigned int j = 0; j < piece_pointers.size(); j++) {
    if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == id) {
      //already = true;
      std::cout << "ALREADY PRESSING A BUTTON! " << std::endl;
      return;
    }
  }


  //for (unsigned int j = 0; j < piece_pointers.size(); j++) {
  for (int j = int(piece_pointers.size())-1; j >= 0; j--) {
    PuzzlePiece *b2 = piece_pointers[j];
    //std::cout << "considering " << std::endl;

    if (b2->PointInside(Pt(x,y))) { //Hover(trail,HOVER_THRESHOLD)) {
      
      // don't steal from someone else!
      if (piece_pointers[j]->isPressed()) {
	//std::cout << "ALREADY PRESSED" << std::endl;
	continue;
      }
      
      assert (!piece_pointers[j]->isPressed());
        
      //      std::cout << "PRESS THIS BUTTON!" << std::endl;
      Pt pt = b2->Offset(Pt(x,y)); //(*Interaction::getGlobalPointTracker())[i].getCurrentPosition());
      b2->press(global_colors.GetColor(id),id,pt);
      b2->OnPress();

      // don't click more than one button!!!
      break;
      
    }
  }
}

void TryToReleaseButton(int id, double x, double y) {
  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;
  y = args->tiled_display.full_display_height-y;
  for (unsigned int j = 0; j < piece_pointers.size(); j++) {
    if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == id) {

      //      std::cout << "pressed by " <<
      //piece_pointers[j]->isPressed()
      //        << " " << piece_pointers[j]->PressedBy() << == id) { id << " " << p->PressedBy() << std::endl;

      ReleasePiece(j,id);
      //piece_pointers[j]->release();
      return;
    }
  }
  //  std::cout << "OH WELL, NOT PRESSING A BUTTON! " << std::endl;
}

void TryToMoveButton(int id, double x, double y) {
  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;
  y = args->tiled_display.full_display_height-y;
  // check if this tracker is already attached to a button
  for (unsigned int j = 0; j < piece_pointers.size(); j++) {
    if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == id) {
      Button *b2 = piece_pointers[j];
      Pt p = Pt(x,y);
      Pt offset = piece_pointers[j]->getPressPosition();
      Pt q = Pt(p.x-offset.x,p.y-offset.y);
      b2->Move(q);
      return;
    }
  }
  //  std::cout << "OH WELL, NOT PRESSING A BUTTON! " << std::endl;
}



// From Interaction
//void check_for_button_press() {
void TryToPressObj( int index ) {
/*
  for (unsigned int i = 0; i < Interaction::getGlobalPointTracker()->size(); i++) {
    if (!(*Interaction::getGlobalPointTracker())[i].IsActive()) continue;
    int id = (*Interaction::getGlobalPointTracker())[i].getID();
    const std::list<Pt> &trail = (*Interaction::getGlobalPointTracker())[i].getPtTrail();
    
    bool already = false;
    // check if this tracker is already attached to a button
    for (unsigned int j = 0; j < piece_pointers.size(); j++) {
      if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == id) {
	    already = true;
      }
    }
    if (already) continue;
    
    //for (unsigned int j = 0; j < piece_pointers.size(); j++) {
    for (int j = int(piece_pointers.size())-1; j >= 0; j--) {
      PuzzlePiece *b2 = piece_pointers[j];
// This is where the "click and drag" probably goes...
      if (b2->Hover(trail,HOVER_THRESHOLD)) {
	    // don't steal from someone else!
	    if (!piece_pointers[j]->isPressed()) {
	      Pt pt = b2->Offset((*Interaction::getGlobalPointTracker())[i].getCurrentPosition());
	      b2->press(global_colors.GetColor(id),id,pt);
	      b2->OnPress();
	    }
	
	    // don't click more than one button!!!
	    break;
      }
    }
    
  }
*/
  
  for(unsigned int k = 0; k < piece_pointers.size(); ++k)
    piece_pointers[k]->ResetBorder();
}


void corner_check(){

	enum position{center=0, topleft=1, left=2, botleft=3, bot=4,
		botright=5, right=6, topright=7, top=8};

	for(unsigned int i = 0; i < piece_pointers.size(); ++i)
	{

		if(piece_pointers[i]->GetBorderVal(left) && piece_pointers[i]->GetBorderVal(top))
			piece_pointers[i]->SetBorderVal(topleft, 1);
		if(piece_pointers[i]->GetBorderVal(left) && piece_pointers[i]->GetBorderVal(bot))
			piece_pointers[i]->SetBorderVal(botleft, 1);
		if(piece_pointers[i]->GetBorderVal(bot) && piece_pointers[i]->GetBorderVal(right))
			piece_pointers[i]->SetBorderVal(botright, 1);
		if(piece_pointers[i]->GetBorderVal(right) && piece_pointers[i]->GetBorderVal(top))
			piece_pointers[i]->SetBorderVal(topright, 1);

	}
}

void xedge_set(PuzzlePiece &a, PuzzlePiece &b){

	//Indexing list to make marking borders easier
	enum position{center=0, topleft=1, left=2, botleft=3, bot=4,
		botright=5, right=6, topright=7, top=8};

	double epsilon = 1;
	double cxdif = a.getCorrectPoint().x - b.getCorrectPoint().x;
	double cydif = a.getCorrectPoint().y - b.getCorrectPoint().y;
	double xdif = a.getCentroid().x - b.getCentroid().x;

	if( !(fabs(fabs(cxdif) - TILE_w) < epsilon) || fabs(cydif) > 1){
		return;
	}

	if( (cxdif < 0) && fabs(xdif + TILE_w) < epsilon )
	{
		a.SetBorderVal(right, true);
		b.SetBorderVal(left, true);
	}
	else if( (cxdif > 0) && fabs(xdif - TILE_w) < epsilon )
	{
		a.SetBorderVal(left, true);
		b.SetBorderVal(right, true);
	}

}

void yedge_set(PuzzlePiece &a, PuzzlePiece &b){

	//Indexing list to make marking borders easier
	enum position{center=0, topleft=1, left=2, botleft=3, bot=4,
		botright=5, right=6, topright=7, top=8};

	double epsilon = 1;
	double cxdif = a.getCorrectPoint().x - b.getCorrectPoint().x;
	double cydif = a.getCorrectPoint().y - b.getCorrectPoint().y;
	double ydif = a.getCentroid().y - b.getCentroid().y;

	if( !(fabs(fabs(cydif) - TILE_h) < epsilon) || fabs(cxdif) > 1){
		return;
	}

	if( (cydif < 0) && fabs(ydif + TILE_h) < epsilon )
	{
		a.SetBorderVal(top, true);
		b.SetBorderVal(bot, true);
	}
	else if( (cydif > 0) && fabs(ydif - TILE_h) < epsilon )
	{
		a.SetBorderVal(bot, true);
		b.SetBorderVal(top, true);
	}

}

void neighbor_check(PuzzlePiece &a, std::vector<PuzzlePiece*> &Pieces){

	double distance;
	double xdistance;
	double ydistance;
	//Neighbor checks are based on distance, these variables
	//are meant to store the test distances

	//Testing loops
	for(unsigned int i = 0; i < Pieces.size(); ++i)
	{

	  distance = a.CentroidDist(*Pieces[i]);
		xdistance = fabs(a.getCentroid().x - Pieces[i]->getCentroid().x);
		ydistance = fabs(a.getCentroid().y - Pieces[i]->getCentroid().y);

		if(!piece_pointers[i]->isPressed()){

			if( distance != 0 ){

				if( (xdistance - (TILE_w)) < ((1/10.0)*TILE_w) && ydistance < 1)
				{
					xedge_set(a, *Pieces[i]);
				}
				if( (ydistance - (TILE_h)) < ((1/10.0*TILE_h)) && xdistance < 1)
				{
					yedge_set(a, *Pieces[i]);
				}

			}
			else
				continue;
		}

	}


}

void check_for_correctness(){

  //  std::cout << "CHECK FOR CORRECTNESS... " << std::endl;

  bool complete = true;
  bool skip = false;

  for(unsigned int i = 0; i < piece_pointers.size(); ++i){
    if(!piece_pointers[i]->isPressed())
      neighbor_check(*piece_pointers[i], piece_pointers);
  }

  corner_check();

  for(unsigned int i = 0; i < piece_pointers.size(); ++i) {
    skip = false;
    if(!piece_pointers[i]->isCorrect())
      complete = false;
    
    if(piece_pointers[i]->getCorrectPoint() == piece_pointers[i]->getLowerLeftCorner()) {
      
      //this loop makes sure that there is no button in the slot
      //this makes sure that we do not mark a piece correct if
      //two are occupying the same tile
      for(unsigned int j = 0; j < piece_pointers.size(); ++j) {        
        if(piece_pointers[i]->getCorrectPoint() == piece_pointers[j]->getLowerLeftCorner() && i != j) {
          skip = true;
        }
      }

      if(!skip && !piece_pointers[i]->isCorrect()) {
#ifndef __NO_SOUND__
        soundcontroller.PlaySource(2);
#endif
        piece_pointers[i]->setCorrectness(true);
      }      
    }
    else {
      piece_pointers[i]->setCorrectness(false);
    }
    
  }
  
  //  std::cout << "HERE" << std::endl;

  if(complete && piece_pointers.size() > 0) {

    if (completed == false) {
      AppendHighScore(PUZZLE_HIGH_SCORES_FILENAME, args->image_filename.c_str(),
                      args->cols, args->rows, GLOBAL_total_time, all_scores);
    }
    completed = true;

    //    std::cout << "COMPLETE!" << std::endl;
    
    for(unsigned int k = 0; k < piece_pointers.size(); ++k)
      piece_pointers[k]->FullBorder();
    


    /*
    sleep(2);
#ifndef __NO_SOUND__
    soundcontroller.PlaySource(0);
#endif
    */
  }
  
}


// ===================================================================
// MOUSE FUNCTIONS


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

	assert (piece_pointers.size() == piece_data.size());

	/*
	for(unsigned int i = 0; i < piece_pointers.size(); ++i)
		piece_pointers[i]->ResetBorder();
	*/

	HandleGLError("AFTER RESHAPE");
}







void keyfunc(unsigned char key, int x, int y) {
  //  std::cout << "RECEIVED KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    log_keypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  } else {
    keyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  }
}

void specialkeyfunc(int key, int x, int y) {
  //  std::cout << "RECEIVED SPECIAL KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    log_specialkeypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  } else {
    specialkeyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  }
}


void mousefunc(int button, int state, int x, int y) {
  //  std::cout << "RECEIVED MOUSE DIRECTLY: " << button << " " << state << " " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    // adjust position of click
    int from_top = args->tiled_display.full_display_height - (args->tiled_display.my_height+args->tiled_display.my_bottom);
    y += from_top;
    log_mouseaction(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
  } else {
    mousefunc_helper(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
  }
}

void motionfunc(int x, int y) {
  //std::cout << "\nSTART MOTIONFUNC" << std::endl;
  //  std::cout << "RECEIVED MOTION DIRECTLY: " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
    if (x == 0 && y == 0) {
      std::cout << "IGNORE: OFFSCREEN MOTION BUG" << std::endl;
      return;
    }
    int from_top = args->tiled_display.full_display_height - (args->tiled_display.my_height+args->tiled_display.my_bottom);
    y += from_top;
    log_mousemotion(PRIMARY_MOUSE,x,y,0); //glutGetModifiers());
  } else {
    motionfunc_helper(PRIMARY_MOUSE,x,y,0); //glutGetModifiers());
  }
  //std::cout << "END MOTIONFUNC\n" << std::endl;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

// this will get called eventually :)
void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers) {
  //  std::cout << "RECEIVED KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glut_modifiers << std::endl;
  if (key == 'q') {
    exit(0);
  }
  if(key == 'b'){
    for(unsigned int i = 0; i < piece_pointers.size(); ++i){
      piece_pointers[i]->ResetBorder();
    }
  }
  else {
    std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
  display();
}


void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers) {
  //  std::cout << "RECEIVED SPECIAL KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glut_modifiers << std::endl;

}

void mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers) {
  //  std::cout << "RECEIVED MOUSE FROM LOGGER: " << which_mouse << " " << button << " " << state << " " << x << " " << y << " " << glut_modifiers << std::endl;

  if (state == GLUT_DOWN) {
    TryToPressButton(which_mouse,x,y);
  }
  if (state == GLUT_UP) {
    TryToReleaseButton(which_mouse,x,y);
  }

}

void motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers) {
  //  std::cout << "RECEIVED MOTION FROM LOGGER: " << which_mouse << " " << x << " " << y << " " << glut_modifiers << std::endl;

  TryToMoveButton(which_mouse,x,y);

}

// ===================================================================

using std::cout;
using std::endl;

void cleanup_buttons() {
  for (int i = 0; i < (int)piece_data.size(); i++) {
    delete piece_data[i];
  }
  piece_data.clear();
}

void initialize_buttons() {

	//SPACING = args->width*0.003;
	SPACING = 0;
	X_BORDER = args->tiled_display.full_display_width*0.05;
	Y_BORDER = X_BORDER;
        
        //std::cout << "INITIALIZE BUTTONS " << piece_data.size() << std::endl;
        if (piece_data.size() != 0) {
          cleanup_buttons();
        }

        assert (piece_data.size() == 0);
	piece_data.clear();
	piece_pointers.clear();

	// unfortunately this image will get reloaded to create the button texture...
	Image<sRGB> image;
	image.load_from_file(args->image_filename.c_str());
	int iw = image.getCols();
	int ih = image.getRows();


        LoadHighScores(PUZZLE_HIGH_SCORES_FILENAME, args->image_filename.c_str(),
                       args->cols, args->rows,
                       GLOBAL_fastest_time, GLOBAL_fewest_moves, GLOBAL_num_people);

	
	int useable_screen_width = args->tiled_display.full_display_width - 2*X_BORDER - (args->cols-1)*SPACING;
	int useable_screen_height = args->tiled_display.full_display_height - 2*Y_BORDER - (args->rows-1)*SPACING;

	assert (useable_screen_width > 1);
	assert (useable_screen_height > 1);
	

	double image_aspect = iw / double(ih);
	double screen_aspect = useable_screen_width / double (useable_screen_height);
	double ratio = image_aspect / screen_aspect;

	if (ratio > 1) {
	  Y_BORDER += (1-1/ratio)*useable_screen_height / 2.0;
	} else {
	  X_BORDER += (1-ratio)*useable_screen_width / 2.0;
	}

	TILE_w = (args->tiled_display.full_display_width-2*X_BORDER-(args->cols-1)*SPACING) / double(args->cols);
	TILE_h = (args->tiled_display.full_display_height-2*Y_BORDER-(args->rows-1)*SPACING) / double(args->rows);

	if (TILE_w < 180 || TILE_h < 180) {
	  SNAP_GRID = 3;
	}

	std::cout << "SNAP GRID = " << SNAP_GRID << std::endl;

	positions.clear();
	correctpositions.clear();
	std::vector<Pt> text_coords;

	double du = 1.0 / args->cols;
	double dv = 1.0 / args->rows;


	for (int i = 0; i < args->cols; i++) {
	  for (int j = 0; j < args->rows; j++) {
	    positions.push_back(Pt(X_BORDER+i*(TILE_w+SPACING),SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j+1)*(TILE_h+SPACING))));
	    text_coords.push_back(Pt(i*du,j*dv));
	  }
	}

	correctpositions = positions;

	std::random_shuffle(positions.begin(),positions.end());
	std::random_shuffle(positions.begin(),positions.end());

	for (unsigned int p = 0; p < positions.size(); p++)
	{
	  piece_data.push_back(new PuzzlePiece(positions[p],TILE_w,TILE_h,args->image_filename,
					   text_coords[p].x,text_coords[p].y,
					   text_coords[p].x+du,text_coords[p].y+dv));
	  piece_data[p]->setCorrectPoint(correctpositions[p]);
	  piece_pointers.push_back(piece_data[p]);
	}

}

void SavePuzzleState(const std::vector<PuzzlePiece*> &buttons, const std::string &state_filename) {
  //std::cout << "TRY TO SAVE" << std::endl;

  static unsigned int frame_counter = 0;
  frame_counter++;
  
  while (!global_app_dirlock.TryLock()) { usleep(1000); }

  { /* SCOPE FOR ostr */
    std::ofstream ostr(state_filename.c_str());
    assert (ostr);
    ostr << "frame " << frame_counter << "\n";

    int num_pieces = piece_data.size();
    ostr << "num_pieces " << num_pieces << "\n";
    for (int i = 0; i < num_pieces; i++) {
      Pt pt = piece_data[i]->getCentroid();
      ostr << "piece " 
	   << std::setw(3) << i << " " 
	   << std::setw(10) << std::fixed << std::setprecision(3) << pt.x << " "
	   << std::setw(10) << std::fixed << std::setprecision(3) << pt.y << " "
	   << std::setw(6) << piece_data[i]->getLastTouched() << "  ";
      if (piece_data[i]->isPressed()) {
	Vec3f color = piece_data[i]->getSingleBorderColor();
	ostr << "pressed " << color.r() << " " << color.g() << " " << color.b() << "\n";
	//std::cout << "is pressed " << std::endl;
      } else {
	ostr << "not_pressed\n";
      }
    }
    int num_strokes = GLOBAL_strokes.size();
    ostr << "num_strokes " << num_strokes << "\n";
    for (int i = 0; i < num_strokes; i++) {
      std::list<Pt> trail = GLOBAL_strokes[i].first;
      Vec3f color = GLOBAL_strokes[i].second;
      ostr << "stroke " << i << " num_points " << trail.size() << " ";
      for (std::list<Pt>::iterator itr = trail.begin(); itr != trail.end(); itr++) {
	ostr << itr->x << " " << itr->y << " ";
      }
      ostr << "color " << color.r() << " " << color.g() << " " << color.b() << "\n";
    }


  } /* SCOPE FOR ostr */
  
  global_app_dirlock.Unlock();
  //std::cout << "SAVED" << std::endl;  
}


void LoadPuzzleState(std::vector<PuzzlePiece*> &buttons, const std::string &state_filename) {

  //std::cout << "TRY TO LOAD" << std::endl;

  static unsigned int last_frame_counter = 0;
  
  while (!global_app_dirlock.TryLock()) { usleep(1000); }

  bool success;
  { /* SCOPE FOR istr */
    std::ifstream istr(state_filename.c_str());
    if (!istr) { success = false; }
    else {    
      std::string token;
      unsigned int this_frame;
      istr >> token >> this_frame;
      assert (token == "frame");
      if (this_frame == last_frame_counter) {
	success = false;
	//std::cout << "same frame counter" << std::endl;
      } else {
	if (this_frame < last_frame_counter) {
	  std::cout << "whoops, must have started with a bad frame " << this_frame << " vs. " << last_frame_counter << std::endl;
	}
	last_frame_counter = this_frame;

	int num_pieces;
	istr >> token >> num_pieces;
	assert (token == "num_pieces");
	if (num_pieces != (int) piece_data.size()) {
	  success = false; 
	} else {
	  for (int i = 0; i < num_pieces; i++) {
	    int j;
	    double x,y;
	    int last_touched;
	    istr >> token >> j >> x >> y >> last_touched;
	    assert (i == j);
	    piece_data[i]->MoveNoDamping(Pt(x,y));
	    piece_data[i]->setLastTouched(last_touched);	    
	    istr >> token;
	    if (token == "pressed") {
	    double r,g,b;
	    istr >> r >> g >> b;
	    piece_data[i]->clearBorders();
	    piece_data[i]->addBorder(BorderInfo(Vec3f(r,g,b),BORDER_THICKNESS,1));
	    std::cout << "setting border " << std::endl;
	    } else {
	      assert (token == "not_pressed");
	      piece_data[i]->clearBorders();
	    }
	  }
	  success = true;
	}
	//	std::cout << "read " << num_pieces << " pieces" << std::endl;
	int num_strokes;
	istr >> token >> num_strokes;
	assert (token == "num_strokes");
	GLOBAL_strokes.clear();
	for (int i = 0; i < num_strokes; i++) {
	  int j;
	  istr >> token >> j;
	  assert (token == "stroke");
	  assert (i == j);
	  int num_points;
	  istr >> token >> num_points;
	  assert (token == "num_points");
	  std::list<Pt> pts;
	  for (int k = 0; k < num_points; k++) {
	    double x,y;
	    istr >> x >> y;
	    pts.push_back(Pt(x,y));
	  }
	  double r,g,b;
	  istr >> token >> r >> g >> b;
	  assert (token == "color");
	  GLOBAL_strokes.push_back(std::make_pair(pts,Vec3f(r,g,b)));	  
	}
	//std::cout << "loaded " << GLOBAL_strokes.size() << " strokes" << std::endl;
	success = true;
      }
    }
  } /* SCOPE FOR istr */
  
  global_app_dirlock.Unlock();

  for(unsigned int k = 0; k < piece_pointers.size(); ++k)
    piece_pointers[k]->ResetBorder();
  check_for_correctness();


  if (success) {
    //std::cout << "LOADED" << std::endl;  
  } else {
    //std::cout << "LOAD FAIL" << std::endl;  
  }
  //usleep(100000);

}



int  main(int argc, char **argv) 
{

	args = new ArgParser(argc,argv);

// ALL OF THESE THINGS BELONG IN INTERACTION
//	Interaction::setGlobalPointTracker( NULL );

//    Interaction::setGlobalCalibrationData( new PlanarCalibration(Interaction::PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME, Interaction::PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME) );
	//Interaction::getGlobalCalibrationData() = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME);

//	global_colors.AssignRandomAvailableColor(PRIMARY_MOUSE);
//	global_colors.AssignRandomAvailableColor(MOUSE_2);
//	global_colors.AssignRandomAvailableColor(MOUSE_3);
//	global_colors.AssignRandomAvailableColor(MOUSE_4);

	
	// initialize things...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
        args->tiled_display.print();
        glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);

	glutInitWindowPosition(20,20);
	glutCreateWindow("Puzzle");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyfunc);
	glutSpecialFunc(specialkeyfunc);
	glutMouseFunc(mousefunc);
	glutMotionFunc(motionfunc);

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
          std::cout << "FULL SCREENING" << std::endl;
          glutFullScreen();
	}

    // Set up the cursors
    Interaction::setupCursors(&(args->tiled_display), AddTrackedPoint, RemoveTrackedPoint );

    // REDUNDANT - CSS
    // Interaction::setGlobalPointTracker( new PointTracker(Interaction::getGlobalCalibrationData(), AddTrackedPoint, RemoveTrackedPoint,-1,0.5) );
    // This is redundant, it happens in setupLasers called from setupCursor inside Interaction
	//args->tiled_display.set_from_calibration_data(Interaction::getGlobalCalibrationData());
	
#ifndef __NO_SOUND__
	initsound(&soundcontroller);
#endif
	glutMainLoop();

	return 0;

}


// CHRIS STUETZLE
// TEMP 
//void TryToPressObj(int cursIndex) {}
void TryToReleaseObj(int cursIndex) {}
void TryToMoveObj(int cursIndex) {}
