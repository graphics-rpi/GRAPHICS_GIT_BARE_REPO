/*
 * Edge.h
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */



#ifndef QTEDGE_H_
#define QTEDGE_H_
#include <vector>
#include "QTNode.h"
#include "QTElement.h"
#include "Metadata.h"
#include "../paint/path.h"
#include "../paint/DrawableObject.h"
#include "../../../../../remesher/src/vectors.h"
#include "../../../../../remesher/src/utils.h"

class QTNode;
class Metadata;

class QTEdge : public QTElement, public Path {

  friend std::ostream& operator<< (std::ostream &ostr, const QTEdge &e);

public:

  // CONSTRUCTORS, ETC.
  QTEdge(int _ID, QTNode *a, QTNode *b, Metadata d, const Vec3f &_color, const std::deque<Pt> &_path );

  // ACCESSORS
  QTNode* GetStart() const {return start;}
  bool IsEndValid() const { return (endMIGHTBENULL != NULL); }
  QTNode* GetEnd() const { assert (IsEndValid()); return endMIGHTBENULL;}
  const Metadata* GetData() const { return &data; }
  //Pt getPosition(){return Pt(0,0);}
  Pt getPosition();//{return start->getPosition();}

  int GetDrawMode() const { return draw_mode; }
  virtual bool Overlap(const BoundingBox2f &bb) const;
  
  //  bool getStatus() const;
  const std::string& getDamageInfo() const;
  bool isDamaged() const;

  double DistanceFrom(const Pt &p) const;

  bool displayText(){
      return draw_mode != 1;
  }
  std::string getText();

  virtual bool isVisible() const;
  virtual double  getZOrder() const;

  // MODIFIERS
  Metadata* GetData() {return &data;}
  bool AnimateSize();
  void SetDrawMode(int i);


  void Draw();
  virtual void paint(const Vec3f &background_color=Vec3f(0,0,0)) const { return; }

protected:
  void ComputeBoundingBox(const std::deque<Pt> &path);

private:
  const static unsigned int WIDE_LINE_WIDTH;
  const static unsigned int THIN_LINE_WIDTH;

  QTNode *start;
  QTNode *endMIGHTBENULL;
  std::deque<Pt> path;
  Metadata data;
  int draw_mode;

  
};

#endif /* QTEdge_H_ */
