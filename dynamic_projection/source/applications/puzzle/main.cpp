// ==========================================================================================
// SYSTEM INCLUDES
// ==========================================================================================

#include "../paint/gl_includes.h"


#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <set>
#include <ctime>
#include <iomanip>

// ==========================================================================================
// LOCAL INCLUDES
// ==========================================================================================

#include "../../common/directory_locking.h"
#include "../../multi_cursor/interaction.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"

#include "../../../../remesher/src/vectors.h"

#include "../paint/text.h"

#include "puzzle_argparser.h"
#include "PuzzlePiece.h"
#include "score.h"

#include "../paint/button.h"
#include "../paint/ButtonManager.h"
#include "../paint/path.h"
#include "../paint/text.h"
#include "../../common/Image.h"


// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

PuzzleArgParser *args;
//Colors Interaction::global_colors;


// Change these to use ClickableObjects??
std::vector<PuzzlePiece*> piece_data;
std::vector<PuzzlePiece*> piece_pointers;

PuzzlePiece* GLOBAL_opening_screen_button = NULL;
PuzzlePiece* GLOBAL_solved_puzzle_button = NULL;

ButtonManager *GLOBAL_BUTTON_MANAGER;

int SPACING;
int X_BORDER;
int Y_BORDER;

int SNAP_GRID = 2;

double TILE_w;
double TILE_h;

bool completed = false;

//int GLOBAL_total_moves = 0;
int GLOBAL_total_time = 0;
int GLOBAL_fastest_time;
int GLOBAL_fewest_moves;
int GLOBAL_num_people;

std::map<int,Score> all_scores;


#define APPLICATIONS_STATE_DIRECTORY            "../state/applications"
#define PUZZLE_STATE_FILENAME             "../state/applications/puzzle_state.txt"
#define PUZZLE_HIGH_SCORES_FILENAME             "../state/applications/puzzle_high_scores.txt"

#define BORDER_THICKNESS 25

DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);


// Don't know what this is...
std::vector<std::pair<std::list<Pt>,Vec3f> > GLOBAL_strokes;

// Change this to all be stored within Cursor ... multi-mice
Pt mouse_location;
std::vector<Pt> positions;
std::vector<Pt> correctpositions;




// ==========================================================================================
// HELPER FUNCTIONS
// ==========================================================================================

void draw_grid();
void draw_scores();
void draw_cursors();
void draw_background_buttons();
void draw_pressed_buttons();

void initialize_buttons();
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


void keyfunc(GLFWwindow *window, int key, int scancode, int action, int modifiers);
void mousefunc(GLFWwindow *window, int button, int action, int modifiers);
void motionfunc(GLFWwindow *window, double x, double y);

void keyfunc_helper(GLFWwindow *window, int which_keyboard, int key, int scancode, int action, int modifiers);

void mousefunc_helper(GLFWwindow *window, int which_mouse, int button, int action, int modifiers);
void motionfunc_helper(GLFWwindow *window, int which_mouse, int x, int y);

void SavePuzzleState(const std::vector<PuzzlePiece*> &piece_data, const std::string &state_filename);
void LoadPuzzleState(std::vector<PuzzlePiece*> &buttons, const std::string &state_filename);

void check_for_correctness();


void TryToSelectObj( Cursor *c );
void TryToMoveObj( Cursor *c );
void TryToReleaseObj( Cursor  *c );


// ==========================================================================================
// DRAWING ROUTINES
// ==========================================================================================


void draw() { 
  HandleGLError("enter draw()");

  /*
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
  */
  
  std::sort(piece_pointers.begin(),piece_pointers.end(),piece_sorter);
  
  static bool first = true;
  if (!first && args->opening_screen_mode) {
    if(GLOBAL_opening_screen_button==NULL)
      std::cout<<"OPENING SCREEN BUTTON NULL";
    else
      GLOBAL_opening_screen_button->paint();
  } else if (first || args->puzzle_mode) {
    draw_grid();
    draw_background_buttons();
    timeval tv;
    gettimeofday(&tv,NULL);
    Interaction::drawCursorTrails(true,Vec3f(0,0,0),0.5,tv);
    draw_pressed_buttons();
    // these are the multimice cursors!
    //draw_cursors();
  } else {
    assert (args->solved_puzzle);
    GLOBAL_solved_puzzle_button->paint();
  }
  first = false;

  draw_scores();

  //glDisable(GL_LINE_SMOOTH);
  //  glDisable(GL_BLEND);
  
  /*
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);
  */
  
  //  glutSwapBuffers();
  HandleGLError("leaving draw()");  
}


