#ifndef __BVH_H__
#define __BVH_H__

#include <set>
#include "element.h"
#include "boundingbox.h"


// ==================================================
// SIMPLE BOUNDING VOLUME HIERARCHY IMPLEMENTATION

class BVH {
 public:

  // CONSTRUCTOR & DESTRUCTOR
  BVH(const std::set<Element*> &seeds, int depth_ = 0);
  virtual ~BVH();

  // THE IMPORTANT FUNCTION!
  void query(const BoundingBox &querybox, std::set<Element*> &neighbors) const;

 private:

  // HELPER FUNCTION
  static void Split(const BoundingBox &bbox, const std::set<Element*> &input_elements,
		    std::set<Element*> &elements_a, std::set<Element*> &elements_b);

  // REPRESENTATION
  BoundingBox *bbox;
  // if children are null elements is not or vice versa
  BVH *child1;
  BVH *child2;
  std::set<Element*> *elements;
  int depth;

};

// ==================================================

#endif
