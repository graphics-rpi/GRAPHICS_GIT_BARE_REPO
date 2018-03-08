// GraphInteraction_WithCursors

#include "../paint/gl_includes.h"


/***********************
INCLUDED FILES
***********************/
//Standard includes
#include <string>
#include <iostream>
#include <vector>
// Debugging includes
#include <cassert>
// Files from other projects
#include "../../multi_cursor/interaction.h"
#include "../../common/directory_locking.h"
// From this directory
#include "argparser.h"
#include "graph.h"
#include "object_group.h"

/***********************
GLOBAL FILE NAMES
***********************/
#define APPLICATIONS_STATE_DIRECTORY      "../state/applications/"
#define GRAPH_INTERACTION_STATE_FILENAME  "../state/applications/graph_interaction_state.txt"

/***********************
NAMESPACE USES
***********************/
using std::cout;
using std::endl;
using std::string;
using std::cerr;
using std::vector;

/***********************
GLOBAL VARIABLES
***********************/
ArgParser *args;  // Handles input arguments
// The directory locking mechanism
DirLock global_app_dirlock(APPLICATIONS_STATE_DIRECTORY);
// A vector of nodes to be grouped

vector<ClickableObject*> buttonsToBeGrouped;
//vector<ObjectGroup*> curGroups;

static std::vector<std::string> GLOBAL_mouse_targets;
static std::vector<int> GLOBAL_mouse_wait;


Graph *GLOBAL_graph;
timeval GLOBAL_pause_time;


Pt GLOBAL_LOWERLEFT;
Pt GLOBAL_UPPERRIGHT;


/***********************
FUNCTIONS
***********************/


#define PADDING 50


void ChangeWithMaxSpeed(double &a, double new_a, int diffTime) {

  double cap = 2; //diffTime / 2.0;

  //  std::cout << "cap " << cap << std::endl;

  if (fabs(new_a-a) > cap ) {
    if (new_a > a) {
      a += cap;
    } else {
      a -= cap;
    }
  } else {
    a = new_a;
  }

}


void ChangeWithMaxSpeed(Pt &a, const Pt &new_a, int diffTime) {
  ChangeWithMaxSpeed(a.x,new_a.x,diffTime);
  ChangeWithMaxSpeed(a.y,new_a.y,diffTime);
}


