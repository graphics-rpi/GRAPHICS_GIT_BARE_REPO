/*
 * Spring.cpp
 *
 *  Created on: Jul 1, 2011
 *      Author: phipps
 */

#include "Spring.h"
#include <iostream>

Spring::Spring()
{
	ID = -1;
	w = .5;
	optimal = 240;
	start = NULL;
	end = NULL;
	mode = 0;
	visible = 1;
	ignore = 0;
}

//type 0 is push/pull type 1 is push type 2 is pull
Spring::Spring(QTNode *start, QTNode *end, double optimal, double w, int ID, int mode)
{
	this->ID = ID;
	this->start = start;
	this->end = end;
	this->optimal = optimal;
	this->w = w;
	this->mode = mode;
	visible = 1;
	ignore = 0;
}

//type 0 is push/pull type 1 is push type 2 is pull
Spring::Spring(QTNode *start, Pt anchor, double optimal, double w, int ID, int mode)
{
	this->ID = ID;
	this->start = start;
	this->end = NULL;
	this->anchor = anchor;
	this->optimal = optimal;
	this->w = w;
	this->mode = mode;
	visible = 1;
	ignore = 0;
}

QTNode* Spring::GetEnd()
{
	return end;
}

double Spring::GetOptimal()
{
	return optimal;
}

QTNode* Spring::GetStart()
{
	return start;
}

double Spring::GetW()
{
  /*
	if(!start->IsVisible() || start->IsCollapsed() > -1 || ignore)
		return 1;
  */
	return w;
}

int Spring::GetIndex()
{
	return ID;
}

bool Spring::SetEnd(QTNode *end)
{
	if(end != NULL)
	{
		this->end = end;
		return true;
	}

	return false;
}

void Spring::SetOptimal(double optimal)
{
	this->optimal = optimal;
}

void Spring::SetStart(QTNode *start)
{
	this->start = start;
}

void Spring::SetW(double w)
{
	this->w = w;
}

bool Spring::SetAnchor(Pt p)
{
	if(end == NULL)
	{
		anchor = p;
		return true;
	}

	return false;
}


Pt Spring::GetAnchor()
{
	return anchor;
}

static double eucdist(double x1, double x2, double y1, double y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}


//This function returns to the graph class a point that corresponds
//to the new position, this is used in the weighted position calculation
//in the adjust function
Pt Spring::GetNewPos()
{

	Pt a = start->getPosition();
	Pt b;

	if(ignore)
		return Pt(a.x, a.y);

	b = end == NULL ? anchor:end->getPosition();

  /*
	if(!start->IsVisible())
	{
		return Pt(b.x, b.y);
	}

	if(end != NULL)
	{
		if(!end->IsVisible())
		{
			return Pt(a.x, a.y);
		}
	}
  */
	double dx = a.x-b.x;
	double dy = a.y-b.y;
	double dist = eucdist(a.x,b.x,a.y,b.y);
	double x,y;

	dx /= dist;
	dy /= dist;

	x = a.x;
	y = a.y;

	if(mode == 0)
	{
		x = b.x + dx*optimal;
		y = b.y + dy*optimal;
		x = 0.5 * x + 0.5 * a.x;
		y = 0.5 * y + 0.5 * a.y;
	}
	else if(mode == 1 && dist < optimal)
	{
		x = b.x + dx*optimal;
		y = b.y + dy*optimal;
		x = 0.5 * x + 0.5 * a.x;
		y = 0.5 * y + 0.5 * a.y;
	}
	else if(mode == 2 && dist > optimal)
	{
		x = b.x + dx*optimal;
		y = b.y + dy*optimal;
		x = 0.5 * x + 0.5 * a.x;
		y = 0.5 * y + 0.5 * a.y;
	}
	return Pt(x,y);


}

void Spring::SetMode(int i)
{
	mode = i;
}

void Spring::SetVisible(bool val)
{
	visible = val;
}

void Spring::Draw()
{
	if(!visible)
		return;

	Pt p1 = start->getPosition();
	Pt p2 = end == NULL ? anchor:end->getPosition();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

	glLineWidth(2);
	glBegin(GL_LINES);
	glColor4f(1,0,0,125);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glEnd();

}

void Spring::SetIgnore(bool val)
{
	ignore = val;
}

bool Spring::IsIgnored()
{
	return ignore;
}

bool Spring::operator==(const Spring &other)
		{
	if(ID == other.ID)
		return true;
	return false;
		}
