
#include "interaction.h"
#include <fstream>

#include <glm/gtx/string_cast.hpp>


// Constants:
  // this wasn't working for mapview (possibly related to library??)
/*
const string Interaction::IR_STATE_DIRECTORY = "../state/ir_tracking";
const string Interaction::MK_STATE_DIRECTORY = "../state/mouse_and_keyboard";
const string Interaction::MK_ACTION_FILENAME_TEMPLATE = "../state/mouse_and_keyboard/actions_XXX.txt";
const string Interaction::FOUND_IR_POINTS_FILENAME = "../state/ir_tracking/found_ir_points.txt";
const string Interaction::PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME = "../state/ir_tracking/planar_calibration_geometry_data.txt";
const string Interaction::PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME = "../state/ir_tracking/planar_calibration_intensity_data.txt";
const int Interaction::default_laser_tail_length = 30;
const unsigned int Interaction::PIXEL_MOTION_TOLERANCE = 10;
*/

std::vector<Cursor*> Interaction::cursorVec;
std::vector<MultiMouse*> Interaction::multiMouseVec;
std::vector<Laser*> Interaction::laserVec;
TiledDisplay *Interaction::tiled_display;
PointTracker* Interaction::global_point_tracker;
PlanarCalibration* Interaction::global_calibration_data;
//DirLock Interaction::global_ir_dirlock(Interaction::IR_STATE_DIRECTORY);
DirLock Interaction::global_ir_dirlock(IR_STATE_DIRECTORY);
DirLock Interaction::global_mk_dirlock(MK_STATE_DIRECTORY);

// CHRIS STUETZLE
// Callback functions: One for each possible gesture, assigned in the application
void (*Interaction::moveFunc)(Cursor *c) = NULL;
void (*Interaction::selectFunc)(Cursor *c) = NULL;
void (*Interaction::expandFunc)(Cursor *c) = NULL;
void (*Interaction::simplifyFunc)(Cursor *c) = NULL;
void (*Interaction::groupFunc)(Cursor *c) = NULL;
void (*Interaction::ungroupFunc)(Cursor *c) = NULL;
void (*Interaction::changeFunc)(Cursor *c) = NULL;
void (*Interaction::releaseFunc)(Cursor *c) = NULL;
void (*Interaction::lockFunc)(Cursor *c) = NULL;
void (*Interaction::zoomFunc)(Cursor *c) = NULL;

// Initialization of laser members
std::map<int,Action> Interaction::LActions;

// List of colors
Colors Interaction::global_colors;

double Interaction::interface_scale = 1.0;
Pt Interaction::interface_offset(0,0);

//Tyler============
bool Interaction::invert_cursor_y = true;
//==================

GLuint Interaction::m_vao[2];
GLuint Interaction::m_vbo[2];
GLuint Interaction::m_outline_vbo[2];


//FUNCTION PROTOTYPES================================================== 


void dumbTest(int n);

void clamp_to_display(Pt &pt) { 
  int x = pt.x;
  int y = pt.y;
  x = std::max(0,std::min(x,Interaction::tiled_display->full_display_width));
  y = std::max(0,std::min(y,Interaction::tiled_display->full_display_height));
  pt = Pt(x,y);
}



//INITIALIZE CURSORS===================================================
void Interaction::setupMultiMice() {
  srand( time(NULL) );


  std::vector<glm::vec3> colors;

  // Add the primary mouse
  Vec3f yellow(0.9,0.7,0);
  MultiMouse *Main = new MultiMouse("m0", Pt(0,0), PRIMARY_MOUSE, PRIMARY_MOUSE_CURSOR_ID, yellow );
  cursorVec.push_back( Main );
  multiMouseVec.push_back( Main );
  
  for(int i=0; i<MULTIMOUSENUM; i++){
    
    Vec3f cursorColor = MULTIMICE_COLOR[i];
    // Repeated 12 times for 12 verticies for positions
    for( int j = 0; j < 12; j++ ){
        colors.push_back(glm::vec3(cursorColor.r(), cursorColor.g(), cursorColor.b()));
    }

    std::stringstream ssname; 
    ssname << "m" << i+2;
    std::string name = ssname.str();
    
    int cid = 1001 + i;
    
    MultiMouse *M = new MultiMouse(name, MULTIMICE_POS[i], MULTIMICE_ID[i], cid, MULTIMICE_COLOR[i]);
    cursorVec.push_back(M);
    multiMouseVec.push_back(M);
  } // for

  glGenVertexArrays(2, &m_vao[0]);
  glGenBuffers(2, &m_vbo[0]);
  glGenBuffers(2, &m_outline_vbo[0]);

  glBindVertexArray(m_vao[CursorVAO::SHAPE]);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CursorVBO::COLOR]);
  glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CursorVBO::POSITION]);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CursorVBO::COLOR]);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindVertexArray(m_vao[CursorVAO::OUTLINE]);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo[CursorVBO::POSITION]);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo[CursorVBO::COLOR]);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindVertexArray(0);

} // setupMultiMice