void drawGrid() {


/*
  Pt lowerleft(1000000,1000000);
  //Pt lowerleft(0,0);
  Pt upperright(-1000000,-1000000);


  

  unsigned int numCursors = Interaction::getNumCursors();
  for (unsigned int i = 0; i < numCursors; i++) {

    // get the cursor
    Cursor* cursor = Interaction::getCursor(i);

    // don't move the primary mouse
    

    // JUST FOR NOW!
    if (cursor->getId() == PRIMARY_MOUSE_CURSOR_ID) continue;


    if (!cursor->isMouse()) continue;
    
      const std::list<std::pair<timeval,Pt> >& screen_position_trail = cursor->getScreenPositionTrail();

    
    for (std::list<std::pair<timeval,Pt> >::const_iterator itr = screen_position_trail.begin();
         itr != screen_position_trail.end(); itr++) {
      
      timeval tmp = itr->first;
      long tmpTimeInMilliseconds = (tmp.tv_sec * 1000) + (tmp.tv_usec / 1000);
      long pauseTimeInMilliseconds = (GLOBAL_pause_time.tv_sec * 1000) + (GLOBAL_pause_time.tv_usec / 1000);
      int diff = (pauseTimeInMilliseconds - tmpTimeInMilliseconds);


      if (diff > 1000*args->draw_trails_seconds) continue;
      Pt curs_pos = ScreenToWorld(itr->second);

      lowerleft.x = std::min(lowerleft.x,curs_pos.x);
      lowerleft.y = std::min(lowerleft.y,curs_pos.y);
      upperright.x = std::max(upperright.x,curs_pos.x);
      upperright.y = std::max(upperright.y,curs_pos.y);
    }
    
    Pt curs_pos = cursor->getWorldPosition();
    lowerleft.x = std::min(lowerleft.x,curs_pos.x);
    lowerleft.y = std::min(lowerleft.y,curs_pos.y);
    upperright.x = std::max(upperright.x,curs_pos.x);
    upperright.y = std::max(upperright.y,curs_pos.y);

  }

  double boxh = upperright.y-lowerleft.y + PADDING;
  double boxw = upperright.x-lowerleft.x + PADDING;

  Pt center = (lowerleft + upperright) * 0.5;

  double fdh = args->tiled_display.full_display_height;
  double fdw = args->tiled_display.full_display_width;
  
  if (boxh/boxw >= fdh/fdw) {
    boxw = boxh * fdw/fdh;
  } else {
    assert (boxh/boxw <= fdh/fdw);
    boxh = boxw * fdh/fdw;
  }

  assert (fabs(boxh/boxw - fdh/fdw) < 0.001);

  Pt lowerleft2 = center - 0.5*Pt(boxw,boxh);
  Pt upperright2 = center + 0.5*Pt(boxw,boxh);
*/

  /*  //  glColor3f(0.5,0.5,0.5);
  glLineWidth(6);
  glBegin(GL_LINES);
  glVertex2f(lowerleft2.x,lowerleft2.y);
  glVertex2f(lowerleft2.x,upperright2.y);
  glVertex2f(lowerleft2.x,upperright2.y);
  glVertex2f(upperright2.x,upperright2.y);

  glVertex2f(upperright2.x,upperright2.y);
  glVertex2f(upperright2.x,lowerleft2.y);
  glVertex2f(upperright2.x,lowerleft2.y);
  glVertex2f(lowerleft2.x,lowerleft2.y);
  glEnd();
  */




  if (args->draw_grid) {


#if 0 
    glColor3f(0.4,0.4,0.4); //1,0,0);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_UPPER_RIGHT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_UPPER_RIGHT.y);
    glVertex2f(GLOBAL_UPPER_RIGHT.x,GLOBAL_UPPER_RIGHT.y);
    
    glVertex2f(GLOBAL_UPPER_RIGHT.x,GLOBAL_UPPER_RIGHT.y);
    glVertex2f(GLOBAL_UPPER_RIGHT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_UPPER_RIGHT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_LOWERLEFT.y);
    glEnd();
#endif

    Vec3f tmp = args->background_color*0.8 + 0.2 *Vec3f(1,0,0);
    
    glColor3f(tmp.r(),
              tmp.g(),
              tmp.b()); //1,0.5,0.5); /y/0.4,0.4,0.4); //1,0,0);


#if 0
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_UPPERRIGHT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_UPPERRIGHT.y);
    glVertex2f(GLOBAL_UPPERRIGHT.x,GLOBAL_UPPERRIGHT.y);
    
    glVertex2f(GLOBAL_UPPERRIGHT.x,GLOBAL_UPPERRIGHT.y);
    glVertex2f(GLOBAL_UPPERRIGHT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_UPPERRIGHT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_LOWERLEFT.y);
    glEnd();
#else
    glBegin(GL_QUADS);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_LOWERLEFT.y);
    glVertex2f(GLOBAL_LOWERLEFT.x,GLOBAL_UPPERRIGHT.y);
    glVertex2f(GLOBAL_UPPERRIGHT.x,GLOBAL_UPPERRIGHT.y);
    glVertex2f(GLOBAL_UPPERRIGHT.x,GLOBAL_LOWERLEFT.y);
    glEnd();
#endif


  }

}

