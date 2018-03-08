/*
 * QTEdge.cpp
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */

#include "QTEdge.h"
#include <vector>
#include <cstdio>
#include <cstdlib>

/*
Edge::Edge() : DrawableObject()
{
	ID = -1;
	start = NULL;
	endMIGHTBENULL = NULL;
	visible = true;
	linemode = 0;
	interpolateFlag = false;
	color = Vec3f(0,0,1);
	///r = 0;
	//g = 0;
	//b = 1;
	//a = 1;
}
*/
//  -- ASK FOR INPUT ON THIS --
//  Yes, the parameter passed to QTElement are awful 
//  -- FIX LATER SO ITS NOT AWFUL --
QTEdge::QTEdge(int _ID, DrawnNode *a, DrawnNode *b, Metadata d, const Vec3f &_color)//float _r, float _g, float _b, float _al)
  : QTElement(Pt(0,0), 1, 1, _color, BoundingBox2f(a->getPosition(),b->getPosition(),true))
{
	linemode = 0;
	interpolateFlag = false;
	Init(_ID, a, b, d);
	//SetColor(_color);//_r, _g, _b, _al);
}

/*
Edge::Edge(const Edge &other) : DrawableObject()
{
	ID = other.ID;
	start = other.start;
	endMIGHTBENULL = other.endMIGHTBENULL;
	data = other.data;
	visible = other.visible;
	linemode = other.linemode;
	color = other.color; //SetColor(other.color);//other.r, other.g, other.b, other.a);
	interpolateFlag = other.interpolateFlag;
}
*/

 /*
bool Edge::operator=(const Edge &other)
{
	ID = other.ID;
	start = other.start;
	endMIGHTBENULL = other.endMIGHTBENULL;
	data = other.data;
	visible = other.visible;
	linemode = other.linemode;
	color = other.color; //SetColor(other.color);//other.r, other.g, other.b, other.a);

	return true;
}
 */

QTEdge::~QTEdge(){}

bool QTEdge::operator==(const QTEdge &other)
{

	if(ID == other.ID)
		return true;
	else
		return false;

}

           /*
double PerpendicularDistanceToLineSegment(const Vec3f &pt, const Vec3f &v0, const Vec3f &v1){
      // return -1 if pt projected perpendicularly to line does not
      //   // lie on the segment from l1 to l2, otherwise return distance to
      //   line
      //
             */

float QTEdge::hittest(BoundingBox2f _bounds)
{
    const Pt test_pt = _bounds.GetCenter();
    Vec3f test_vec = Vec3f(test_pt.x, test_pt.y, 0);
    Vec3f start = Vec3f(0,0,0);
    Vec3f end = Vec3f(0,0,0);
    double mindist = 10000000000;
    double temp;

    int i = 0;
    for(unsigned int i = 0; i < polylines.size(); i += 2)
    {
        //start = Vec3f(polylines[i], polylines[i+1], 0);
        start = Vec3f(polylines[i+2], polylines[i+3], 0);

        temp = DistanceBetweenTwoPoints(start, test_vec);
        //temp = DistanceToLineSegment(test_vec, start, end);

        if(temp < mindist)
            mindist = temp;
    }
    std::cout << "mindist: " << mindist << std::endl;
    return mindist;
}

void QTEdge::Init(int _ID, DrawnNode *a, DrawnNode *b, Metadata d) //float _r, float _g, float _b, float _al)
{

	ID = _ID;
	start = a;
	endMIGHTBENULL = b;
	data = d;
	visible = true;
	linemode = 0;

}