// Initializes and populates the Colors member object
void Interaction::setupColors()
{
  // Set up the colors for the lasers
  // std::vector<Vec3f> colors;
  // colors.push_back(Vec3f(1,1,1));  // 0 = white
  // colors.push_back(Vec3f(1,0,0));  // 1 = red
  // colors.push_back(Vec3f(0,1,0));  // 2 = green
  // colors.push_back(Vec3f(0,0,1));  // 3 = blue
  global_colors = Colors();
} // end of setupColors


// Set up the cursors (main mouse, multi-mice, and lasers)
void Interaction::setupCursors(TiledDisplay *td, void (*add)(TrackedPoint *pt), void (*remove)(TrackedPoint *pt)){
  setupColors();
  tiled_display = td;
  setupMultiMice();
  setupLasers(add, remove);

}


/***********************************
********MULTI_MOUSE FUNCTIONS ******
***********************************/

int Interaction::decodeMultiMice(int mouseId)
{
  for(int i=0; i<MULTIMOUSENUM; i++){
    if(multiMouseVec[i]->getMouseId() == mouseId){
      for(unsigned int j=0; j<cursorVec.size(); j++){
      if(cursorVec[j]->getId() == multiMouseVec[i]->getId()){
        return j;
      } // end of if
      } // end of for
    } // end of if
  } // end of for
  return -1;
}

//DRAW CURSORS==========================================================
void Interaction::drawCursors( bool screen_space, const Vec3f &background_color, glm::mat4 projection) 
{//double fdh){
  drawMultiMice( screen_space, projection );
  for( unsigned int i = 0 ; i < cursorVec.size() ; i++ ){
    // Skip drawing the main mouse, as it's up to the OS
//    if( cursorVec[ i ] -> getId() != PRIMARY_MOUSE_CURSOR_ID )
    //cursorVec[ i ] -> draw(screen_space,background_color);
  } 
}

void Interaction::drawMultiMice( bool screen_space, glm::mat4 projection ){
  std::vector<glm::vec2> positions;
  std::vector<glm::vec2> outline_pos;
  std::vector<glm::vec3> outline_color;

  for( unsigned int i = 0; i < multiMouseVec.size(); i++ ){
    std::vector<glm::vec2> cursorPosition = multiMouseVec[ i ] -> getCursorShape();
    for( unsigned int j = 0; j < cursorPosition.size(); j++ ){
      positions.push_back(cursorPosition[j]);  
    }

    std::vector<glm::vec2> cursorOutline = multiMouseVec[ i ] -> getCursorOutline();
    glm::vec3 cursorColor = multiMouseVec[ i ] -> getCursorOutlineColor();

    for( unsigned int j = 0; j < cursorOutline.size(); j++ ){
      outline_pos.push_back(cursorOutline[j]);
      outline_color.push_back(cursorColor);
    }
  }

  ShaderManager::BindProgram("cursor");
  glDepthFunc(GL_ALWAYS);

  std::vector<glm::mat4> matricies;
  glm::mat4 model_view = glm::mat4(1.0);
  matricies.push_back( model_view );
  ShaderManager::current_shader->BindUniform( "model_view_matrix", matricies );
  matricies.clear();

  matricies.push_back( projection );
  ShaderManager::current_shader->BindUniform( "projection_matrix", matricies );
  matricies.clear();

  glBindVertexArray(m_vao[CursorVAO::SHAPE]);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CursorVBO::POSITION]);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2), &positions[0], GL_DYNAMIC_DRAW);

  glDrawArrays(GL_TRIANGLES, 0, positions.size() );


  glBindVertexArray(m_vao[CursorVAO::OUTLINE]);
  glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo[CursorVBO::POSITION]);
  glBufferData(GL_ARRAY_BUFFER, outline_pos.size() * sizeof(glm::vec2), &outline_pos[0], GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_outline_vbo[CursorVBO::COLOR]);
  glBufferData(GL_ARRAY_BUFFER, outline_color.size() * sizeof(glm::vec3), &outline_color[0], GL_DYNAMIC_DRAW);

  glDrawArrays(GL_LINES, 0, outline_pos.size() );
  glDepthFunc(GL_LEQUAL);
}


void Interaction::drawCursorTrails( bool screen_space, const Vec3f &background_color, double seconds, timeval cur) 
{//double fdh){
  for( unsigned int i = 0 ; i < cursorVec.size() ; i++ )
    // Skip drawing the main mouse, as it's up to the OS
//    if( cursorVec[ i ] -> getId() != PRIMARY_MOUSE_CURSOR_ID )
    cursorVec[ i ] -> draw_trail(screen_space,background_color,seconds,cur);
}


// Draw the laser strokes
void Interaction::drawStrokes( bool button_strokes )
{
  assert (global_point_tracker != NULL);

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    int id = (*global_point_tracker)[i]->getID();
    bool pressing = false;
    // check if this tracker is attached to a button
//    for (unsigned int j = 0; j < GLOBAL_buttons.size(); j++) {
//      if (GLOBAL_buttons[j]->isPressed() && GLOBAL_buttons[j]->PressedBy() == id)
//        pressing = true;
//    }  // end of for
    if (!pressing && button_strokes) continue;
//    if (pressing && !button_strokes) continue;

    if (!(*global_point_tracker)[i]->IsActive()) continue;

    // SET COLOR
    Vec3f color = global_colors.GetColor(id);
    glColor3f(color.x(),color.y(),color.z());

    // DRAW TRAIL
    glLineWidth(3);
    Path::draw_smooth_stroke((*global_point_tracker)[i]->getPtTrail());
  }

} // end of drawStrokes