void draw_cursors() {
  Vec3f color;
  glPointSize(20);
  glBegin(GL_POINTS);
  color = Interaction::global_colors.GetColor(PRIMARY_MOUSE);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,GLOBAL_PRIMARY_MOUSE_POS.y);

  color = Interaction::global_colors.GetColor(MOUSE_2);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_2_POS.x,GLOBAL_MOUSE_2_POS.y);
  color = Interaction::global_colors.GetColor(MOUSE_3);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_3_POS.x,GLOBAL_MOUSE_3_POS.y);
  color = Interaction::global_colors.GetColor(MOUSE_4);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_4_POS.x,GLOBAL_MOUSE_4_POS.y);

  color = Interaction::global_colors.GetColor(MOUSE_5);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_5_POS.x,GLOBAL_MOUSE_5_POS.y);
  color = Interaction::global_colors.GetColor(MOUSE_6);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_6_POS.x,GLOBAL_MOUSE_6_POS.y);
  glEnd();

}


void draw_grid() {
  
  int SPACING = 1;
  
  Pt pt,pt2;

  glColor3f(0.15,0.15,0.15);
  glBegin(GL_QUADS);
  for (int i = 0; i < args->cols; i++) {
    for (int j = 0; j < args->rows; j++) {
      glVertex2f(X_BORDER+i*(TILE_w+SPACING),       Y_BORDER+j*(TILE_h+SPACING));
      glVertex2f(X_BORDER+i*(TILE_w+SPACING)+TILE_w,Y_BORDER+j*(TILE_h+SPACING));
      glVertex2f(X_BORDER+i*(TILE_w+SPACING)+TILE_w,Y_BORDER+j*(TILE_h+SPACING)+TILE_h);
      glVertex2f(X_BORDER+i*(TILE_w+SPACING),       Y_BORDER+j*(TILE_h+SPACING)+TILE_h);
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

}



static double elapsed_time = 0.0;

void draw_scores() {

  static time_t start = time(NULL);
  static time_t last = time(NULL);
  // COLLECT THE CURRENT TIME
  double diff;
  double diffstart = difftime(last,start);


  if (!completed) {
    time_t current = time(NULL);
    diff = difftime(current,last);
    diffstart = difftime(current,start);
    if (diff >= 0.1) {
      last = current;
      if (args->puzzle_mode) {
	elapsed_time += diff;
      }
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
  if (args->puzzle_mode) {
    drawstring(0.25*w, 0.95*h, tmp.str().c_str(), Vec3f(1,1,1), 0.4*w,0.05*h);
  }


  if (args->puzzle_mode) {
    int current_total_moves = 0;
    for (std::map<int,Score>::iterator itr = all_scores.begin(); itr != all_scores.end(); itr++) {
      current_total_moves += itr->second.getGood() + itr->second.getNeutral() + itr->second.getBad();
    }

    
    // DISPLAY CURRENT TIME
    tmp.str("");
    //  GLOBAL_total_time = int(diffstart);
    GLOBAL_total_time = int(elapsed_time);
    //std::cout << "TT " << GLOBAL_total_time << " " << elapsed_time << std::endl;

    tmp << "current: ";
    //  make_nice_time(tmp,diffstart);
    
    make_nice_time(tmp,elapsed_time); //diffstart);
    tmp << " in " << current_total_moves << " moves";
    drawstring(0.75*w, 0.95*h, tmp.str().c_str(), Vec3f(1,1,1), 0.4*w,0.05*h);
    
#if 0
    int count = 0;
    int num_players = all_scores.size();
    for (std::map<int,Score>::iterator itr = all_scores.begin(); itr != all_scores.end(); itr++) {
      tmp.str("");
      if (itr->first == 201) {
	tmp << "mouse:";
      } else {
	tmp << itr->first << ":";
      }
      pretty_colors_move_quality(tmp.str(), itr->second.getGood(), itr->second.getNeutral(), itr->second.getBad(), count, num_players);      count++;
    }
#endif
    
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

void display(void) {
  HandleGLError("BEFORE DISPLAY");
  draw();
  HandleGLError("AFTER DISPLAY");
}


// ==========================================================================================
// TRACKING ROUTINES
// ==========================================================================================





// ==========================================================================================
// IDLE
// ==========================================================================================

void idle(void) {
  
  //  std::cout << "idle " << std::endl;

  // Update the gestures, call the appropriate callback
  Interaction::determineCallBacks();


  if (!args->tiled_display.is_tiled_display ||
      (args->tiled_display.is_tiled_display && args->tiled_display.is_master)) {

    nudge_toward_grid();
    // FINISH THE PUZZLE?
    check_for_correctness();
    
    if (args->tiled_display.is_tiled_display && args->tiled_display.is_master) {      
      SavePuzzleState(piece_data, PUZZLE_STATE_FILENAME);
    }
  } 
  else {

    // THIS PROCESS IS NOT RESPONSIBLE FOR LASERS, JUST FOR VISUALIZING
    assert (args->tiled_display.is_tiled_display && !args->tiled_display.is_master);
    LoadPuzzleState(piece_data, PUZZLE_STATE_FILENAME);
  } 
  //display();
  usleep(100);
}


#define HOVER_THRESHOLD 10



Pt SnappedToGrid(const Pt &current) {

  double di = (current.x-X_BORDER) / (TILE_w+SPACING) ;
  double dj = (current.y-Y_BORDER) / (TILE_h + SPACING) ; //;
  // make sure it doesn't move too far off the board
  if (di < -1.0) di = -1.0;
  if (dj < -1.0) dj = -1.0;
  if (di > args->cols) di = args->cols;
  if (dj > args->rows) dj = args->rows;
  
  double i2 = int(SNAP_GRID*(di+5)+0.5) / double(SNAP_GRID) - 5;
  double j2 = int(SNAP_GRID*(dj+5)+0.5) / double(SNAP_GRID) - 5;
  
  double closest_x = X_BORDER+i2*(TILE_w+SPACING);
  double closest_y = Y_BORDER+j2*(TILE_h+SPACING);
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



void TryToSelectObj( Cursor *c ) {

  std::cout << "SELECT OB" << std::endl;

  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;


  //std::cout << "select w/ " << identified_cursor << std::endl;
  //Cursor *c = Interaction::getCursor( unique_track_id );  
  //Pt center = c -> getWorldPosition();

  Pt center = c->getWorldPosition();
  
  std::cout << center;

  ClickableObject* ob = c->getClickedObject();

  assert (ob == NULL);

  std::cout << "SIZE " << piece_pointers.size() << " " << piece_data.size() << std::endl;

  for (unsigned int j = 0; j < piece_pointers.size(); j++) {
    if (piece_pointers[j]->isPressed()) {
      assert (piece_pointers[j]->PressedBy() != c->getId());
    }
  }

  // look for an object to click
  for (int j = int(piece_pointers.size())-1; j >= 0; j--) {

    //std::cout << "trying " << j << std::endl;
    PuzzlePiece *b2 = piece_pointers[j];
    if (b2->PointInside(center)) { 
      // don't steal from someone else!
      if (b2->isPressed()) { 
	std::cout << " Can't have piece " << j << " it is pressed by " << b2->PressedBy() << std::endl;
	continue; 
      }
      assert (!piece_pointers[j]->isPressed());
        
      //std::cout << "PRESS THIS BUTTON!" << j << "  " << b2->getCorrectPoint() << std::endl;
      Pt pt = b2->Offset(center);


      //std::cout << "why is this red? " << Interaction::global_colors.GetColor(c->getId()) << std::endl;

      b2->press(c->getColor(),c->getId(),pt);
      //b2->press(Interaction::global_colors.GetColor(c->getId()),c->getId(),pt);


      b2->OnPress();
      //c->setClickedObject(b2);
      c->setClickedObject(b2);

      /*
      // -- sanity
      ClickableObject* ob = c->getClickedObject();
      assert (ob != NULL);
      PuzzlePiece *p = dynamic_cast<PuzzlePiece*>(ob);
      assert (p != NULL);
      assert (p->isPressed());
      assert (p->PressedBy() == unique_track_id);
      // --
      */

      break;
    }
  }


  // confirm that this cursor is not attached to any clickable object!
  ob = c->getClickedObject();
  if (ob == NULL) {
    std::cout << "FAILED TO SELECT AN OBJECT (that's ok)" << std::endl;
  }


  for(unsigned int k = 0; k < piece_pointers.size(); ++k)
    piece_pointers[k]->ResetBorder();
}



void TryToMoveObj( Cursor *c ) {


  //Cursor *c = Interaction::getCursor( unique_track_id );  
  //Pt center = c -> getWorldPosition();

  Pt center = c->getWorldPosition();
  ClickableObject* ob = c->getClickedObject();

  if (ob == NULL) {
    //std::cout << "that was not a left click??" << std::endl;
    return;
  }

  //  std::cout << "MOVE"; fflush(stdout);

  // -- sanity
  //ClickableObject* ob = c->getClickedObject();
  assert (ob != NULL);
  PuzzlePiece *p = dynamic_cast<PuzzlePiece*>(ob);
  assert (p != NULL);
  assert (p->isPressed());
  assert (p->PressedBy() == c->getId());
  // --



  if (args->tiled_display.is_tiled_display && !args->tiled_display.is_master) return;

  // check if this tracker is already attached to a button
  for (unsigned int j = 0; j < piece_pointers.size(); j++) {
    if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == c->getId()) {
      Button *b2 = piece_pointers[j];
      assert (p == b2);
      Pt offset = piece_pointers[j]->getPressPosition();
      Pt q = center-offset;
      b2->Move(q);
      return;
    }
  }
  //  std::cout << "OH WELL, NOT PRESSING A BUTTON! " << std::endl;
}






void TryToReleaseObj( Cursor *c ) {

  std::cout << "release OB" << std::endl;

  ClickableObject* ob = c->getClickedObject();
  if (ob == NULL) {
    //std::cout << "that was not a left click??" << std::endl;
    return;
  }
  assert (ob != NULL);

  PuzzlePiece *p = dynamic_cast<PuzzlePiece*>(ob);
  assert (p != NULL);

  //std::cout << "RELEASE THIS BUTTON!" << p->getCorrectPoint() << std::endl;

  assert (p->isPressed());
  assert (p->PressedBy() == c->getId());

  p->release();
  if (!completed) {
    p->OnRelease(all_scores[c->getIdentifiedCursor()]);
  }
  
  c->setClickedObject(NULL);
  //c->setClickedObject(NULL);
}





void corner_check(){

  enum position{center=0, topleft=1, left=2, botleft=3, bot=4,
		botright=5, right=6, topright=7, top=8};
  
  for(unsigned int i = 0; i < piece_pointers.size(); ++i) {      
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
    



  }
  
}


// ===================================================================
// MOUSE FUNCTIONS


void reshape(GLFWwindow *window, int w, int h) {

  
  std::cout << "RESHAPE WITH: " << std::endl;
  args->tiled_display.print();



  HandleGLError("BEFORE RESHAPE");
  args->tiled_display.reshape(w,h);
  //args->width = w;
  //args->height = h;
  //

  /*
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0, 1.0, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0.0, 0.0, 5.0,
    0.0, 0.0, 0.0,
    0.0, 1.0, 0.);
    glTranslatef(0.0, 0.6, -1.0);
  */

  assert (piece_pointers.size() == piece_data.size());

  /*
    for(unsigned int i = 0; i < piece_pointers.size(); ++i)
    piece_pointers[i]->ResetBorder();
  */

  HandleGLError("AFTER RESHAPE");
}






void keyfunc(GLFWwindow *window, int key, int scancode, int action, int mods) {
  //void keyfunc(unsigned char key, int x, int y) {
  //  std::cout << "RECEIVED KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
  //if (args->tiled_display.is_tiled_display) {
  //log_keypress(PRIMARY_KEYBOARD,key,scancode,action,mods);
  //} else {
  keyfunc_helper(window, PRIMARY_KEYBOARD,key,scancode,action,mods);
  //}
}

/*
  void specialkeyfunc(int key, int x, int y) {
  //  std::cout << "RECEIVED SPECIAL KEY DIRECTLY: '" << key << "' " << x << " " << y << std::endl;
  if (args->tiled_display.is_tiled_display) {
  log_specialkeypress(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  } else {
  specialkeyfunc_helper(PRIMARY_KEYBOARD,key,x,y,glutGetModifiers());
  }
  }
*/



////////////////////////////////////////////////////////////////////////////////////////////////////

void resetPuzzle();
// this will get called eventually :)
void keyfunc_helper(GLFWwindow *window, int which_keyboard, int key, int scancode, int action, int glut_modifiers) {
  //  std::cout << "RECEIVED KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glut_modifiers << std::endl;
  if (key == 'q' || key == 'Q') {
    exit(0);
  } else if (key == 'o' || key == 'O') {
    args->opening_screen_mode = true;
    args->puzzle_mode = false;
    args->solved_puzzle = false;
  } else if (key == 'p' || key == 'P') {
    args->opening_screen_mode = false;
    args->puzzle_mode = true;
    args->solved_puzzle = false;
  } else if (key == 's' || key == 'S') {
    args->opening_screen_mode = false;
    args->puzzle_mode = false;
    args->solved_puzzle = true;
  } else if(key == 'r' || key=='R'){
    resetPuzzle();
    if(completed){
      display();
      resetPuzzle();
    }
  }else {
    std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
  display();
}


/*
  void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers) {
  //  std::cout << "RECEIVED SPECIAL KEY FROM LOGGER: " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glut_modifiers << std::endl;

  }
*/


void mousefunc(GLFWwindow *window, int button, int action, int mods) {
  //void mousefunc(int button, int state, int x, int y) {
  //if (args->tiled_display.is_tiled_display) {
  //int from_top = args->tiled_display.full_display_height - 
  //  (args->tiled_display.my_height+args->tiled_display.my_bottom);
  ///y += from_top;
  // adjust position of click
  //log_mouseaction(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
  //} else {
  //y = args->tiled_display.full_display_height - y - 1;
  //assert (y >= 0 && y < args->tiled_display.full_display_height);
  mousefunc_helper(window, PRIMARY_MOUSE,button,action,mods);
  //}
}

void motionfunc(GLFWwindow *window, double x, double y) {
  if (args->tiled_display.is_tiled_display) {
    if (x == 0 && y == 0) {
      std::cout << "IGNORE: OFFSCREEN MOTION BUG" << std::endl;
      return;
    }
    //log_mousemotion(PRIMARY_MOUSE,x,y);
  } else {
    y = args->tiled_display.full_display_height - y - 1;

    // HACK?  Why is this claming necessary?
    if (y < 0) 
      y = 0;
    if (y >= args->tiled_display.full_display_height)
      y = args->tiled_display.full_display_height-1;
    
    assert (y >= 0 && y < args->tiled_display.full_display_height);
    motionfunc_helper(window, PRIMARY_MOUSE,x,y);
  }
}


void mousefunc_helper(GLFWwindow *window, int which_mouse, int button, int action, int modifiers) {
  /*
  if (action == GLFW_PRESS) {
    std::cout << "SHOULD PRESS!" << std::endl;
  }
  if (action == GLFW_RELEASE) {
    std::cout << "SHOULD RELEASE!" << std::endl;
  }
  */
  log_mouseaction(PRIMARY_MOUSE,button,action,modifiers);
}

void motionfunc_helper(GLFWwindow *window, int which_mouse, int x, int y) {
  log_mousemotion(PRIMARY_MOUSE,x,y); 
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

const int hackSeeds[] = {4, 8, 15, 17, 19, 23, 9001, 518};
const int NUMHACKSEEDS = 8;


void initialize_buttons() {
  printf(" INITIALIZING BUTTONS\n " );

  args->tiled_display.print();

  srand(time(NULL));

  int whichSeed = rand() % NUMHACKSEEDS;
  srand(whichSeed);

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

  image.load(args->image_filename.c_str());
  //image.load_from_file(args->image_filename.c_str());

  int iw = image.getCols();
  int ih = image.getRows();


  LoadHighScores(PUZZLE_HIGH_SCORES_FILENAME, args->image_filename.c_str(),
                 args->cols, args->rows,
                 GLOBAL_fastest_time, GLOBAL_fewest_moves, GLOBAL_num_people);

	
  int useable_screen_width = args->tiled_display.full_display_width - 2*X_BORDER - (args->cols-1)*SPACING;
  int useable_screen_height = args->tiled_display.full_display_height - 2*Y_BORDER - (args->rows-1)*SPACING;

  std::cout << "USEABLE " << useable_screen_width << " " << useable_screen_height << std::endl;

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
    SNAP_GRID = 2;
  }

  std::cout << "SNAP GRID = " << SNAP_GRID << std::endl;

  positions.clear();
  correctpositions.clear();
  std::vector<Pt> text_coords;

  double du = 1.0 / args->cols;
  double dv = 1.0 / args->rows;


  for (int i = 0; i < args->cols; i++) {
    for (int j = 0; j < args->rows; j++) {
      //positions.push_back(Pt(X_BORDER+i*(TILE_w+SPACING),SPACING+args->tiled_display.full_display_height-(Y_BORDER+(j+1)*(TILE_h+SPACING))));
      positions.push_back(Pt(X_BORDER+i*(TILE_w+SPACING),
                             Y_BORDER+j*(TILE_h+SPACING)));

      // flipp needed here because something is wrong about the neighbor correctness check???
      int j2 = args->rows-j-1;

      text_coords.push_back(Pt(i*du,j2*dv));
    }
  }

  correctpositions = positions;

  GLOBAL_opening_screen_button = new PuzzlePiece(Pt(X_BORDER,Y_BORDER),
                                                 (args->tiled_display.full_display_width-2*X_BORDER-(args->cols-1)*SPACING),
                                                 (args->tiled_display.full_display_height-2*Y_BORDER-(args->rows-1)*SPACING),
                                                 args->opening_image_filename,
                                                 0,0,
                                                 1,1);


  GLOBAL_solved_puzzle_button = new PuzzlePiece(Pt(X_BORDER,Y_BORDER),
                                                (args->tiled_display.full_display_width-2*X_BORDER-(args->cols-1)*SPACING),
                                                (args->tiled_display.full_display_height-2*Y_BORDER-(args->rows-1)*SPACING),
                                                args->image_filename,
                                                0,0,
                                                1,1);
  for (int i = 0; i < 9; i++) {
    GLOBAL_opening_screen_button->SetBorderVal(i,1);
    GLOBAL_solved_puzzle_button->SetBorderVal(i,1);
  }

#if 1
  std::random_shuffle(positions.begin(),positions.end());
  std::random_shuffle(positions.begin(),positions.end());
#endif

  for (unsigned int p = 0; p < positions.size(); p++)
    {
      PuzzlePiece *tmp = new PuzzlePiece(positions[p],TILE_w,TILE_h,args->image_filename,
                                         text_coords[p].x,text_coords[p].y,
                                         text_coords[p].x+du,text_coords[p].y+dv);

      std::cout << "TILE_w (" << p << ") " << TILE_w << " " << TILE_h << " " << positions[p] << std::endl;

      piece_data.push_back(tmp);
      tmp->setCorrectPoint(correctpositions[p]);
      piece_pointers.push_back(tmp);

      GLOBAL_BUTTON_MANAGER->AddButton(tmp);
      tmp->enable_texture();
    }



}

void resetPuzzle(){
  elapsed_time = 0.0;
  completed = false;
  int whichSeed = rand() % NUMHACKSEEDS;
  srand(whichSeed);

  std::random_shuffle(positions.begin(),positions.end());
  std::random_shuffle(positions.begin(),positions.end());

  piece_data.clear();
  for (unsigned int p = 0; p < positions.size(); p++)
    {
      // piece_data.push_back(new PuzzlePiece(positions[p],TILE_w,TILE_h,args->image_filename,
      //  text_coords[p].x,text_coords[p].y,
      //  text_coords[p].x+du,text_coords[p].y+dv));
      piece_data[p]->MoveNoDampingCentroid(positions[p]);
      piece_pointers[p]->ResetBorder();
    }
}





void SavePuzzleState(const std::vector<PuzzlePiece*> &buttons, const std::string &state_filename) {
  //std::cout << "TRY TO SAVE" << std::endl;

  static unsigned int frame_counter = 0;
  frame_counter++;
  
  while (!global_app_dirlock.TryLock()) { usleep(100); }

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
  
  while (!global_app_dirlock.TryLock()) { usleep(100); }

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

GLFWwindow* GLOBAL_window = NULL;



int  main(int argc, char **argv) {

  args = new PuzzleArgParser(argc,argv);



  Interaction::registerSelect( TryToSelectObj );
  Interaction::registerMove( TryToMoveObj );
  Interaction::registerRelease( TryToReleaseObj );



  glfwSetErrorCallback(error_callback);

  // Initialize GLFW
  if( !glfwInit() ) {
    std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
    exit(1);
  }
  

  // We will ask it to specifically open an OpenGL 3.2 context
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a GLFW window
  
  GLOBAL_window = glfwCreateWindow(args->tiled_display.my_width,
                                   args->tiled_display.my_height,
                                   "Puzzle!", NULL, NULL);
  if (!GLOBAL_window) {
    std::cerr << "ERROR: Failed to open GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(GLOBAL_window);
  HandleGLError("in glcanvas first");




  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "ERROR: Failed to initialize GLEW" << std::endl;
    glfwTerminate();
    exit(1);
  }

  // there seems to be a "GL_INVALID_ENUM" error in glewInit that is a
  // know issue, but can safely be ignored
  HandleGLError("after glewInit()",true);

  std::cout << "-------------------------------------------------------" << std::endl;
  std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << '\n';
  std::cout << "-------------------------------------------------------" << std::endl;

  // Initialize callback functions
  // Only set callbacks if you are not ignoring the primary mouse
  if( !args->ignore_primary_mouse )
  {
    glfwSetCursorPosCallback(GLOBAL_window,motionfunc);
    glfwSetMouseButtonCallback(GLOBAL_window,mousefunc);  
  }
  glfwSetKeyCallback(GLOBAL_window,keyfunc);
  glfwSetWindowSizeCallback(GLOBAL_window,reshape);


  /*
  programID = LoadShaders(  args->path+"/"+args->shader_filename+".vs",
                            args->path+"/"+args->shader_filename+".fs");

  GLCanvas::initializeVBOs();
  GLCanvas::setupVBOs();
  */



  /*
  // initialize things...
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  //args->tiled_display.print();
  glutInitWindowSize(args->tiled_display.my_width,args->tiled_display.my_height);
  
  glutInitWindowPosition(20,20);
  glutCreateWindow("Puzzle");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyfunc);
  //glutSpecialFunc(specialkeyfunc);
  glutMouseFunc(mousefunc);
  glutMotionFunc(motionfunc);
  */

  /*  
  GLfloat light0_ambient[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat light0_diffuse[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat light1_diffuse[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat light1_position[] = {1.0, 1.0, 1.0, 0.0};
  GLfloat light2_diffuse[] = {0.0, 1.0, 0.0, 1.0};
  GLfloat light2_position[] = {-1.0, -1.0, 1.0, 0.0};
  */

  /*
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
  */
  
  HandleGLError("main 1");
  
  /*
  if (args->tiled_display.full_screen) {
    std::cout << "FULL SCREENING" << std::endl;
    glutFullScreen();
  }
  */



  GLOBAL_BUTTON_MANAGER = new ButtonManager(args);



  ShaderManager::Initialize(args);



  Interaction::setupCursors(&(args->tiled_display));
  
  //glutMainLoop();
  
  //reshape(args->tiled_display.my_width, args->tiled_display.my_height);

  initialize_buttons();


  ShaderManager::CreateProgram("textured_rectangle");
  ShaderManager::CreateProgram("flat_circle",true);
  ShaderManager::CreateProgram("flat_rectangle");
  ShaderManager::CreateProgram("text");

  Shader* shader = ShaderManager::GetProgram("textured_rectangle");
  shader->InitUniform("model_view_matrix");
  shader->InitUniform("projection_matrix");
  shader->InitUniform("texture_sampler");

  shader = ShaderManager::GetProgram("flat_rectangle");
  shader->InitUniform("model_view_matrix");
  shader->InitUniform("projection_matrix");

  shader = ShaderManager::GetProgram("flat_circle");
  shader->InitUniform("model_view_matrix");
  shader->InitUniform("projection_matrix");

  while (!glfwWindowShouldClose(GLOBAL_window))  {

    idle();

    glm::mat4 projection_matrix = args->tiled_display.getOrtho();
    glm::mat4 model_view_matrix = glm::mat4(1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLOBAL_BUTTON_MANAGER->Draw(model_view_matrix, projection_matrix);

    // Swap buffers
    glfwSwapBuffers(GLOBAL_window);
    fflush(stdout);
    glfwPollEvents();
    fflush(stdout);
  }
  
  return 0;
  
}


