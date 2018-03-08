#include "object_group.h"


// Accessors
ObjectGroup::ObjectGroup( const std::vector< ClickableObject* > &v )
{
  objects = v;
  parentObjectGroup = NULL;
  // Inherited from ClickableObject
  is_object_group = true;
} // end of 

bool ObjectGroup::isObjectInGroup( ClickableObject* d ) 
{ 
  std::vector<ClickableObject*>::iterator itr;
  itr = std::find( objects.begin(), objects.end(), d );
  if( itr != objects.end() ) return true;
    return false;
} // end of isNodeInGroup



double ObjectGroup::DistanceFrom(const Pt &p) const {
  assert (objects.size() > 0);
  double answer = objects[0]->DistanceFrom(p);
  for (unsigned int i = 1; i < objects.size(); i++) {
    double tmp = objects[i]->DistanceFrom(p);
    if (tmp < answer) answer = tmp;
  }
  return answer;
}


void ObjectGroup::paint(const Vec3f &background_color) const {
  for (unsigned int i = 0; i < objects.size(); i++) {
    objects[i]->paint(background_color);
  }
}