// this will get called eventually :)
void Interaction::keyfunc_helper(int which_keyboard, int key, int scancode, int action, int glfw_modifiers) {
  //std::cout << "INTERACTION MODULE RECEIVED KEY : " << which_keyboard << " '" << key << "' " << x     << " " << y << " " << glfw_modifiers << std::endl;
  if (key == 'q') {
    exit(0);
  } else if (key == 'o' || key == 'O') {
    // OPENING SCREEN

  } else if (key == 'p' || key == 'P') {
    // PUZZLE TIME!
    
  } else if (key == 's' || key == 'S') {
    // SOLUTION SCREEN

  } else {
    std::cout << "WARNING unknown key: '" << key << "'" << std::endl;
  }
}


/*
void Interaction::specialkeyfunc_helper(int which_keyboard, int key, int x, int y, int glfw_modifiers) {
  //std::cout << "INTERACTION MODULE RECEIVED SPECIAL KEY : " << which_keyboard << " '" << key << "' " << x << " " << y << " " << glfw_modifiers << std::endl;

}
*/

void Interaction::mousefunc_helper(int which_mouse, int button, int action, int mods)  
{
  int mouseIndex = -1;
  for( int i = 0 ; i < MULTIMOUSENUM ; i++ )
    if( which_mouse == multiMouseVec[i]->getMouseId() )
    {
      mouseIndex = i;
      break;
    } // end of if
  assert( mouseIndex > -1 );

  // First, get the cursor index
  int cursIndex = decodeMultiMice(which_mouse);
    
  /*
  // This is a hack to update the "position" of the primary mouse
  if( which_mouse == PRIMARY_MOUSE ) {
    cursorVec[ cursIndex ] -> setScreenPosition( Pt( x, y ) );
  }
  */

  // Get current time
  timeval curTime;
  gettimeofday(&curTime, NULL);
  long curTimeInMilliseconds = (curTime.tv_sec * 1000) + (curTime.tv_usec / 1000);

  ///* TODO FIXME HACK SHOULD BE UNCOMMENTED??? 

  // Deal with the glfw modifiers so all users have all necessary actions
  bool shiftPressed = mods & GLFW_MOD_SHIFT;
  bool ctrlPressed = mods & GLFW_MOD_CONTROL;
  bool altPressed = mods & GLFW_MOD_ALT;

  //bool shiftPressed = glfw_modifiers & GLFW_ACTIVE_SHIFT;
  //bool ctrlPressed = glfw_modifiers & GLFW_ACTIVE_CTRL;
  //bool altPressed = glfw_modifiers & GLFW_ACTIVE_ALT;
  bool my_left_click = 
    (button == GLFW_MOUSE_BUTTON_1 && !shiftPressed && !ctrlPressed && !altPressed);
  bool my_right_click = 
    (button == GLFW_MOUSE_BUTTON_2) ||
    (button == GLFW_MOUSE_BUTTON_1 && (shiftPressed || ctrlPressed || altPressed));
  
  // If the button state is down, update the cursor and call the select callback
  if (action == GLFW_PRESS) 
  {

    std::cout << "action is press" << std::endl;

    cursorVec[cursIndex]->setWhichButton(button);
    cursorVec[cursIndex]->setStateDown();
    
    // If it was a single left-click
    if( my_left_click ) 
      {
      // If it's a single click, and we're not in group mode
      //      if( curTimeInMilliseconds - timeOfPreviousClick[ mouseIndex ] > DOUBLE_CLICK_TIMEOUT )
      if( curTimeInMilliseconds - multiMouseVec[mouseIndex]->getTimeOfPreviousClick() > DOUBLE_CLICK_TIMEOUT )
        // Call the single-click callback
        if( selectFunc ) selectFunc( cursorVec[cursIndex]  );
    } // end of if

	
  } 
  else if( action == GLFW_RELEASE )
  {

    std::cout << "action is release" << std::endl;
    cursorVec[cursIndex]->setWhichButton(button);
    cursorVec[cursIndex]->setStateUp();
    

    //
    // I ADDED THIS, I DON'T KNOW WHY IT WASN'T HERE ALREADY???? (BARB)
    if( releaseFunc) releaseFunc( cursorVec[cursIndex] );
    //



    // If it was a left click
    if( my_left_click )
    {
      // If it's a double click
      if( curTimeInMilliseconds - multiMouseVec[mouseIndex]->getTimeOfPreviousClick() > DOUBLE_CLICK_TIMEOUT )
	//if( curTimeInMilliseconds - timeOfPreviousClick[ mouseIndex ] < DOUBLE_CLICK_TIMEOUT )
      {
        // If we're currently in group mode, call the group function
        if( cursorVec[ cursIndex ] -> isGroupUngroupMode() )
        {
          if( groupFunc ) groupFunc( cursorVec[cursIndex] );
        }
        else if( cursorVec[ cursIndex ] -> isNormalMode() )
        {
          if( expandFunc ) expandFunc( cursorVec[cursIndex] );
        }
      } // end of if
      else if( cursorVec[ cursIndex ] -> isNormalMode() )
      {
        if( releaseFunc ) releaseFunc( cursorVec[cursIndex] );
      }
        
      // Reset the time of previous click
      multiMouseVec[mouseIndex]->setTimeOfPreviousClick(curTimeInMilliseconds);
      //timeOfPreviousClick[ mouseIndex ] = curTimeInMilliseconds;
    
      //for WHEEL_UP motion
      if(button == 3){
      //for WHEEL_DOWN motion
      }else if(button == 4){
      }
    } // end of if 
    // Otherwise, if it was a right click
    else if( my_right_click ) 
    {
      // If it's a right click in group/ungroup mode, it's an ungroup action  
      if( cursorVec[ cursIndex ] -> isGroupUngroupMode() )
      {
        if( ungroupFunc ) ungroupFunc(cursorVec[cursIndex] );
      }
      else if( cursorVec[ cursIndex ] -> isNormalMode() )
      {
        if( simplifyFunc ) simplifyFunc(cursorVec[cursIndex] );
      } // end of else
    } // end of else if
  } // end of else if
  else
  {
    cursorVec[cursIndex]->setStateNull();
  } // end of if else

  // end of hack

} // end of mousefunc_helper