void calculateZoom() {


  static timeval lastTime;
  timeval curTime;
  gettimeofday(&curTime, NULL);
  long lastTimeInMilliseconds = (lastTime.tv_sec * 1000) + (lastTime.tv_usec / 1000);
  long curTimeInMilliseconds = (curTime.tv_sec * 1000) + (curTime.tv_usec / 1000);
  if (lastTimeInMilliseconds == curTimeInMilliseconds) return;
  int diffTime = curTimeInMilliseconds - lastTimeInMilliseconds;
  lastTime = curTime;
  if (diffTime > 10000) {
    std::cout << "HUGE TIME DIFF " << diffTime << std::endl;
    return;
  }

  Pt lowerleft(1000000,1000000);
  //Pt lowerleft(0,0);//1000000,1000000);
  Pt upperright(-1000000,-1000000);

  unsigned int numCursors = Interaction::getNumCursors();
  for (unsigned int i = 0; i < numCursors; i++) {

    // get the cursor
    Cursor* cursor = Interaction::getCursor(i);

    // don't move the primary mouse
    if (cursor->getId() == PRIMARY_MOUSE_CURSOR_ID) continue;
    if (!cursor->isMouse()) continue;
    
    const std::list<std::pair<timeval,Pt> >& world_position_trail = cursor->getWorldPositionTrail();

    
    for (std::list<std::pair<timeval,Pt> >::const_iterator itr = world_position_trail.begin();
         itr != world_position_trail.end(); itr++) {
      
      timeval tmp = itr->first;
      long tmpTimeInMilliseconds = (tmp.tv_sec * 1000) + (tmp.tv_usec / 1000);

      long pauseTimeInMilliseconds = (GLOBAL_pause_time.tv_sec * 1000) + (GLOBAL_pause_time.tv_usec / 1000);
      
       int diff = (pauseTimeInMilliseconds - tmpTimeInMilliseconds);


       if (diff > 1000*args->draw_trails_seconds) continue;
       

       Pt curs_pos = itr->second; //ScreenToWorld(itr->second);
       lowerleft.x = std::min(lowerleft.x,curs_pos.x);
       lowerleft.y = std::min(lowerleft.y,curs_pos.y);
       upperright.x = std::max(upperright.x,curs_pos.x);
       upperright.y = std::max(upperright.y,curs_pos.y);



    }

       Pt curs_pos = cursor->getWorldPosition();
       lowerleft.x = std::min(lowerleft.x,curs_pos.x);
       lowerleft.y = std::min(lowerleft.y,curs_pos.y);

       upperright.x = std::max(upperright.x,curs_pos.x);
       upperright.y = std::max(upperright.y,curs_pos.y);
  }

  double boxh = upperright.y-lowerleft.y + PADDING;
  double boxw = upperright.x-lowerleft.x + PADDING;

  Pt center = (lowerleft + upperright) * 0.5;

  double fdh = args->tiled_display.full_display_height;
  double fdw = args->tiled_display.full_display_width;
  
  if (boxh/boxw >= fdh/fdw) {
    boxw = boxh * fdw/fdh;
  } else {
    assert (boxh/boxw <= fdh/fdw);
    boxh = boxw * fdh/fdw;
  }

  assert (fabs(boxh/boxw - fdh/fdw) < 0.001);

  Pt lowerleft2 = center - 0.5*Pt(boxw,boxh);
  //  Pt upperright2 = center + 0.5*Pt(boxw,boxh);  /// not used?


  GLOBAL_LOWERLEFT = lowerleft;
  GLOBAL_UPPERRIGHT = upperright;

  Interaction::interface_offset = lowerleft2;
  Interaction::interface_scale = boxh / fdh;


  Cursor* primary_mouse = NULL;
  for (unsigned int i = 0; i < Interaction::getNumCursors(); i++) {
    Cursor* m = Interaction::getCursor(i);
    if (m->getId() == PRIMARY_MOUSE_CURSOR_ID) {
      primary_mouse = m;
      break;
    }
  }
  assert (primary_mouse != NULL);


}


// This function is responsible for setting up lighting and actually drawing everything on the canvas
void draw()
{

  calculateZoom();
  if (!args->crop_zoom) {
    Interaction::interface_offset = Pt(0,0);
    Interaction::interface_scale = 1.0;
  }

  // Handle the GL Error before drawing anything
  HandleGLError("BEFORE draw()");

  // Clear the color buffer
  glClearColor(args->background_color.r(),args->background_color.g(),args->background_color.b(),1);
  glClear(GL_COLOR_BUFFER_BIT);

  //  if (args->draw_trails) {
  
  if (!args->pause) {
    gettimeofday(&GLOBAL_pause_time,NULL);
  }

  //  Interaction::drawCursorTrails(true,args->background_color, args->draw_trails_seconds, GLOBAL_pause_time);
    //}

  // Reload the ortho projection
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  if (!args->crop_zoom) {
    args->tiled_display.ORTHO();
  } else {

    double scale = Interaction::interface_scale;

    double height = args->tiled_display.full_display_height*scale;
    double width = args->tiled_display.full_display_width*scale;
    
    Pt offset = Interaction::interface_offset;

    int left = offset.x;
    int right = offset.x + width;
    int top = offset.y + height;
    int bottom = offset.y;


    gluOrtho2D(left,
	       right,
	       bottom,
	       top);
  }

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  // Handle GL errors that occurred in the projections
  HandleGLError("BEFORE drawgraph()");
  
  drawGrid();
  Interaction::drawCursorTrails(false,args->background_color, args->draw_trails_seconds, GLOBAL_pause_time);


  GLOBAL_graph->drawedges();
  Interaction::drawStrokes( true );
  GLOBAL_graph->drawnodes();
  Interaction::drawStrokes( false );

  HandleGLError("AFTER drawstrokes()");


  // Draw the cursors
  //  Interaction::drawCursors(false);


  // AGAIN
  // Reload the ortho projection
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  args->tiled_display.ORTHO();
  // Draw the cursors
  Interaction::drawCursors(true,args->background_color);


  HandleGLError("AFTER draw()");

} // end of draw


