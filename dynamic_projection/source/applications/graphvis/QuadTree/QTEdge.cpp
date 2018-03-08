/*
 * QTEdge.cpp
 *
 *  Created on Jun 17, 2011
 *      Author: phipps
 */

#include "QTEdge.h"
#include "float.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "../../paint/text.h"
#include "../../paint/path.h"

#include "../../../calibration/planar_interpolation_calibration/MersenneTwister.h"
#include "../Layer.h"

#include <sstream>

const unsigned int QTEdge::WIDE_LINE_WIDTH = 30;
const unsigned int QTEdge::THIN_LINE_WIDTH = 3;

// Some of the paths/geometry for the edges/arcs in the database have
// the points in reverse order, reverse the sequence of points for consistency
static bool flip_path(std::deque<Pt>& path, const Pt &a, const Pt &b) {
  double dist = DistanceBetweenTwoPoints(a,b);
  double dist_a = DistanceBetweenTwoPoints(a,path.front());
  double dist_b = DistanceBetweenTwoPoints(b,path.back());
  if (dist_a < 0.1*dist && dist_b < 0.1*dist) {
    return false;
  }  
  double dist_a2 = DistanceBetweenTwoPoints(b,path.front());
  double dist_b2 = DistanceBetweenTwoPoints(a,path.back());
  if (dist_a2 < 0.1*dist &&
      dist_b2 < 0.1*dist) {
    int s = path.size();
    for (int i = 0; i < s/2; i++) {
      Pt tmp = path[i];
      path[i] = path[s-1-i];
      path[s-1-i] = tmp;
    }
    return true;
  }
  std::cout << "BAD DATA" << std::endl;
  return false;
}



// takes a path argument.
QTEdge::QTEdge(int _ID, QTNode *a, QTNode *b, Metadata d, const Vec3f &_color, const std::deque<Pt> &_path )
  :  Path(_path, _color, 1), QTElement(_ID, Pt(0,0), 1, 1, _color) {
  start = a;
  endMIGHTBENULL = b;
  data = d;

  path = _path;
  if (path.size() == 0) {
    path.push_back(a->getPosition());
    path.push_back(b->getPosition());
  } 
  /*  bool flipped = */ flip_path(path,a->getPosition(),b->getPosition());
  //if (flipped) {  std::cout << "FLIP " << getID() << std::endl; }
  
  draw_mode = 1;
  ComputeBoundingBox(path); 
}




void QTEdge::Draw(){



  //std::cout << "DRAW EDGE " << draw_mode << std::endl;
  // HACK
  //  ((QTEdge*)this)->draw_mode = 1;


  
  if (!isVisible()) return;


  /*
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  */

  bool isdamaged = isDamaged();


  if (draw_mode == 1) {
    // pre-click lines
    path_width = THIN_LINE_WIDTH;
    /*
    glLineWidth(3); 
    glPointSize(3);
    glColor4f(color.r(),color.g(),color.b(),1); 
    */
  } else {
    // clicked/selected lines
    path_width = WIDE_LINE_WIDTH;
        /*
    glLineWidth(wide_line_width);

    glPointSize(wide_line_width);
    */
    if (isdamaged) {
      //glColor4f(color.r(),color.g(),color.b(),1); 
    } else {
      Vec3f color2 = 0.75*color+0.25*Vec3f(1,1,1);
      //glColor4f(color2.r(),color2.g(),color2.b(),1); 
    }
  }



  // draw the line segments
  /*
  assert (path.size() >= 2);
  glBegin(GL_LINE_STRIP);
  for(unsigned int i = 0; i < path.size(); i++)
    glVertex2f(path[i].x, path[i].y);
  glEnd();
  */

  // draw the vertices too to make smooth joints
  /*
  glBegin(GL_POINTS);
  for(unsigned int i = 0; i < path.size(); i++)
    glVertex2f(path[i].x, path[i].y);
  glEnd();
  */

  // overwrite the edge with text
  if (draw_mode != 1) {
    std::stringstream ss;
    bool text_bold = false;
    if (isdamaged) { text_bold = true; }

    //std::cout << "HERE!!!" << std::endl;

    ss << " > > Link " << getID() << " (N" << GetStart()->getID() << "-N" << GetEnd()->getID() << ") " << getDamageInfo();
    //int text_height = std::max(30,WIDE_LINE_WIDTH);
    //drawstringonpath(path,ss.str(),Vec3f(1,1,1),text_height,getScaleFactor(),text_bold);
  }

  //glDisable(GL_LINE_SMOOTH);

}

