// ==========================================================================================
// OPENGL INCLUDES
// ==========================================================================================

#include "../../../dynamic_projection/source/applications/paint/gl_includes.h"


#include <map>

// ==========================================================================================
// LOCAL INCLUDES
// ==========================================================================================


#include "../../multi_cursor/interaction.h"
#include "../../calibration/planar_interpolation_calibration/colors.h"

#include "../paint/text.h"


#include "argparser.h"
#include "PuzzlePiece.h"
#include "score.h"


// ==========================================================================================
// GLOBAL VARIABLES
// ==========================================================================================

ArgParser *args; // A global argument list
Colors global_colors; // A list of colors

std::vector<PuzzlePiece*> piece_data;
std::vector<PuzzlePiece*> piece_pointers;


int SPACING;
int X_BORDER;
int Y_BORDER;


int SNAP_GRID = 3;

double TILE_w;
double TILE_h;

bool completed = false;

int GLOBAL_total_time = 0;
int GLOBAL_fastest_time;
int GLOBAL_fewest_moves;
int GLOBAL_num_people;


std::map<int,Score> all_scores;


// ==========================================================================================
// Function Prototypes
// ==========================================================================================

//void prepare_strokes();
void draw_strokes(); 
void draw_grid();
void draw_scores();
void draw_cursors();
void draw_background_buttons();
void draw_pressed_buttons();

void TryToGrabObj( int cursIndex );
void TryToMoveObj( int cursIndex );
void TryToReleaseObj( int cursIndex );

// ==========================================================================================
// TRACKING ROUTINES (Eventually to be moved to Interaction)
// ==========================================================================================

void AddTrackedPoint(const TrackedPoint &pt) {
    int id = pt.getID();
    global_colors.AssignRandomAvailableColor(id);
}

void RemoveTrackedPoint(const TrackedPoint &pt) {
    int id = pt.getID();
    Vec3f color = global_colors.GetColor(id);
    global_colors.RemoveId(id);

    /*
      for (unsigned int i = 0; i < piece_pointers.size(); i++) {
        if (piece_pointers[i]->isPressed() && piece_pointers[i]->PressedBy() == id) {
          //button_pressed_by[i] = -1;
          ReleasePiece(i,id);

        }
      }
     */
}

// ==========================================================================================
// OPENGL ROUTINES
// ==========================================================================================

// This function handles errors collected during executions of OpenGL code

int HandleGLError(std::string foo) {
    GLenum error;
    int i = 0;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("GL ERROR(#%d == 0x%x):  %s\n", i, error, gluErrorString(error));
        std::cout << foo << std::endl;
        if (error != GL_INVALID_OPERATION) i++;
    }
    if (i == 0) return 1;
    return 0;
}


// ==========================================================================================
// GLUT ROUTINES
// ==========================================================================================

void draw() {

    static GLfloat amb[] = {0.4, 0.4, 0.4, 0.0};
    static GLfloat dif[] = {1.0, 1.0, 1.0, 0.0};

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

    std::sort(piece_pointers.begin(), piece_pointers.end(), piece_sorter);

    draw_grid();
    draw_background_buttons();
    //if (!args->tiled_display.is_tiled_display ||
    //	(args->tiled_display.is_tiled_display && args->tiled_display.is_master)) {
    //prepare_strokes();
    //}
    draw_strokes();
    draw_pressed_buttons();
    draw_cursors();
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

void display(void) {
    HandleGLError("BEFORE DISPLAY");
    draw();
    HandleGLError("AFTER DISPLAY");
}

void idle(void) {
    //    // If we have a tiled display
    //    if (!args->tiled_display.is_tiled_display ||
    //            (args->tiled_display.is_tiled_display && args->tiled_display.is_master)) {
    //        if (args->tiled_display.is_tiled_display && args->tiled_display.is_master) {
    //            SavePuzzleState(piece_data, PUZZLE_STATE_FILENAME);
    //        } // end of if
    //    }// end of if
    //    else {
    //        // THIS PROCESS IS NOT RESPONSIBLE FOR LASERS, JUST FOR VISUALIZING
    //        assert(args->tiled_display.is_tiled_display && !args->tiled_display.is_master);
    //        LoadPuzzleState(piece_data, PUZZLE_STATE_FILENAME);
    //    } // end of else

    display();
    usleep(1000);

} // end of idle function

void reshape(int w, int h) {
    HandleGLError("BEFORE RESHAPE");

    args->tiled_display.reshape(w, h);
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0, 1.0, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0.0, 0.0, 5.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.);
    glTranslatef(0.0, 0.6, -1.0);
    //	initialize_buttons();

    HandleGLError("AFTER RESHAPE");
} // end of reshape

// A helper to deal with multiple keyboards

void keyfunc_helper(int which_keyboard, unsigned char key, int x, int y, int glut_modifiers) {
    if (key == 'q') {
        exit(0);
    }//  if(key == 'b'){
        //    for(unsigned int i = 0; i < piece_pointers.size(); ++i){
        //      piece_pointers[i]->ResetBorder();
        //    }
        //  }
    else {
        std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
    }
    display();
} // end of keyfun_helper

