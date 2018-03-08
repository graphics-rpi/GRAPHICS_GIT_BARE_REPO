#ifndef QTELEMENT_H_
#define QTELEMENT_H_

#include "../../paint/ClickableObject.h"
#include "../../../calibration/planar_interpolation_calibration/planar_calibration.h"

class QTElement : virtual public ClickableObject{

public:
  
  virtual ~QTElement() {}

  // ========================
  // BOUNDING BOX 
  //  void setBox(const Pt & a, const Pt & b);
  const BoundingBox2f &getBox() const;
  //virtual void paint() =0;
  virtual Pt getPosition()=0;

  virtual bool Overlap(const BoundingBox2f &bb) const {
    return boundingbox.Overlap(bb);
  }

  friend std::ostream& operator<< (std::ostream &ostr, const QTElement &e);

  //  double getLastDistance() const { return last_distance; }
  //void setLastDistance( double dist ){ last_distance = dist; }
  unsigned int getID() const { return my_id; }

protected:
  // ============
  // CONSTRUCTORS
  QTElement( unsigned int id, const Pt &p, double w, double h, const Vec3f &c /*const Pt & _a, const Pt & _b, bool reorder = false */);

  // BoundingBox 
  BoundingBox2f boundingbox;
  
  //  double last_distance;
 private:
  unsigned int my_id;
};

#endif 