//====================================================================================
// INTERACTION CALLS======================================================
//====================================================================================

ClickableObject* getAffectedObject(Graph *graph, Cursor *c) { //int cursIndex ) {

  // Get the node affected by the cursor
  //Cursor* c = Interaction::getCursor( cursIndex );
// GET THIS FIXED...SUPPOSED TO BE "RADIUS", WHICH I ASSUME IS THE SIZE OF THE BUTTONS
// THIS IS A HACK!!!
  double radius = args->tiled_display.my_width/10.0;


  Pt center = c -> getWorldPosition();
  ClickableObject* o = ((ClickableObject*)graph->ClickClosestObject(center,radius));
  if( !o ) return NULL;
  // Move all the way up the graph node tree
  while( o -> getParentObjectGroup() )
    o = o -> getParentObjectGroup();
  
  return o;

}

// A recursive version of the expandNode function
void expandNodeHelper( ClickableObject* c )
{
  if( c -> isObjectGroup() )
  {
    ObjectGroup* o = static_cast<ObjectGroup*>( c );
    for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
    {
      expandNodeHelper( o -> getNodeInObjectGroup( i ) );
    } // end of for
  } // end of if
  // If it's not an object group, call expand
  else if( !( c -> isObjectGroup() ) )
    // Changed to dynamic_cast from static_cast
    GLOBAL_graph->Expand( dynamic_cast<GraphNode*>( c ) );
  else
    assert( false );
} // end of expandNodeHelper


// When a double click, expand the node
void expandNode( Cursor *c ) //int cursIndex )
{
  cerr << "Expanding object with cursor " << c->getId() << endl; //cursIndex << endl;
  // Returns the top level of the group tree
 ClickableObject *new_affected = getAffectedObject( GLOBAL_graph, c ); //ursIndex );

  // If there's nothing to do, return
  if( !new_affected ) return;

  // If it's part of a group, call the helper to deal with all of the other nodes in the group
  expandNodeHelper( new_affected );
} // end of expandNode


// A recursive version of the collapseNode function
void collapseNodeHelper( ClickableObject* c )
{
  if( c -> isObjectGroup() )
  {
    ObjectGroup* o = static_cast<ObjectGroup*>( c );
    for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
    {
      collapseNodeHelper( o -> getNodeInObjectGroup( i ) );
    } // end of for
  } // end of if
  // If it's not an object group, call expand
  else if( !( c -> isObjectGroup() ) )
  {
    if( !dynamic_cast<GraphNode*>( c ) -> isVisible() ) return;
    // Changed to dynamic_cast from static_cast
    GLOBAL_graph->Collapse( dynamic_cast<GraphNode*>( c ) );
  }
  else
    assert( false );
} // end of expandNodeHelper


// When a right click, collapse the node
void collapseNode( Cursor *c ) //int cursIndex )
{
//cerr << "Collapsing object with cursor " << cursIndex << endl;
  // Returns the top level of the group tree
  ClickableObject *new_affected = getAffectedObject( GLOBAL_graph, c); //ursIndex );

  // If there's nothing to do, return
  if( !new_affected ) return;

  // If it's part of a group, call the helper to deal with all of the other nodes in the group
  collapseNodeHelper( new_affected );
} // end of collapseNode