void keyfunc(unsigned char key, int x, int y) {
    //    if (args->tiled_display.is_tiled_display) {
    //        log_keypress(PRIMARY_KEYBOARD, key, x, y, glutGetModifiers());
    //    } else {
    keyfunc_helper(PRIMARY_KEYBOARD, key, x, y, glutGetModifiers());
    //    }
} // end of keyfunc

void specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glut_modifiers) {

}

void specialkeyfunc(int key, int x, int y) {
    //    if (args->tiled_display.is_tiled_display) {
    //        log_specialkeypress(PRIMARY_KEYBOARD, key, x, y, glutGetModifiers());
    //    } else {
    specialkeyfunc_helper(PRIMARY_KEYBOARD, key, x, y, glutGetModifiers());
    //    }
}


// ==========================================================================================
// PUZZLE CALLBACK ROUTINES
// ==========================================================================================

Pt SnappedToGrid(const Pt &current) {



    // GET RID OF THIS
    return Pt();
} // end of SnappedToGrid

void TryToPressButton(int id, double x, double y) {


} // end of TryToPressButton

void TryToPressObj(int index) {

} // end of TryToPressObj

void TryToReleaseObj(int cursIndex) {

} // end of TryToReleaseObj

void TryToMoveObj(int cursIndex) {


} // end of TryToMoveObj


// ==========================================================================================
// MAIN FUNCTION
// ==========================================================================================

int main(int argc, char **argv) {
  args = new ArgParser(argc, argv);

  // Register callbacks
  //Interaction::registerExpand( expandNode );
  //Interaction::registerSimplify( collapseNode );
  Interaction::registerSelect( TryToGrabObj );
  Interaction::registerMove( TryToMoveObj );
  Interaction::registerRelease( TryToReleaseObj );
  //Interaction::registerGroup( TryToGroupObjs );
  //Interaction::registerUngroup( TryToUngroupObjs );

    // initialize things...
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    args->tiled_display.print();
    glutInitWindowSize(args->tiled_display.my_width, args->tiled_display.my_height);

    glutInitWindowPosition(20, 20);
    glutCreateWindow("Puzzle");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyfunc);
    glutSpecialFunc(specialkeyfunc);
    //    glutMouseFunc(mousefunc);
    //    glutMotionFunc(motionfunc);

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
    Interaction::setupCursors(&(args->tiled_display), AddTrackedPoint, RemoveTrackedPoint);
    /*
    #ifndef __NO_SOUND__
            initsound(&soundcontroller);
    #endif
     */
    glutMainLoop();

    return 0;

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

  std::cout << "num pieces " << piece_pointers.size() << std::endl;

  for (unsigned int i = 0; i < piece_pointers.size(); i++) {
    std::cout << "!";
    if (piece_pointers[i]->hasBorder()) continue;
    piece_pointers[i]->paint();
    std::cout << "b";
  }
}


void draw_pressed_buttons() {
  for (unsigned int i = 0; i < piece_pointers.size(); i++) {
    std::cout << "?";
    if (!piece_pointers[i]->hasBorder()) continue;
    piece_pointers[i]->paint();
    std::cout << "p";
  }
  //  std::cout << std::endl;
}


/*
void prepare_strokes() { //bool button_strokes) {

  //  assert (Interaction::getGlobalPointTracker() != NULL);
  //GLOBAL_strokes.clear();
    
  //  for (unsigned int i = 0; i < Interaction::getGlobalPointTracker()->size(); i++) {
  //int id = (*Interaction::getGlobalPointTracker())[i].getID();
//    bool pressing = false;
    // check if this tracker is attached to a button
//    for (unsigned int j = 0; j < piece_pointers.size(); j++) {
//      if (piece_pointers[j]->isPressed() && piece_pointers[j]->PressedBy() == id)
//        pressing = true;
//    }
    
//    Pt pt = (*Interaction::getGlobalPointTracker())[i].getCurrentPosition();
  //int laser = (*Interaction::getGlobalPointTracker())[i].getWhichLaser();
  //Vec3f color = global_colors.GetColor(id);
  //
  //    if (laser != -1) {
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
*/

void draw_strokes() {//bool button_strokes) {

  /*
  for (unsigned int i = 0; i < GLOBAL_strokes.size(); i++) {
    const Vec3f &color = GLOBAL_strokes[i].second;
    // SET COLOR
   glColor3f(color.x(),color.y(),color.z());
    const std::list<Pt> &trail = GLOBAL_strokes[i].first;
    // DRAW TRAIL
    glLineWidth(3);
    Path::draw_smooth_stroke(trail); //(*Interaction::getGlobalPointTracker())[i].getPtTrail());
    
    }*/

}




void grabHelper( ClickableObject* c, int cursIndex ) {
  Cursor *cur = Interaction::getCursor( cursIndex );
  // Set the node as pressed
  c -> SetPress( true );
  Pt center = c -> getCentroid();
  c->press(cur -> getColor(), cursIndex , c -> Offset(center) );
}

