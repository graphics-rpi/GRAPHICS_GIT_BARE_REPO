#include "bvh.h"

#define BVH_SPLIT_COUNT_THRESHHOLD 10

// =========================================================================================
// =========================================================================================

// CONSTRUCTOR
BVH::BVH(const std::set<Element*> &input_elements, int depth_)  {

  int num_elements = input_elements.size();
  assert (num_elements > 0);
  depth = depth_;
    
  // ====================
  // COMPUTE BOUNDING BOX
  Vec3f centroid;
  (*input_elements.begin())->computeCentroid(centroid);
  bbox = new BoundingBox(centroid,centroid);
  for (std::set<Element*>::const_iterator itr = input_elements.begin();
       itr != input_elements.end(); itr++) {
    (*itr)->computeCentroid(centroid);
    bbox->Extend(centroid);
  }

  if (num_elements > BVH_SPLIT_COUNT_THRESHHOLD) {
    std::set<Element*> elements_a;
    std::set<Element*> elements_b;

    // DETERMINE WHICH AXIS TO SPLIT & DIVIDE ELEMENTS
    Vec3f dim = bbox->getMax() - bbox->getMin();
    if (dim.x() >= dim.y() && dim.x() >= dim.z()) {
      Split(BoundingBox(bbox->getMin(), bbox->getMax()-Vec3f(0.5*dim.x(),0,0)), input_elements,elements_a,elements_b);
    } else if (dim.y() >= dim.z()) {
      Split(BoundingBox(bbox->getMin(), bbox->getMax()-Vec3f(0,0.5*dim.y(),0)), input_elements,elements_a,elements_b);
    } else {
      Split(BoundingBox(bbox->getMin(), bbox->getMax()-Vec3f(0,0,0.5*dim.z())), input_elements,elements_a,elements_b);
    }
    child1 = new BVH(elements_a,depth+1);
    child2 = new BVH(elements_b,depth+1);
    elements = NULL;
  } else {
    child1 = NULL;
    child2 = NULL;
    elements = new std::set<Element*>(input_elements);
  }
}


// DESTRUCTOR
BVH::~BVH() {
  delete child1;
  delete child2;
  delete bbox;
  delete elements;
}


// =========================================================================================
// =========================================================================================

void BVH::Split(const BoundingBox &bbox, const std::set<Element*> &input_elements,
		std::set<Element*> &elements_a, std::set<Element*> &elements_b) {

  Vec3f centroid;
  for (std::set<Element*>::const_iterator itr = input_elements.begin();
       itr != input_elements.end(); itr++) {
    Vec3f centroid;
    (*itr)->computeCentroid(centroid);

    if (bbox.Contains(centroid)) {
      elements_a.insert(*itr);
    } else {
      elements_b.insert(*itr);
    }
  }
  assert (elements_a.size() > 0);
  assert (elements_b.size() > 0);
  assert (elements_a.size() + elements_b.size() == input_elements.size());
}

// =========================================================================================
// =========================================================================================

void BVH::query(const BoundingBox &querybox, std::set<Element*> &neighbors) const {
  if (querybox.Overlaps(*bbox)) {
    if (elements != NULL) {
      assert (child1 == NULL);
      assert (child2 == NULL);
      // add all these elements
      for (std::set<Element*>::const_iterator itr = elements->begin();
	   itr != elements->end(); itr++) {
	neighbors.insert(*itr);
      }
    } else {
      assert (child1 != NULL);
      assert (child2 != NULL);
      // recurse
      child1->query(querybox,neighbors);
      child2->query(querybox,neighbors);
    }
  }
}

// =========================================================================================
// =========================================================================================