void grabHelper( ClickableObject* c, Cursor *cur )//int cursIndex )
{
  //Cursor *cur = Interaction::getCursor( cursIndex );
  // Set the node as pressed
  c -> SetPress( true );
  if( c -> isObjectGroup() )
  {
    ObjectGroup* o = static_cast<ObjectGroup*>( c );
    // Call this helper recursively
    for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
    {
      grabHelper( o -> getNodeInObjectGroup( i ), cur); //cursIndex );
    } // end of for
  } // end of if
  else
  {
    Pt center = c -> getCentroid();
    c->press(cur -> getColor(), cur->getId() /*cursIndex*/ , c -> Offset(center) );
  } // end of else
} // end of grabHelper

// When we try to "grab" an object
void TryToGrabObj( Cursor *c ) //int cursIndex )
{
//cerr << "Grabbing object with cursor " << cursIndex << endl;
  // Get the associated cursor
  //Cursor *c = Interaction::getCursor( cursIndex );

  // Figure out which button is being clicked
  double radius = args->tiled_display.my_width/10.0;
  Pt center = c -> getWorldPosition();
  ClickableObject* new_affected = ((ClickableObject*)GLOBAL_graph -> ClickClosestObject(center,radius));
//  ClickableObject *new_affected = getAffectedObject( GLOBAL_graph, cursIndex );

  GraphNode* closestObj = GLOBAL_graph -> ClickClosestObject(center, radius);

  // If the user pressed into nothingness, change the mode of the cursor
  if( !new_affected )
  {
    c -> toggleMode();
    return;
  } // end of if
  // If the object is already grabbed, then ignore
  else if( new_affected -> isPressed() )
    return; 

  if( new_affected -> getParentObjectGroup() )
    new_affected = new_affected -> getParentObjectGroup();
    
  // Now, associate them if the cursor mode is normal
  if( c -> isNormalMode() )
  {
    c -> setClickedObject ( closestObj );
    //std::string objType = "GraphNode";
    //c -> setObjType( objType );
    
    grabHelper( new_affected, c ); //ursIndex );
  } // end of if
  else if( c -> isGroupUngroupMode() )
  {
    // Set the new_affected to "pressed" so it draws correctly
    grabHelper( new_affected, c ); //ursIndex );
    buttonsToBeGrouped.push_back( new_affected );
  } // end of else if
} // end of tryToGrabObj


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

// When we try to move a grabbed object
void TryToMoveObj( Cursor *c ) //int cursIndex )
{
//cerr << "Moving object with cursor " << cursIndex << endl;
  // Get the cursor
  //Cursor *c = Interaction::getCursor(cursIndex);
  ClickableObject* o = c -> getClickedObject();
  if( !o ) return;
  //  std::string objType = c -> getObjType();
  
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
} // end of TryToMoveObj


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




// Try to release an object
void TryToReleaseObj( Cursor *c ) //int cursIndex )
{
//cerr << "Releasing object with cursor " << cursIndex << endl;
  //Cursor *c = Interaction::getCursor(cursIndex);
  ClickableObject* objPtr = c->getClickedObject();
  //  std::string objType = c->getObjType();
  //std::string nullType = "NULL";
  
  if( !objPtr ) return;
  
  // Move all the way up the graph node tree
  while( objPtr -> getParentObjectGroup() )
    objPtr = objPtr -> getParentObjectGroup();

  if(objPtr != NULL)
  {
    // Unset the grabbed object
    c -> setClickedObject(NULL);
    //GrabbedObj( NULL );
    //std::string objType = "NULL";
    //c -> setObjType( objType );

    releaseHelper( objPtr );
  } // end of if
} // end of TryToReleaseObj


// Groups any objects that are in the v
void TryToGroupObjs( Cursor *c ) //int cursIndex )
{
//cerr << "Grouping objects with cursor " << cursIndex << endl;
  //Cursor *c = Interaction::getCursor(cursIndex);
// CHANGE THE NAMING SCHEME!
  // Create a new ObjectGroup object, and populate it
  ObjectGroup* newGroup = new ObjectGroup(buttonsToBeGrouped);

  // Unpress all pressed buttons, and assign the group to the parent
  for( unsigned int i = 0 ; i < buttonsToBeGrouped.size() ; i++ )
  {
    releaseHelper( buttonsToBeGrouped[ i ] );
    buttonsToBeGrouped[ i ] -> setParentObjectGroup( newGroup );
  } // end of for

  c -> setModeNormal();
  buttonsToBeGrouped.clear();
} // Groups objects that are to be grouped