// When we try to "grab" an object
void TryToGrabObj( int cursIndex )
{
  // Get the associated cursor
  //Cursor *c = Interaction::getCursor( cursIndex );
  // Figure out which button is being clicked
  //  double radius = args->tiled_display.my_width/10.0;
  //  Pt center = c -> getWorldPosition();
  //  ClickableObject* new_affected = ((ClickableObject*)GLOBAL_graph -> ClickClosestObject(center,radius));
//  ClickableObject *new_affected = getAffectedObject( GLOBAL_graph, cursIndex );
/*
  GraphNode* closestObj = GLOBAL_graph -> ClickClosestObject(center, radius);

  else if( new_affected -> isPressed() )
    return; 

  if( new_affected -> getParentObjectGroup() )
    new_affected = new_affected -> getParentObjectGroup();
    
  // Now, associate them if the cursor mode is normal
  if( c -> isNormalMode() )
  {
    c -> setGrabbedObj( closestObj );
    std::string objType = "GraphNode";
    c -> setObjType( objType );
    
    grabHelper( new_affected, cursIndex );
  } // end of if
  else if( c -> isGroupUngroupMode() )
  {
    // Set the new_affected to "pressed" so it draws correctly
    grabHelper( new_affected, cursIndex );
    buttonsToBeGrouped.push_back( new_affected );
  } // end of else if
*/
} // end of tryToGrabObj

#if 0
// Helper to recursively move all objects
void moveHelper( ClickableObject* node, Pt offset )
{
  if( node -> isObjectGroup() )
  {
    ObjectGroup* o = static_cast<ObjectGroup*>( node );
    for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
    {
      moveHelper( o -> getNodeInObjectGroup( i ), offset );
    } // end of for
  } // end of if
  else
  {
    //node -> Move(newPos);
    GraphNode* n = dynamic_cast<GraphNode*>(node);
    Pt nodePos = n -> getPosition();
    Pt newPos = Pt( nodePos.x + offset.x, nodePos.y + offset.y );
    n -> setPosition( newPos.x, newPos.y );
  } // end of else
} // end of moveHelper
#endif

#if 0
// When we try to move a grabbed object
void TryToMoveObj( int cursIndex )
{
#if 0
//cerr << "Moving object with cursor " << cursIndex << endl;
  // Get the cursor
  Cursor *c = Interaction::getCursor(cursIndex);
  ClickableObject* o = ((ClickableObject*) c -> getGrabbedObj());
  if( !o ) return;
  std::string objType = c -> getObjType();
  
  // Get the distance to move
  Pt p = c -> getWorldPosition();
  // We know it's a GraphNode because only they can be grabbed right 
  GraphNode* node = dynamic_cast<GraphNode*>( o );
  Pt nodePos = node -> getPosition();
  Pt offset = Pt(p.x - nodePos.x, p.y - nodePos.y);
//  Pt newPos( nodePos.x + offset.x, nodePos.y + offset.y );
  
  // Move all the way up the graph node tree
//  while( o -> getParentObjectGroup() )
//    o = o -> getParentObjectGroup();
  if( o -> getParentObjectGroup() )
    o = o -> getParentObjectGroup();
  
  moveHelper( o, offset );
#endif
} // end of TryToMoveObj

#if 0
void releaseHelper( ClickableObject* c )
{
  c -> SetPress(false);
  
  if( c -> isObjectGroup() )
  {
    //<<<<<<< HEAD
    //Pt p = c -> getWorldPosition();
    //Pt offset = node -> getPressPosition();
    //Pt q = Pt(p.x-offset.x,p.y-offset.y);
    //node -> Move(q);
    //=======
    ObjectGroup* o = static_cast<ObjectGroup*>( c );
    for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
    {
      releaseHelper( o -> getNodeInObjectGroup( i ) );
    } // end of for
    //>>>>>>> d7253a429214fea3131c4edf8f75621bd5538cb5
  } // end of if
  // If it's not an object group, call expand
  else if( !( c -> isObjectGroup() ) )
  {
    // Release the node
    c -> release(4000);
  }
  else
    assert( false );
} // end of expandNodeHelper
#endif



// Try to release an object
void TryToReleaseObj( int cursIndex )
{
#if 0
//cerr << "Releasing object with cursor " << cursIndex << endl;
  Cursor *c = Interaction::getCursor(cursIndex);
  ClickableObject* objPtr = ((ClickableObject*)c->getGrabbedObj());
  std::string objType = c->getObjType();
  std::string nullType = "NULL";
  
  if( !objPtr ) return;
  
  // Move all the way up the graph node tree
  while( objPtr -> getParentObjectGroup() )
    objPtr = objPtr -> getParentObjectGroup();

  if(objPtr != NULL)
  {
    // Unset the grabbed object
    c -> setGrabbedObj( NULL );
    std::string objType = "NULL";
    c -> setObjType( objType );

    releaseHelper( objPtr );
  } // end of if
#endif
} // end of TryToReleaseObj
#endif