std::string QTEdge::getText(){
  std::stringstream ss;
  ss << " > > Link " << getID() << " (N" << GetStart()->getID() << "-N" << GetEnd()->getID() << ") " 
      << getDamageInfo();
  return ss.str();
}







std::ostream& operator<< (std::ostream &ostr, const QTEdge &e){
  int eid = -1;
  if (e.IsEndValid()) eid = e.GetEnd()->getID();
  ostr << "QTEdge: " << e.getID() << " Start: " << e.start->getID()
       << " End: " << eid << " Type: " << e.start->getType() << "->" << e.endMIGHTBENULL->getType() << std::endl;
  return ostr;
}

bool QTEdge::isDamaged() const {
  return (GetData()->getVal("damage") != "");
}

const std::string& QTEdge::getDamageInfo() const {
  static std::string ok = "status ok";
  if (isDamaged()) {
    return GetData()->getVal("damage");
  } else {
    return ok;
  }
}




void QTEdge::SetDrawMode(int i){
  if(i >= 1 && i <= 4) {
    draw_mode = i;
  }
}


bool QTEdge::AnimateSize(){
  return false;
}


static inline double PerpendicularDistanceToLineSegment(const Pt &p, const Pt &p1, const Pt &p2) {
  return PerpendicularDistanceToLineSegment(Vec3f(p.x,p.y,0),
                                            Vec3f(p1.x,p1.y,0),
                                            Vec3f(p2.x,p2.y,0));
}


static inline void combine(double d, double &answer) {
  if (d < 0) return;
  if (answer < 0) answer = d;
  else answer = std::min(d,answer);
}


double QTEdge::DistanceFrom(const Pt &p) const {
  //  Pt p1 = start->getPosition();
  double answer = -1;
  for(unsigned int i = 0; i < path.size(); i++) {
    double d = DistanceBetweenTwoPoints(p,path[i]);
    combine(d,answer);
    if (i > 0) {
      double d = PerpendicularDistanceToLineSegment(p,path[i-1],path[i]);
      combine (d,answer);
    }
  }
  return answer;
}


void QTEdge::ComputeBoundingBox(const std::deque<Pt> &path) {
  boundingbox.Extend(start->getCentroid());
  boundingbox.Extend(endMIGHTBENULL->getCentroid());
  for (unsigned int i = 0; i < path.size(); i++) {
    boundingbox.Extend(path[i]);
  }
}


bool QTEdge::Overlap(const BoundingBox2f &bb) const {
  bool conservative = QTElement::Overlap(bb);
  if (!conservative) { return false; }

  for (unsigned int i = 0; i < path.size()-1; i++) {
    BoundingBox2f tmp(path[i],path[i+1]);
    if (tmp.Overlap(bb)) return true;
  }
  return false;
}


bool QTEdge::isVisible() const {
  Layer *l = GetStart()->getLayer();
  assert (l != NULL);
  if (!l->GetNodeShow()) return false;
  if (!l->GetEdgeShow()) return false;
  
  bool same = GetStart()->getType() == GetEnd()->getType();
  if (!same && !l->GetCrossEdgeShow()) return false;
  return true;
}

double QTEdge::getZOrder() const {
    return (double)getLastTouched()/(double)getMaxTouched();
}

Pt QTEdge::getPosition(){
    return (start->getPosition()+endMIGHTBENULL->getPosition())/2;
}