// Helper recursive function for ungroup
void ungroupHelper( ClickableObject* c )
{
  if( c -> isObjectGroup() )
  {
    ObjectGroup* o = static_cast<ObjectGroup*>( c );
    for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
    {
      ungroupHelper( o -> getNodeInObjectGroup( i ) );
    } // end of for
  } // end of if
  // If it's not an object group, call expand
  else if( !( c -> isObjectGroup() ) )
    // Changed to dynamic_cast from static_cast
    c -> setParentObjectGroup( NULL );
  else
    assert( false );
} // end of expandNodeHelper


void TryToUngroupObjs( Cursor *c) //nt cursIndex )
{
//cerr << "Ungrouping objects with cursor " << cursIndex << endl;
  // First, get the affected group
  //Cursor *c = Interaction::getCursor(cursIndex);
   
//  ClickableObject *new_affected = getAffectedObject( GLOBAL_graph, cursIndex );
  double radius = args->tiled_display.my_width/10.0;
  Pt center = c -> getWorldPosition();
  ClickableObject* new_affected = ((ClickableObject*)GLOBAL_graph -> ClickClosestObject(center,radius));
  if( !new_affected ) return;
  new_affected = new_affected -> getParentObjectGroup();
  if( new_affected == NULL ){cerr << "Returning!" << endl; return; }
  
  // Ungroup all of the nodes
  ObjectGroup* o = static_cast<ObjectGroup*>(new_affected);
  for( unsigned int i = 0 ; i < o -> numNodesInObjectGroup() ; i++ )
  {
    o -> getNodeInObjectGroup( i ) -> setParentObjectGroup( NULL );
  } // end of for

  o -> clearObjectGroup();
  c -> setModeNormal();
}

/************************
**** Laser Callbacks ****
************************/

/*
void AddTrackedPoint(const TrackedPoint &pt)
{
  int id = pt.getID();
  // everybody starts with the color white!
  Interaction::global_colors.AssignColor(id,0);
} // end of AddTrackedPoint

void RemoveTrackedPoint(const TrackedPoint &pt)
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


/***********************
GL CALLBACKS
***********************/

// The display call, called each frame
void display(void) 
{
  HandleGLError("BEFORE DISPLAY");
  draw();
  HandleGLError("AFTER DISPLAY");
} // end of display


// The keyboard call bak if a button is pressed
void keyboard(unsigned char key, int x, int y) 
{
  if (key == 'q') {
    std::cout << "exiting normally" << std::endl;
    exit(0);
  } else if (key == 'p') {
    args->pause = !args->pause;
  } else if (key == 'e') {
    GLOBAL_graph->Expand();
  } else if (key == 'c') {
    GLOBAL_graph->Collapse();
  } else if (key == 't') {
    args->draw_trails_seconds += 0.5;
    if (args->draw_trails_seconds > 10.0)
      args->draw_trails_seconds = 10.0;
    std::cout << "trail length " << args->draw_trails_seconds << std::endl;
  } else if (key == 'T') {
    args->draw_trails_seconds -= 0.5;
    if (args->draw_trails_seconds < 0)
      args->draw_trails_seconds = 0;
    std::cout << "trail length " << args->draw_trails_seconds << std::endl;
  } else if (key == 'r') {
    GLOBAL_mouse_targets.clear();
    GLOBAL_graph->RandomizeGraph();
  } else if (key == 'z') {    
    args->crop_zoom = !args->crop_zoom;
  } else if (key == 'g') {    
    args->draw_grid = !args->draw_grid;
  } else if (key == 'm') {    
    args->randomly_move_multimice = !args->randomly_move_multimice;
  } else if (key == 'w') {    
    args->background_color = Vec3f(1,1,1) - args->background_color;
  } else {
    std::cout << "unknown key '" << key << "'" << std::endl;
  }
  display();
  glutPostRedisplay();
} // end of keyboard


