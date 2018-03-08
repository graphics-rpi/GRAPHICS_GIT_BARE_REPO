#include "multiMouse.h"
#include "interaction.h"

#define scale 10


MultiMouse::MultiMouse(const std::string& n, Pt p, int mid, int cid, const Vec3f& c) : Cursor(n,cid, c) {
  if (getId() == PRIMARY_MOUSE_CURSOR_ID)
    setScreenPosition(p); 
  else
    setWorldPosition(p); 
  state = 'x';
  // maybe will need to be added back in??
  //pressed_button = NULL;
  //pressed_polyButton_ind = -1;
  mouseId = mid;
  timeOfPreviousClick = 0; // ?? initial value
}


void MultiMouse::updatePosition( const Pt u ) { 
  assert (getId() != PRIMARY_MOUSE_CURSOR_ID);
  //std::cout << "updateposition " << u << std::endl;
  setWorldPosition(getWorldPosition()+u);
  //    world_position.x += u.x; world_position.y -= u.y; 
  //screen_position = world_position;
}


void InsertGLColor(const Vec3f &c) {
  glColor3f(c.r(),c.g(),c.b());
}

glm::vec3 MultiMouse::getCursorOutlineColor(){
    if(isStateDown()){
        return( glm::vec3(color.r(), color.g(), color.b()) * 0.6f );
    }
    return( glm::vec3( 0.0f, 0.0f, 0.0f ));
}

std::vector<glm::vec2> MultiMouse::getCursorOutline(){
    std::vector<glm::vec2> positions;
    
    Pt backtofront_vec(-scale*2,scale*4);
    Pt lefttoright_vec(scale*2,scale);

    Pt tip = getScreenPosition();

    Pt back = tip - 0.7*backtofront_vec;
    Pt p = 0.2*tip + 0.8*back;
    Pt left = p - 0.5*lefttoright_vec;
    Pt right = p + 0.5*lefttoright_vec;
    Pt mid = 0.5*tip + 0.5*back;
    Pt midleft = mid - 0.1*lefttoright_vec;
    Pt midright = mid + 0.1*lefttoright_vec;
    Pt backleft = back - 0.1*lefttoright_vec;
    Pt backright = back + 0.1*lefttoright_vec;

    positions.push_back(glm::vec2(left.x,left.y));
    positions.push_back(glm::vec2(tip.x,tip.y));

    positions.push_back(glm::vec2(tip.x,tip.y));
    positions.push_back(glm::vec2(right.x,right.y));

    positions.push_back(glm::vec2(right.x,right.y));
    positions.push_back(glm::vec2(midright.x,midright.y));

    positions.push_back(glm::vec2(midright.x,midright.y));
    positions.push_back(glm::vec2(backright.x,backright.y));

    positions.push_back(glm::vec2(backright.x,backright.y));
    positions.push_back(glm::vec2(backleft.x,backleft.y));

    positions.push_back(glm::vec2(backleft.x,backleft.y));
    positions.push_back(glm::vec2(midleft.x,midleft.y));

    positions.push_back(glm::vec2(midleft.x,midleft.y));
    positions.push_back(glm::vec2(left.x,left.y));
    
    return positions;
}

std::vector<glm::vec2> MultiMouse::getCursorShape(){
    std::vector<glm::vec2> positions;
    
    Pt backtofront_vec(-scale*2,scale*4);
    Pt lefttoright_vec(scale*2,scale);

    Pt tip = getScreenPosition();

    Pt back = tip - 0.7*backtofront_vec;
    Pt p = 0.2*tip + 0.8*back;
    Pt left = p - 0.5*lefttoright_vec;
    Pt right = p + 0.5*lefttoright_vec;
    Pt mid = 0.5*tip + 0.5*back;
    Pt midleft = mid - 0.1*lefttoright_vec;
    Pt midright = mid + 0.1*lefttoright_vec;
    Pt backleft = back - 0.1*lefttoright_vec;
    Pt backright = back + 0.1*lefttoright_vec;


    positions.push_back(glm::vec2(left.x,left.y));
    positions.push_back(glm::vec2(tip.x,tip.y));
    positions.push_back(glm::vec2(mid.x,mid.y));

    positions.push_back(glm::vec2(mid.x,mid.y));
    positions.push_back(glm::vec2(tip.x,tip.y));
    positions.push_back(glm::vec2(right.x,right.y));

    positions.push_back(glm::vec2(midleft.x,midleft.y));
    positions.push_back(glm::vec2(midright.x,midright.y));
    positions.push_back(glm::vec2(backright.x,backright.y));

    positions.push_back(glm::vec2(midleft.x,midleft.y));
    positions.push_back(glm::vec2(backright.x,backright.y));
    positions.push_back(glm::vec2(backleft.x,backleft.y));

    return positions;
}

