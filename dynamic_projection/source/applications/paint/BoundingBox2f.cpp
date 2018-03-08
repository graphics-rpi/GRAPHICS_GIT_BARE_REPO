/*
 * BoundingBox2f.cpp
 *
 *  Created on: Aug 9, 2011
 *      Author: drillprp
 */

#include "BoundingBox2f.h"

BoundingBox2f::BoundingBox2f(Pt t1, Pt t2) {
  is_initialized = false;
  Extend(t1);
  Extend(t2);
}


Pt BoundingBox2f::GetCenter() const {
  assert (isInitialized());
  return Pt((min.x + max.x)/2.0, (min.y + max.y)/2.0);
}

double BoundingBox2f::Distance(const BoundingBox2f &box) const {
  assert (isInitialized());
  Pt c1 = GetCenter();
  Pt c2 = box.GetCenter();
  return sqrt((c1.x-c2.x)*(c1.x-c2.x) +
              (c1.y-c2.y)*(c1.y-c2.y));
}

void BoundingBox2f::DrawBB() const {
  assert (isInitialized());

#if 0
  Pt Min = min + 0.01 * (max-min);
  Pt Max = min + 0.99 * (max-min);
#else
  Pt Min = min;
  Pt Max = max;
#endif

  glBegin(GL_LINES);
  glVertex2f(Min.x, Min.y);
  glVertex2f(Max.x, Min.y);
  glVertex2f(Max.x, Min.y);
  glVertex2f(Max.x, Max.y);
  glVertex2f(Max.x, Max.y);
  glVertex2f(Min.x, Max.y);
  glVertex2f(Min.x, Max.y);
  glVertex2f(Min.x, Min.y);
  glEnd();

  /*
  glBegin(GL_LINES);
  glVertex2f(Min.x, Min.y);
  glVertex2f(Max.x, Max.y);
  glVertex2f(Max.x, Min.y);
  glVertex2f(Min.x, Max.y);
  glEnd();
  */
}

bool BoundingBox2f::Overlap(const BoundingBox2f & other) const {
  assert (isInitialized());
  assert( min.x <= max.x );
  assert( min.y <= max.y );
  assert( other.min.x <= other.max.x );
  assert( other.min.y <= other.max.y );
  
  if (this->min.x > other.max.x) return false;
  if (this->max.x < other.min.x) return false;

  if (this->min.y > other.max.y) return false;
  if (this->max.y < other.min.y) return false;

  return true;
}


bool BoundingBox2f::PointInside(const Pt & pt) const {
   assert (isInitialized());
  if( (pt.x >= min.x && pt.x < max.x) && 
      (pt.y >= min.y && pt.y < max.y) )
    {
      return true;
    }
  return false;
}


void BoundingBox2f::Extend(Pt p) {
   if (!isInitialized()) {
     min = p;
     max = p;
     is_initialized = true;
   } else {
     min.x = std::min(p.x,min.x);
     min.y = std::min(p.y,min.y);
     max.x = std::max(p.x,max.x);
     max.y = std::max(p.y,max.y);
   }
   assert (isInitialized());
 }