void Interaction::motionfunc_helper(int which_mouse, int x, int y) { //, int glfw_modifiers) {
  //std::cout << "INTERACTION MODULE RECEIVED MOTION : " << which_mouse << " " << x << " " << y <<       " " << glfw_modifiers << std::endl;
  
  // Get the id of the multimouse
  int cursIndex = decodeMultiMice(which_mouse);
  assert(cursIndex != -1);

  if( which_mouse == PRIMARY_MOUSE ) {
    
    // Update the position
    assert (cursIndex >= 0 && cursIndex < (int)cursorVec.size());
    cursorVec[cursIndex]->setScreenPosition(Pt(x,y));

  } else {

    // Update the position
    assert (cursIndex >= 0 && cursIndex < (int)cursorVec.size());
    cursorVec[cursIndex]->setScreenPosition(Pt(x,y));

  }

  // Call the associated motion callback
  if( moveFunc )  moveFunc(cursorVec[cursIndex]);

}

void Interaction::scrollfunc_helper(int which_mouse, double x, double y){
    int cursIndex = decodeMultiMice(which_mouse);
    assert(cursIndex != -1);

    if( y < 0 ){
        cursorVec[cursIndex] -> setScrollDown();
    }
    else if( y > 0 ){
        cursorVec[cursIndex] -> setScrollUp();
    }

    cursorVec[cursIndex]-> setScrollPosition( (int)y );

    if( zoomFunc ) zoomFunc(cursorVec[cursIndex]);
}


// This function determines which functions to be called based on what changes have occurred for each cursor
void Interaction::determineCallBacks() {
  dealWithKeyboardAndMice();
  dealWithLasers();
} 

void Interaction::dealWithKeyboardAndMice() {
  vector< KeyOrMouseAction* > KMActions;
  // Call the key log reader
  load_and_save_key_and_mouse_data(global_mk_dirlock, MK_ACTION_FILENAME_TEMPLATE, KMActions );

  // Now we have a list of actions, go through them
  for( unsigned int i = 0 ; i < KMActions.size() ; i++ )
  {
    //std::cout << "ACTION!" << std::endl;

    // The index of the current cursor
    int cursorIndex = decodeMultiMice( KMActions[ i ] -> which_device );

    // First, get the cursor associated with it
    int gesture = getGestureFromAction( KMActions[ i ] );
    switch( gesture )
    {
      case MOVE:
      {
        // Update the position (HACK: If not the main mouse)
        if( cursorIndex != 0 )
          updateCursorPosition( cursorIndex, KMActions[ i ] );
        // If it is the primary mouse, update its position
        else
          updateMainMouseCursorPosition( Pt( KMActions[i] -> x, KMActions[i] -> y ) );

        if( moveFunc ) moveFunc( cursorVec[cursorIndex] );
        break;
      }
      case ZOOM:
      {
        scrollfunc_helper( KMActions[ i ] -> which_device,
                           KMActions[ i ] -> x,
                           KMActions[ i ] -> y );
      }
      // If it's anything else, ask for help
      default:
        mousefunc_helper( KMActions[ i ] -> which_device, 
                          KMActions[ i ] -> button, 
                          KMActions[ i ] -> action, 
                          KMActions[ i ] -> glfw_modifiers );
        break;
    } // end of switch
  } // end of for
} // end of dealWithKeyboardAndMice