// This function draws the mouse cursor on screen
void MultiMouse::draw(bool screen_space, const Vec3f &background_color) const
{

  if (world_position_trail.size() <= 1) return;

  //If the cursor is in "down" state
  glPointSize(2*scale); //12);

  Pt backtofront_vec(-scale*2,scale*4);
  Pt lefttoright_vec(scale*2,scale);
  
  //  for(int i=0; i<MULTIMOUSENUM; i++)
  //  {
  Pt tip;
  if (screen_space) {
    tip = getScreenPosition();
  } else {
    tip = getWorldPosition();
  }

    Pt back = tip - 0.7*backtofront_vec;
    Pt p = 0.2*tip + 0.8*back;
    Pt left = p - 0.5*lefttoright_vec;
    Pt right = p + 0.5*lefttoright_vec;
    Pt mid = 0.5*tip + 0.5*back;
    Pt midleft = mid - 0.1*lefttoright_vec;
    Pt midright = mid + 0.1*lefttoright_vec;
    Pt backleft = back - 0.1*lefttoright_vec;
    Pt backright = back + 0.1*lefttoright_vec;

    Vec3f color = getColor();
    //    if (screen_space) color*=0.5;

    // DRAW AN EDGE AROUND THE ARROW (white if state down)
    glLineWidth(4);
    InsertGLColor(background_color);
    if(isStateDown())
    {

      InsertGLColor(color*0.6 + background_color*0.4);

    	//glColor3f(1,1,1);
      //glColor3f(color.x()+0.2,
      //        color.y()+0.2,
      //        color.z()+0.2);
  	}
    if(isGroupUngroupMode())
  	{
      InsertGLColor(color*0.8 + background_color*0.2);
      //      glColor3f(color.x()+0.4,
      //        color.y()+0.4,
      //        color.z()+0.4);  	
  	} // end of if
  	
    glBegin(GL_LINE_STRIP);
    glVertex2f(left.x,left.y);
    glVertex2f(tip.x,tip.y);
    glVertex2f(right.x,right.y);
    glVertex2f(midright.x,midright.y);
    glVertex2f(backright.x,backright.y);
    glVertex2f(backleft.x,backleft.y);
    glVertex2f(midleft.x,midleft.y);
    glVertex2f(left.x,left.y);
    glEnd();

    // DRAW A COLORED ARROW
    glBegin(GL_TRIANGLES);
    glColor3f(color.x(),
	      color.y(),
	      color.z());
    glVertex2f(left.x,left.y);
    glVertex2f(tip.x,tip.y);
    glVertex2f(mid.x,mid.y);
    glVertex2f(mid.x,mid.y);
    glVertex2f(tip.x,tip.y);
    glVertex2f(right.x,right.y);
    glVertex2f(midleft.x,midleft.y);
    glVertex2f(midright.x,midright.y);
    glVertex2f(backright.x,backright.y);
    glVertex2f(midleft.x,midleft.y);
    glVertex2f(backright.x,backright.y);
    glVertex2f(backleft.x,backleft.y);
    glEnd();
//  }


} // end of draw()


/*
// This function determines the current gesture and state (will eventually be changed to get rid of state, perhaps?)
int MultiMouse::getCurrentStateAndGesture()
{
  
} // end of setCurrentStateAndGesture
*/

/*
void MultiMouse::mousefunc_helper(int which_mouse, int button, int state, int x, int y, int glut_modifiers)
{
  // If we're not concerned with this mouse
  if( which_mouse != this -> mouseId ) return;

} // end of mousefunc_helper

void MultiMouse::motionfunc_helper(int which_mouse, int x, int y, int glut_modifiers)
{
  // If we're not concerned with this mouse
  if( which_mouse != this -> mouseId ) return;
  
} // end of motionfunc_helper
*/


