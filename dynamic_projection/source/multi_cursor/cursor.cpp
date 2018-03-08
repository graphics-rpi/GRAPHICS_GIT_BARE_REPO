#include "cursor.h"
#include "interaction.h"
#include <sys/time.h>

Cursor::Cursor(const std::string &n, int id, const Vec3f& c)   
{ 
  name = n;
  cursorId = id;
  color = c;
  

  //  objPtr = NULL;
  //objType = "NULL";
  //obj = NULL;
  
  clickedObjectPtr = NULL;

  // MIGHT BREAK THINGS!!
  state = 'n';
  scroll_state = 'n';
  screen_position_set = false;
  world_position_set = true;
  //screen_position = Pt( 0, 0 );
  world_pos = Pt( 0, 0 );
  scroll_pos = 0;
  mode = NORMAL_MODE;

  tracked_point = NULL;
}



std::ostream& operator<< (std::ostream &ostr, const Cursor &c) {
  std::cout << "CURSOR " << c.cursorId << " " << c.name << " button="
	    << c.whichbutton << " state=" << c.state << " pos=" << c.getScreenPosition() << std::endl;
  return ostr;
}


Pt ScreenToWorld(const Pt &p) {
  return (p*Interaction::interface_scale)+Interaction::interface_offset;
}


Pt WorldToScreen(const Pt &p) {
  return (p-Interaction::interface_offset)/Interaction::interface_scale;
}


Pt Cursor::getScreenPosition() const { 
  if (screen_position_set) {
    return screen_pos;
  }
  assert (world_position_set);
  return WorldToScreen(world_pos);
  //return (world_pos-Interaction::interface_offset)/Interaction::interface_scale;
}


Pt Cursor::getWorldPosition() const { 
  if (world_position_set) {
    return world_pos;
  }
  assert (screen_position_set);
  return ScreenToWorld(screen_pos);
  //return (screen_pos*Interaction::interface_scale)+Interaction::interface_offset;
} 


int Cursor::getScrollPosition() const {
    return scroll_pos;
}

void Cursor::setScreenPosition(const Pt& p) { 
  //std::cout << "set screen position " << p << std::endl;
  //std::cout << "get id " << getId() << std::endl;
  //std::cout << "is mouse? " << isMouse() << std::endl;
  //  assert (getId() == PRIMARY_MOUSE_CURSOR_ID || !isMouse());
  screen_pos = p;
  screen_position_set = true;
  world_position_set = false;
  updateTrail();
}

void Cursor::setWorldPosition(const Pt& p) { 
  assert ((getId() != PRIMARY_MOUSE_CURSOR_ID) && isMouse());
  world_pos = p;
  world_position_set = true;
  screen_position_set = false;
  updateTrail();
}

void Cursor::setScrollPosition(const int p){
    scroll_pos += p;
}

void Cursor::updateTrail() {
  timeval now;  
  gettimeofday(&now,NULL);
  long curTimeInMilliseconds = (now.tv_sec * 1000) + (now.tv_usec / 1000);
  // check to see if we've updated the trail too recently
  if (world_position_trail.size() > 0) {
    timeval last = world_position_trail.back().first;
    long lastTimeInMilliseconds = (last.tv_sec * 1000) + (last.tv_usec / 1000);
    // store trail at 500fps
    // 100 fps = 10 ms
    if (curTimeInMilliseconds - lastTimeInMilliseconds < 2) {
      return;
    }
  }
  // add this pt to the trail
  Pt p = getWorldPosition();
  world_position_trail.push_back(std::make_pair(now,p));
  // limit the length of the trail
  // store 10 seconds = 10 000 ms
  timeval start = world_position_trail.front().first;
  long startTimeInMilliseconds = (start.tv_sec * 1000) + (start.tv_usec / 1000);
  int diff = (curTimeInMilliseconds - startTimeInMilliseconds);
  if (diff > 10000) {
    world_position_trail.pop_front();
  }
  //std::cout << "for cursor " << getId() << " trail has length " << screen_position_trail.size() << std::endl;
}


void InsertGLColor(const Vec3f &c);


void Cursor::draw_trail(bool screen_space, const Vec3f &background_color, double seconds, timeval cur) const {
  glLineWidth(3); 
  glBegin(GL_LINE_STRIP);
  //  timeval cur;
  // gettimeofday(&cur,NULL);
  long curTimeInMilliseconds = (cur.tv_sec * 1000) + (cur.tv_usec / 1000);
  for (std::list<std::pair<timeval,Pt> >::const_iterator itr = world_position_trail.begin();
       itr != world_position_trail.end(); itr++) {
    timeval tmp = itr->first;
    long tmpTimeInMilliseconds = (tmp.tv_sec * 1000) + (tmp.tv_usec / 1000);
    int diff = (curTimeInMilliseconds - tmpTimeInMilliseconds);
    if (diff  > 1000 * seconds) continue;
    Pt p = itr->second;
    if (screen_space) {
      p = WorldToScreen(p);
    }
    InsertGLColor(color);
    glVertex2f(p.x,p.y);
  }
  glEnd();
}