void Interaction::updateMainMouseCursorPosition( Pt p )
{
  GLOBAL_PRIMARY_MOUSE_POS = Pt( p.x, p.y );
  cursorVec[ 0 ] -> setScreenPosition( p );

} // updateMainMouseCursorPosition

void Interaction::updateCursorPosition( int cursorIndex, KeyOrMouseAction* km )
{
  Pt &pt = get_GLOBAL_MOUSE_Pt( km -> which_device );
// WHY IS THIS NEGATIVE Y?
  pt += Pt( RELATIVE_MOUSE_SCALE * km -> x, -RELATIVE_MOUSE_SCALE * km -> y );
  clamp_to_display(pt);

  // Update the cursor position
  //cursorVec[ cursorIndex ] -> updatePosition( Pt( km -> x, km -> y ) );
  //Tyler=====================
  if(!invert_cursor_y){
    //std::cout << "NOT INVERTED" << std::endl;
    cursorVec[ cursorIndex ] -> updatePosition( Pt( km -> x, km -> y ) );
  }else{
    //std::cout << "INVERTED" << std::endl;
    cursorVec[ cursorIndex ] -> updatePosition( Pt( km -> x, (-1*(km -> y)) ) );
  }
  //==========================
}

// For multimice, this function gets the gesture from the keyboard or mouse action
int Interaction::getGestureFromAction( KeyOrMouseAction* km )
{
  // Determine the gesture by the action
  // If there's a mouse motion and it's relative, return MOVE
  if( km -> mousemotion )// && km -> relative )
    return MOVE;
  // If there's a mouse action and it's a click, return SELECT
  else if( km -> mouseaction && km -> action == GLFW_PRESS )
    return SELECT;
  // If there's a mouse action and it's a release (not relative), return RELEASE
  else if( km -> mouseaction && km -> action == GLFW_RELEASE )
    return RELEASE;
  else if( km -> mousescroll )
    return ZOOM;
    
  assert(false);
} // end of getGestureFromAction

// ================================================================================================
// ================================================================================================
// LASER FUNCTIONS
// ================================================================================================
// ================================================================================================

/*
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

*/

/*
void AddTrackedPoint(const TrackedPoint &pt)
{
  int id = pt.getID();
  // everybody starts with the color white!
  Interaction::global_colors.AssignColor(id,0);
} // end of AddTrackedPoint

void RemoveTrackedPoint(TrackedPoint &pt)
{
  int id = pt.getID();

  Vec3f color = Interaction::global_colors.GetColor(id);
  Interaction::global_colors.RemoveId(id);

  std::map<int,Action>::iterator itr = Interaction::LActions.find(id);
  if (itr != Interaction::LActions.end()) {
    Interaction::LActions.erase(itr);
  }


  // should really store the button pointer inside of tracked point so we don't need to search for it!
  GLOBAL_graph->DropPressedBy(id);


} // end of RemoveTrackedPoint
*/



void AddTrackedPoint(TrackedPoint *pt) {
  //std::cout << "add tracked point " << pt->getID() << std::endl;
  assert (pt->getCursor() == NULL);
  //std::cout << "which laser = " << pt.getWhichLaser() << std::endl;
  // can't assign here (don't know which laser yet)
}


void AssignCursorToTrackedPoint(TrackedPoint *pt) {
  //std::cout << "assignCursorToTrackedPoint " << pt->getID() << std::endl;
  //std::cout << "which laser = " << pt->getWhichLaser() << std::endl;
  assert (pt->getCursor() == NULL);
  //int tracked_point_id = pt->getID();
  int laserID = pt->getWhichLaser();  // 0-MAX_LASERS-1 (i think)
  assert (laserID >= 0 && laserID < MAX_LASERS);
  assert (laserID != -1);

  Cursor *c = NULL;
  while (1) {
    assert (laserID >= 0 && laserID < MAX_LASERS);
    //    std::cout << "loop laserid = " << laserID << std::endl;
    int cid = laserID + 1001 + MULTIMOUSENUM;
    int curIndex = cid - 1000;

    c = Interaction::getCursor(curIndex);
    assert (c != NULL);
    if (c->getTrackedPoint() == NULL) break;

    laserID = (laserID+1)%MAX_LASERS;
    assert (laserID >= 0 && laserID < MAX_LASERS);
  }

  assert (c != NULL);
  assert (c->getTrackedPoint() == NULL);

  //std::cout << "ASSOCIATING  cursor " << c->getId() << " with pt " << pt->getID() << std::endl;

  /// make friends!
  c->setTrackedPoint(pt);
  pt->setCursor(c,laserID);

  assert (pt->getCursor() == c);
  assert (c->getTrackedPoint() == pt);
}


void RemoveTrackedPoint(TrackedPoint *pt) {
  //std::cout << "remove tracked point " << pt->getID() << std::endl;
  
  Cursor *c = pt->getCursor();
  assert (c != NULL);

  //std::cout << "SEPARATING PT " << pt->getID() << "   & cursor " << c->getId() << std::endl;
  
  assert (pt->getCursor() == c);
  // when commented out works with 1 laser

  assert (c->getTrackedPoint() != NULL);
  //if (c->getTrackedPoint() != pt) {
  // std::cout << "ACH WRONG TP" << pt->getID() << " vs   " << c->getTrackedPoint()->getID() << std::endl;
  //}
  assert (c->getTrackedPoint() == pt);

  pt->setCursor(NULL,-1);
  c->setTrackedPoint(NULL);

  assert (pt->getCursor() == NULL);
  assert (c->getTrackedPoint() == NULL);
}







