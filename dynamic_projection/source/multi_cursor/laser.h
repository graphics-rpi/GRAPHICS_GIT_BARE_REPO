#ifndef _LASER_H_
#define _LASER_H_

#include "cursor.h"

class Laser : public Cursor
{
public:
  Laser(const std::string& laserName, int _laserID, int cursorID, const Vec3f& c) : Cursor(laserName,cursorID, c) 
  {
    laserID = _laserID;
    numFramesOff = 0;
    timeTurnedOff = -1; // initial val??
    setScreenPosition(Pt(0,0));
  } // end of Laser

  int getLaserID() { return laserID; }

  

  // Inherited from Cursor:
  void draw(bool screen_space, const Vec3f &background_color) const;
  int getCurrentStateAndGesture() { return -1; }
  void updatePosition( const Pt p ) {}

  unsigned long getNumFramesOff() const { return numFramesOff; }
  void setNumFramesOff(unsigned long n) { numFramesOff = n; }

private:

  unsigned long timeTurnedOff;
  unsigned int numFramesOff;

  int laserID;
}; // end of class Laser






#endif