// The mouse call back if the mouse button is clicked
void mouse(int button, int state, int x, int y) 
{
  // adjust position of click
  int from_top = args->tiled_display.full_display_height - 
      (args->tiled_display.my_height+args->tiled_display.my_bottom);
  y += from_top;

  // HACK TO FIX THE Y (This might not work on the tiled display)
  y = args -> tiled_display.full_display_height - y;
  
  log_mouseaction(PRIMARY_MOUSE,button,state,x,y,glutGetModifiers());
} // end of mouse

// The mouse motion callback
void motion(int x, int y)
{
  // adjust position of click
  int from_top = args->tiled_display.full_display_height - 
      (args->tiled_display.my_height+args->tiled_display.my_bottom);
  y += from_top;
  y = args -> tiled_display.full_display_height - y;
  // THIS IS A HACK TO CHANGE THE Y-COORDINATE
  
  //std::cout << "a" << std::endl;
  log_mousemotion(PRIMARY_MOUSE,x,y); //,glutGetModifiers());
  //std::cout << "b" << std::endl;
} // end of motion

void passiveMotion( int x, int y )
{
  // Just call motion
  motion( x, y );
}


void RandomlyMoveMultimice() {

  static timeval lastTime;
  timeval curTime;
  gettimeofday(&curTime, NULL);
  long lastTimeInMilliseconds = (lastTime.tv_sec * 1000) + (lastTime.tv_usec / 1000);
  long curTimeInMilliseconds = (curTime.tv_sec * 1000) + (curTime.tv_usec / 1000);
  if (lastTimeInMilliseconds == curTimeInMilliseconds) return;
  int diffTime = curTimeInMilliseconds - lastTimeInMilliseconds;
  lastTime = curTime;
  if (diffTime > 10000) {
    std::cout << "HUGE TIME DIFF " << diffTime << std::endl;
    return;
  }

  unsigned int numCursors = Interaction::getNumCursors();
  for (unsigned int i = 0; i < numCursors; i++) {

    bool print = false;

    // make sure vector is big enough
    if (GLOBAL_mouse_targets.size() < i+1 ) { 
      GLOBAL_mouse_targets.push_back("");
    }
    if (GLOBAL_mouse_wait.size() < i+1) {
      GLOBAL_mouse_wait.push_back(0);
    }
    

    // get the cursor
    Cursor* cursor = Interaction::getCursor(i);
    // don't move the primary mouse
    if (cursor->getId() == PRIMARY_MOUSE_CURSOR_ID) continue;
    if (!cursor->isMouse()) continue;

    // make sure this cursor has a target
    GraphNode* node = GLOBAL_graph->SearchForGraphNode(GLOBAL_mouse_targets[i]);
    if (node == NULL || !node->isVisible()) {
      node = GLOBAL_graph->RandomVisibleNode();
      GLOBAL_mouse_targets[i] = node->getName();
    }
    assert (node != NULL && node->isVisible());

    Pt curs_pos = cursor->getWorldPosition();
    if (!(curs_pos.x >= 0) &&
	!(curs_pos.x <= 0)) {
      exit(0);
    }

   Pt node_pos = node->getPosition();

    if ((node_pos-curs_pos).Length() > 5) {
      //print = true;
    }
    
    if (print) {
      std::cout << "===============\n";
      std::cout << "CURSOR " << i << std::endl;
      std::cout << "CURS: " << curs_pos << std::endl;
      std::cout << "NODE: " << node_pos << std::endl;
    }

    Pt movement_vec = node_pos - curs_pos;
    if (print) {
      std::cout << "MOVEMENT: " << movement_vec << std::endl;
    }

    double len = movement_vec.Length();
    if (len < 5) {
      // very close to goal!
      // choose a new node!
      GLOBAL_mouse_wait[i] += diffTime;
      if (GLOBAL_mouse_wait[i] > 1000 &&
	  args->mtrand.rand() < diffTime/5000.0) {
	GLOBAL_mouse_targets[i] = "";
	//std::cout << "choose new" << std::endl;
	GLOBAL_mouse_wait[i] = 0;
      }
      continue;
    }

    if (len > 20) {
      GLOBAL_mouse_wait[i] = 0;
    }

    if (len > 1) {
      movement_vec.Normalize();
    }

    if (print) {
    std::cout << "MOVEMENT NORMALIZED: " << movement_vec << std::endl;
    }
    Pt diff = movement_vec*diffTime*1/5.0;
    double diff_len = diff.Length();
    if (diff_len > 0.9*len) {
      diff = diff * ((0.9*len) / (diff_len));
    }
    curs_pos += 0.5*diff;
    if (print) {  
      std::cout << "diff: " << diff << std::endl;
      std::cout << "NODE MOVED: " << node_pos << std::endl;
    }
    if (cursor->getId() == PRIMARY_MOUSE_CURSOR_ID) continue;

    assert (cursor->getId() != PRIMARY_MOUSE_CURSOR_ID);
    assert (cursor->isMouse());

    cursor->setWorldPosition(curs_pos);
  }
}