void Interaction::setupLasers(void (*add)(TrackedPoint *pt), void (*remove)(TrackedPoint *pt)) {


  if (laserVec.size() != 0) {    
    std::cout << "ERROR! should not call this more than once!" << std::endl;
    assert (0);
  }

  // First, set up the array of laser IDs
  int curID = 401;
  // Setup the global calibration data and point tracker
  std::cout << " SETUP " << PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME << " " << PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME << std::endl;
  global_calibration_data = new PlanarCalibration(PLANAR_CALIBRATION_GEOMETRY_DATA_FILENAME,PLANAR_CALIBRATION_INTENSITY_DATA_FILENAME); 


  // HACK HACK HACK
  // the display size must match the calibration...  except it doesn't matter when we aren't using lasers
  if (0) {
    tiled_display -> set_from_calibration_data(global_calibration_data);
  } else {
    std::cout << "WARNING!  LASER SETUP TO TILED DISPLAY DISABLED!" << std::endl;
  }
  // HACK HACK HACK


  global_point_tracker = new PointTracker(global_calibration_data,AddTrackedPoint,RemoveTrackedPoint,default_laser_tail_length,0.5);
  // Create the cursors
  std::cout << "setupLasers before " << MAX_LASERS << " " << laserVec.size() << std::endl;
  laserVec.clear();
  std::cout << "setupLasers before " << MAX_LASERS << " " << laserVec.size() << std::endl;
  for( unsigned int i = 0 ; i < MAX_LASERS ; i++ ) {
    int cid = 1001 + MULTIMOUSENUM + i;
    //Laser* L = new Laser( laser_names[ i ], curID++, cid, MULTIMICE_COLOR[i] );
    Laser* L = new Laser( PlanarCalibration::laser_names[ i ], curID++, cid, Interaction::global_colors.GetColorFromAllColors(i));
    cursorVec.push_back( L );
    laserVec.push_back( L );
  } 
  std::cout << "setupLasers after " << MAX_LASERS << " " << laserVec.size() << std::endl;
  assert( global_calibration_data );
  assert( global_point_tracker );
} 



// This functions determines the actions created by the lasers
void Interaction::dealWithLasers() {
  ReadAndProcessLaserData();
  CheckForLaserActionInitializations();
  HandleLaserActions();  
} 


// This function reads and processes the laser data saved be other applications
void Interaction::ReadAndProcessLaserData() {
  while (!global_ir_dirlock.TryLock()) {
    usleep(1000);
  }
  std::vector<IR_Data_Point> raw_points;
  bool success = false;
  { /* SCOPE FOR istr */
    std::ifstream istr(FOUND_IR_POINTS_FILENAME); 
    //std::ifstream istr(FOUND_IR_POINTS_FILENAME.c_str());
    if (istr) {
      assert (istr);
      success = PointTracker::ReadIRPointData(istr,raw_points);
    } else {
      success = true;
    }
  } /* SCOPE FOR istr */
  global_ir_dirlock.Unlock();
  if (!success) {
    // new data not yet available
    usleep(1000);
    return;
  }
  global_point_tracker->ProcessPointData(raw_points);
}