void QTEdge::Draw()
{
  /*
	if(visible == false)
		return;
	if(!(start->IsVisible() && end->IsVisible()))
		return;
  */
	assert(linemode >= 0 && linemode <= 2);

	glLineStipple(3,0xcccc);
	//glLineStipple(3,0xf0f0);

	Pt p1 = start->getPosition();

	bool status = getStatus();


	if(linemode == 0) {

	  if (!IsEndValid()) return;
	  Pt p2 = GetEnd()->getPosition();
      glLineWidth(2);
      glColor4f(color.r(),color.g(),color.b(),1); //r, g, b, a);
	  glBegin(GL_LINES);
	  glVertex2f(p1.x, p1.y);
		//if(interpolateFlag)
		//	glColor4f(1,1,1,1);
	  glVertex2f(p2.x, p2.y);
	  glEnd();

        glPointSize(8);

        glLineWidth(2);

	  if (!status) glEnable(GL_LINE_STIPPLE);

	  glLineWidth(3);
	  glColor4f(color.r(),color.g(),color.b(),1); //r, g, b, a);
	  glBegin(GL_LINES);
	  glVertex2f(p1.x, p1.y);
	  //if(interpolateFlag)
	  //	glColor4f(1,1,1,1);
	  glVertex2f(p2.x, p2.y);
	  glEnd();

	  if (!status) glDisable(GL_LINE_STIPPLE);

	}
	else if(linemode == 1)
	{
	  if (!status) glEnable(GL_LINE_STIPPLE);
	  glLineWidth(3);
		glColor4f(color.r(),color.g(),color.b(),1); //r, g, b, a);
		//glColor4f(r, g, b, a);

        glBegin(GL_LINE_STRIP);
		for(unsigned int i = 0; i < polylines.size(); i+=2)
		    glVertex2f(polylines[i], polylines[i+1]);
		glEnd();

		if (!status) glDisable(GL_LINE_STIPPLE);
	}
	else if(linemode == 2)
	{
	  if (!status) glEnable(GL_LINE_STIPPLE);
		std::vector <std::pair<double,double> > temp;
		Button *currbutton = start->getButton();

		Pt pt = currbutton->getLowerLeftCorner();

		temp.push_back(std::make_pair(pt.x + 0.50 * currbutton->getWidth(), pt.y + .55 * currbutton->getHeight()));
		temp.push_back(std::make_pair(pt.x + 0.55 * currbutton->getWidth(), pt.y + .50 * currbutton->getHeight()));
		temp.push_back(std::make_pair(pt.x + 0.55 * currbutton->getWidth(), pt.y + .45 * currbutton->getHeight()));
		temp.push_back(std::make_pair(pt.x + 0.50 * currbutton->getWidth(), pt.y + .40 * currbutton->getHeight()));
		temp.push_back(std::make_pair(pt.x + 0.45 * currbutton->getWidth(), pt.y + .45 * currbutton->getHeight()));
		temp.push_back(std::make_pair(pt.x + 0.45 * currbutton->getWidth(), pt.y + .50 * currbutton->getHeight()));

		glColor4f(color.r(),color.g(),color.b(),1); //r, g, b, a);
		//glColor4f(r, g, b, a);
		glBegin(GL_TRIANGLE_FAN);

		glVertex2f(GetEnd()->getButton()->getCentroid().x, GetEnd()->getButton()->getCentroid().y);

		for(unsigned int i = 0; i < temp.size(); ++i)
		{
			glVertex2f(temp[i].first, temp[i].second);
		}

		glEnd();
		if (!status) glDisable(GL_LINE_STIPPLE);
	}

}

void QTEdge::AddLinePoint(double x, double y)
{
    double minx = 50000000, maxx = -500000000;
    double miny = 50000000, maxy = -500000000;

    BoundingBox2f curr_box = getBox();

    polylines.push_back(x);
	polylines.push_back(y);

    for(int i = 0; i < polylines.size(); i +=2)
    {
      if(polylines[i] > maxx)
          maxx = polylines[i];
      if(polylines[i] < minx)
          minx = polylines[i];
    }

    for(int i = 1; i < polylines.size(); i +=2)
    {
      if(polylines[i] > maxy)
          maxy = polylines[i];
      if(polylines[i] < miny)
          miny = polylines[i];
    }

    setBox(BoundingBox2f(Pt(minx, miny), Pt(maxx, maxy)));
}


std::ostream& operator<< (std::ostream &ostr,  QTEdge &e){

  int eid = -1;
  if (e.IsEndValid()) eid = e.GetEnd()->getID();
  ostr << "QTEdge: " << e.ID << " Start: " << e.start->getID()
       << " End: " << eid << " Type: " << e.data.getType();

	return ostr;
}

std::string QTEdge::getStartType(){
	return start->GetData()->getType();
}

std::string QTEdge::getEndType(){
	if(endMIGHTBENULL)
		return endMIGHTBENULL->GetData()->getType();
	assert(0);
	return "";
}

bool QTEdge::getStatus(){
return true;
}
