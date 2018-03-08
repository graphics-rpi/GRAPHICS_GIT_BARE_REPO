/*
 * Spring.h
 *
 *  Created on: Jul 1, 2011
 *      Author: phipps
 */

#ifndef SPRING_H_
#define SPRING_H_

#include "QuadTree/QTNode.h"

class Spring {

public:

	Spring();
	Spring(QTNode *start, QTNode *end, double optimal, double w, int index, int mode = 0);
	Spring(QTNode *start, Pt anchor, double optimal, double w, int index, int mode = 0);

	//Data Fetching and spring starts and ends
	int GetIndex();
	QTNode* GetStart();
	QTNode* GetEnd();
	void SetStart(QTNode *start);
	bool SetEnd(QTNode *end);
	bool SetAnchor(Pt p);
	Pt GetAnchor();

	//Spring Calculations
	void SetIgnore(bool val);
	bool IsIgnored();
	void SetOptimal(double optimal);
	double GetOptimal();
	void SetW(double w);
	double GetW();
	Pt GetNewPos();

	//Drawing
	void SetMode(int i);
	void SetVisible(bool val);
	void Draw();

	//Operators
	bool operator==(const Spring &other);

private:

	int ID;
	//type 0 is push/pull type 1 is push type 2 is pull
	int mode;
	double w;
	double optimal;
	bool visible;
	bool ignore;
	QTNode *start;
	QTNode *end;
	Pt anchor;

};

#endif /* SPRING_H_ */