// The function called every frame
void idle() 
{
  Interaction::determineCallBacks();

  if (!args->pause) {
    if (args->randomly_move_multimice) {
      RandomlyMoveMultimice();
    }
    
    // Decide whether to save state and adjust graph or not.
    if (!args->tiled_display.is_tiled_display) {
      GLOBAL_graph->AdjustGraph();
    } else {
      if (args->tiled_display.is_master) {
        GLOBAL_graph->AdjustGraph();
        GLOBAL_graph->SaveGraphState(GRAPH_INTERACTION_STATE_FILENAME);
      } else {
        GLOBAL_graph->LoadGraphState(GRAPH_INTERACTION_STATE_FILENAME);
      }
    }
  }

  // Swap buffers and redisplay
  glutSwapBuffers();
  glutPostRedisplay();
} // end of idle


// The visible call back, if the window is visible
void visible(int vis)
{
  if (vis == GLUT_VISIBLE)
    glutIdleFunc(idle);
  else
    glutIdleFunc(NULL);
} // end of visible

// Called when the window is reshaped
void reshape(int w, int h) 
{
  HandleGLError("BEFORE RESHAPE");

  args->tiled_display.reshape(w,h);

  glViewport(0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  args->tiled_display.ORTHO();
  glMatrixMode (GL_MODELVIEW);

  HandleGLError("AFTER RESHAPE");
} // end of reshape




/***********************
MAIN FUNCTION
***********************/
int main(int argc, char **argv) 
{
  // set up the argument parser
  args = new ArgParser(argc,argv);

  // Register callbacks
  Interaction::registerExpand( expandNode );
  Interaction::registerSimplify( collapseNode );
  Interaction::registerSelect( TryToGrabObj );
  Interaction::registerMove( TryToMoveObj );
  Interaction::registerRelease( TryToReleaseObj );
  Interaction::registerGroup( TryToGroupObjs );
  Interaction::registerUngroup( TryToUngroupObjs );

  // Set up OpenGL state machine
  glutInit(&argc, argv);
  //  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(args->tiled_display.my_width, args->tiled_display.my_height);
  glutInitWindowPosition(20,20);
  glutCreateWindow("Graph Interaction");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutVisibilityFunc(visible);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  // Set the cursor to be none
  glutPassiveMotionFunc(passiveMotion);

  glutSetCursor(GLUT_CURSOR_NONE);
//  glutSetCursor(GLUT_CURSOR_INHERIT);

  // Set up 2D projection
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  args->tiled_display.ORTHO();
  glMatrixMode (GL_MODELVIEW);
  // Turn off depth
  glDisable(GL_DEPTH_TEST);

  // Set up the cursors for mice and lasers
  Interaction::setupCursors(&(args->tiled_display), 
              AddTrackedPoint, RemoveTrackedPoint);
  
  // Are we full screening?
  if (args->tiled_display.full_screen) {
    glutFullScreen();
  } // end if

  // Before we enter the loop, load the data
  if (args->animal_example) {
    GLOBAL_graph = new Graph(args->animals_filename);
  } else {
    assert (args->image_collection_classes.size() >= 1);
    GLOBAL_graph = new Graph(args->image_collection_classes,args->image_collection_directory);
  }

  // Start the loop
  HandleGLError("main");
  glutMainLoop();

  return 0;
}
