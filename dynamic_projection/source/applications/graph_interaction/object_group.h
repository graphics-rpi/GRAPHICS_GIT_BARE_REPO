#ifndef _GROUP_H_
#define _GROUP_H_

/******************
* SYSTEM INCLUDES *
******************/

#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

/******************
** USER INCLUDES **
******************/

#include "../paint/ClickableObject.h"

/******************
*** CLASS DEF *****
******************/

class ObjectGroup : public ClickableObject { 
public:

  // Constructor, Destructor
  ObjectGroup( const std::vector< ClickableObject* > &obs );
  ~ObjectGroup();

  virtual double DistanceFrom(const Pt &p) const;
  virtual void paint(const Vec3f &background_color) const;

  // Accessors 
  void addObjectToGroup( ClickableObject* o ) { objects.push_back( o ); }
  unsigned int numNodesInObjectGroup( ) { return objects.size(); }
  void clearObjectGroup() { objects.clear(); }

  // Note that this checks to see if the pointers point at the same memory location
  bool isObjectInGroup( ClickableObject* o ) ;
  ClickableObject* getNodeInObjectGroup( unsigned int i ) { assert( i < objects.size() ); return objects[ i ]; }
  
  // Accessors for the parent object group
  void setParentObjectGroup( ObjectGroup* g ) { parentObjectGroup = g; }

private:
  std::vector< ClickableObject* > objects;

}; // end of class Group


#endif