// This functions assigns laser actions to each of the lasers
void Interaction::CheckForLaserActionInitializations() {

  /*
  std::cout << "CHECK FOR LASER ACTION INITIALIZATIONS   " 
	    << global_point_tracker->size()  
	    << std::endl;
  */  

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    
    TrackedPoint *tracked_point = (*global_point_tracker)[i];
    

    Cursor *c = tracked_point->getCursor();
    if (c == NULL) {
      AssignCursorToTrackedPoint(tracked_point);
    }
    c = tracked_point->getCursor();    
    assert  (c != NULL);
    int better_cid = c->getId();

    int tracked_point_id = tracked_point->getID();

    //std::cout << "cursor " << c->getId() << std::endl;
    TrackedPoint* pt = c->getTrackedPoint();
    //std::cout << "pt " << pt->getID() << std::endl;
    assert (pt != NULL);
    assert (pt->getCursor() != NULL);
    //std::cout << "pt is friends with " << pt->getCursor()->getId() << std::endl;
    assert (pt->getCursor() == c);

    int laserID = pt->getForcedLaser();
    //int laserID = pt->getWhichLaser(); //tracked_point.getWhichLaser();
    //std::cout << "laser id " << laserID << std::endl;

    assert (laserID != -1);
    assert (laserID >= 0 && laserID < MAX_LASERS);
    assert (laserVec.size() == MAX_LASERS);

    if (laserID != -1) {
      laserVec[laserID]->setNumFramesOff(0);
    }

    // If it's not active, set its action to up and continue
    if (!tracked_point->IsActive()) {
      std::cout << "NOT ACTIVE" << std::endl;
      continue;
    }
          
    
    // Always update the cursor position
    //std::cout << "curIndex = " << curIndex << std::endl;
    ///assert (curIndex >= 0 && curIndex < (int)cursorVec.size());


    //cursorVec[ curIndex ] -> setScreenPosition( (*global_point_tracker)[i].getCurrentPosition() );
    c -> setScreenPosition( (*global_point_tracker)[i]->getCurrentPosition() );
    

    // Get the "trail" of the laser, the last points
    const std::deque<Pt> &trail = (*global_point_tracker)[i]->getPtTrail();

    // Identify current action (if any)
    std::map<int,Action>::iterator action_itr = LActions.find(better_cid);
    Action *current_action = NULL;
    if (action_itr != LActions.end()) {
      assert (better_cid == action_itr->first);
      current_action = &action_itr->second;
    }
    
    // If we are already dragging, don't start a new action
    if (current_action != NULL &&
        current_action -> is_drag() &&
        c->getClickedObject() != NULL) {
      //cursorVec[curIndex]->getClickedObject() != NULL) {
      continue;
    }

    // Classify the current trail
    double radius;
    Pt center;
    action_classification ac = Classify(trail, PIXEL_MOTION_TOLERANCE, radius, center);

   
    // Prepare the new action
    Action *new_action = NULL;
    Action stupid; // to get around problem with "taking address of temporary"
    if (ac == EARLY_TRACKING || ac == OTHER_CLASSIFICATION) {
      global_colors.ReAssignColor(c->getId(),0);
   } else if (ac == STATIONARY) {
    // prepare for drag
      global_colors.ReAssignColor(c->getId(),2); // (pen color green)
      stupid = Action::Hover(radius,center,tracked_point_id);
      new_action = &stupid;
    } else if (ac == HORIZONTAL_Z_STRIKE ||
       ac == VERTICAL_Z_STRIKE ||
       ac == Z_STRIKE) {
      global_colors.ReAssignColor(c->getId(),1); // (pen color red)
      stupid = Action::ZStrike(radius, center,tracked_point_id);
      new_action = &stupid;
      (*global_point_tracker)[i]->clear_history();
    } else {
      assert (ac == CLOCKWISE_CIRCLE ||
       ac == COUNTERCLOCKWISE_CIRCLE);
      global_colors.ReAssignColor(c->getId(),3); // (pen color blue)
      stupid = Action::Circle(radius, center,tracked_point_id);
      new_action = &stupid;
      (*global_point_tracker)[i]->clear_history();
    }
  
    global_colors.ReAssignColor(c->getId(),c->getId()); // color by cursor ID
  // If nothing happens, move on
  if( current_action == NULL && new_action == NULL ) continue;
  
  // If the laser is no longer doing anything, erase the action
  if( current_action == NULL && new_action != NULL )
  {
    LActions[ better_cid ] = *new_action;
  } // end of if
  // If there is both a new action and an old action
  else if( current_action != NULL && new_action != NULL )
  {
    // If they are the same type of action, replace the old action with the new action
    if (current_action->same_action_type(*new_action))
    {


      // I DON'T UNDERSTAND THIS CODE (BARB)
      // I NEED TO COMMENT IT OUT (SHOULD NOT CALL RELEASE HERE)
      
      //releaseFunc( curIndex );



    } // end of if
    LActions[ better_cid ] = *new_action;
  } // end of else if


/*
    if (current_action == NULL && new_action == NULL) {
      //std::cout << "nothing before or now" << std::endl;
    } else if (current_action != NULL && new_action == NULL) {
      //std::cout << "action ceased" << std::endl;
      if (current_action->getNode() != NULL) {
        current_action->getNode()->release();
      }
      LActions.erase(action_itr);
    } else 
    if (current_action == NULL && new_action != NULL) {
      //std::cout << "action initiated" << std::endl;
      if (new_affected != NULL) {
        //std::cout << "action initiated by " << id << " " << new_affected->getName() << std::endl;
        new_action->setNode(new_affected);
        assert (new_affected != NULL);

        Pt pt = new_affected->Offset(center);
       new_affected->press(global_colors.GetColor(id),cid,pt);
 
      } // end of if
      LActions[id] = *new_action;
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
            } // end of if
          } // end of if
          LActions[id] = *new_action;
        } else if (current_action->getNode() == NULL) {
          //std::cout << "node now selected : " << id << " " << new_affected->getName() << std::endl;
          new_action->setNode(new_affected);
          assert (new_affected!= NULL);
          Pt pt = new_affected->Offset(center);
          new_affected->press(global_colors.GetColor(id),cid,pt);
          LActions[id] = *new_action;
        } else if (new_affected == NULL) {
          //std::cout << "node released: " << current_action->getNode()->getName() << std::endl;
          current_action->getNode()->release();
          LActions[id] = *new_action;
        } else {
          //std::cout << "node change  " << id << " " << current_action->getNode()->getName() << "->" << new_affected->getName() << std::endl;
          current_action->getNode()->release();
          new_action->setNode(new_affected);
          assert (new_affected != NULL);
          Pt pt = new_affected->Offset(center);
          new_affected->press(global_colors.GetColor(id),cid,pt);
          LActions[id] = *new_action;
        } // end of else if
      } else {
        if (current_action->getNode() != NULL) {
          current_action->getNode()->release();
        } // end of if
        new_action->setNode(new_affected);
        if (new_affected != NULL) {
          assert (new_affected != NULL);
          Pt pt = new_affected->Offset(center);
          new_affected->press(global_colors.GetColor(id),cid,pt);
        } // end of if
        LActions[id] = *new_action;
      } // end of else
    } // end of else
*/
  } // end of for
  
  // increase their "frames off" count
  //  std::cout << "check " << laserVec.size() << " " << MAX_LASERS << std::endl; 
  assert (laserVec.size() == MAX_LASERS);
  for( unsigned int i = 0 ; i < MAX_LASERS ; i++ ) {
    int numframes = laserVec[i]->getNumFramesOff();
    laserVec[i]->setNumFramesOff(numframes+1); 
    //    laserVec[laserID]->setNumFramesOff(0);
  } 
} 



