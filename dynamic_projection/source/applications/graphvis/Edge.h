/*
 * Edge.h
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */



#ifndef EDGE_H_
#define EDGE_H_
#include <vector>
#include "DrawnNode.h"
#include "Metadata.h"
#include "../paint/DrawableObject.h"
#include "../paint/BoundingBox2f.h"
#include "../../../../../remesher/src/vectors.h"
#include "../../../../../remesher/src/utils.h"

class DrawnNode;
class Metadata;

class Edge : public QTElement{

	friend std::ostream& operator<< (std::ostream &ostr, Edge &e);

public:

	//Constructors and initialization
	//Edge();
	Edge(int _ID, DrawnNode *a, DrawnNode *b, Metadata d, const Vec3f &_color); //float _r = 0, float _g = 0, float _b = 1, float _al = 1);
	//Edge(const Edge &other);
	~Edge();

	//Start and end fetching with data
	int GetID(){return ID;}
	DrawnNode* GetStart() {return start;}
	bool IsEndValid() { return (endMIGHTBENULL != NULL); }
	DrawnNode* GetEnd() { assert (IsEndValid()); return endMIGHTBENULL;}
	Metadata* GetData() {return &data;}
	void SetEnds(DrawnNode &a, DrawnNode &b);
	void AddLinePoint(double x, double y);
	std::string getStartType();
	std::string getEndType();

	//Drawing
	//	void SetColor(const Vec3f &_color) { color = _color; } //float _r, float _g, float _b, float _a){r = _r; g = _g; b = _b; a = _a;}
	void SetVisible(bool val){visible = val;}
	void SetLineMode(int val){linemode = val;}
	void SetInterpolate(bool interpolate){interpolateFlag = interpolate;}
	int GetLineMode(){return linemode;}
	bool IsVisible(){return visible;}
	void Draw();
	bool getStatus();
  //	void paint() const;

	//operators
	bool operator=(const Edge &other);
	bool operator==(const Edge &other);

    float hittest(BoundingBox2f _bounds);

private:

	void Init(int _ID, DrawnNode *a, DrawnNode *b, Metadata d); //float _r = 0, float _g = 0, float _b = 1, float _al = 1);
	int ID;
	//Linemode 0 is linear connections 1 will use polyline data if it exists
	int linemode;
	bool visible;
	//float r,g,b,a;

	bool interpolateFlag;
	std::vector <double> polylines;
	DrawnNode *start;
	DrawnNode *endMIGHTBENULL;
	Metadata data;

};

#endif /* EDGE_H_ */