// Once laser actions have been categorized, this function calls the appropriate callbacks
void Interaction::HandleLaserActions() {

  // Perform each action
  for( std::map<int,Action>::iterator itr = LActions.begin() ; itr != LActions.end() && LActions.size() > 0 ; ) {

    int cid = itr -> first;
    int curIndex = cid - 1000;
    // int laserIndex = curIndex - MULTIMOUSENUM - 1; 


    //unsigned long tracked_point_id = itr->second.getTrackedPointID();

    /*
    std::cout << " cid=" << cid 
	      << " curIndex=" << curIndex 
	      << " laserIndex=" << laserIndex 
	      << " tpi=" << tracked_point_id << std::endl;
    */


      Cursor *c = cursorVec[curIndex];
      assert (c != NULL);
      ClickableObject *ob = c->getClickedObject();

      int laserIndex = -1;
      if(c->getTrackedPoint()){
        laserIndex = c->getTrackedPoint()->getForcedLaser(); 
      }

      if (ob == NULL) {
      // GRAB A NEW OBJECT
      // if( cursorVec[ curIndex ] -> getClickedObject() == NULL ) {
      //std::cout << "SELECT";  fflush(stdout);

      //assert (cursorVec[curIndex]->getClickedObject() == NULL);

      if( selectFunc ) 
       selectFunc( cursorVec[curIndex] );

       ob = c->getClickedObject();

       if (ob != NULL) {
         cursorVec[ curIndex ] -> setStateDown();
       }


         itr++;
      //} 
       } else {
        assert (ob != NULL);

      //std::cout << "handling this laser " << laserIndex << " num frames off " << laserVec[laserIndex]->getNumFramesOff() << " vs discontinous:" <<
      //	DISCONTINUOUS_THRESHOLD << " inactive:" << INACTIVE_THRESHOLD << std::endl;

        Laser *l = NULL;
        if (laserIndex != -1)
         l = laserVec[laserIndex];

      // CHECK FOR DISCONTINUOUS LASER
       if (l == NULL || l->getNumFramesOff() > DISCONTINUOUS_THRESHOLD) {
	//std::cout << "RELEASE";  fflush(stdout);
         if( releaseFunc ) releaseFunc( cursorVec[curIndex] );
	// want to do this, but not available on all compilers yet:
	//   itr = LActions.erase(itr);
         std::map<int,Action>::iterator itr2 = itr;
         itr2++;
         LActions.erase(itr);
         itr = itr2;
       } else {
	// MOVE THE OBJECT
         assert ( cursorVec[ curIndex ] -> isStateDown() == true);
	//std::cout << "MOVE";  fflush(stdout);
         ClickableObject *ob = c->getClickedObject(); 
	//assert( cursorVec[ curIndex ] -> getClickedObject() );
         assert  (ob != NULL);
         if( moveFunc ) moveFunc( cursorVec[curIndex] );
         itr++;
       }
     }
   }
 }



void error_callback(int error, const char* description) {
  std::cerr << "ERROR CALLBACK: " << description << std::endl;
}

std::string WhichGLError(GLenum &error) {
  switch (error) {
  case GL_NO_ERROR:
    return "NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  case GL_STACK_UNDERFLOW:
    return "GL_STACK_UNDERFLOW";
  case GL_STACK_OVERFLOW:
    return "GL_STACK_OVERFLOW";
  default:
    return "OTHER GL ERROR";
  }
}

int HandleGLError(const std::string &message, bool ignore) {
  GLenum error;
  int i = 0;
  while ((error = glGetError()) != GL_NO_ERROR) {
    if (!ignore) {
      if (message != "") {
	std::cerr << "[" << message << "] ";
      }
      std::cerr << "GL ERROR(" << i << ") " << WhichGLError(error) << std::endl;
    }
    i++;
  }
  if (i == 0) return 1;
  if (!ignore) {
    //assert (0);
  }
  return 0;
}


/*

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

*/
