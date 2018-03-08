/*
 * QuadTree.h
 *
 *  Created on: Jul 18, 2011
 *      Author: phipps
 */

#ifndef QUADTREE_H_
#define QUADTREE_H_

#include "TreeNode.h"
#include "../../paint/ClickableObject.h"
#include <stack>
#include <math.h>
#include <algorithm>
#include "../../../../../synenv/visualizations/MapView/mapcore/TransformationManager.h"
#include "../../../../../synenv/visualizations/MapView/mapcore/AZMatrix3.h"
extern TransformationManager g_xforms;

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template <typename T>
class QuadTree {

public:

  //Constructors Destructors
  QuadTree(const Pt & _a, const Pt & _b); 
  virtual ~QuadTree() { Clear(); }
  
  // ACCESSORS
  const BoundingBox2f& GetBounds() const { return root.GetBounds(); }
  TreeNode<T>* GetRoot() { return &root; };
  
  // MODIFIERS
  void Add(T* val);
  void Clear() { root.Clear(); }
  
  // QUERY
  std::vector<std::pair<T*,double> > Query(const BoundingBox2f & _bounds, bool visibility = false);
  
  //Debugging methods
  void DrawTree();
  
private:	

  // helper functions
  void Query_helper(std::vector<std::pair<T*,double> > &answer, TreeNode<T> *node, const BoundingBox2f & _bounds, bool visibility);

  double GetPixelDistance(const QTElement * testing_object, const Pt & clicked_point );


  // REPRESENTATION
  TreeNode<T> root;
  BoundingBox2f last_query;
};


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


// CONSTRUCTOR
template <typename T>
QuadTree<T>::QuadTree(const Pt & _a, const Pt & _b) : 
  root(TreeNode<T>(_a, _b, 0)) , last_query(_a, _b)
{
  //  std::cout << "NEW QUAD TREE " << GetBounds() << std::endl;
}



// helper function used to get the distance between
// two in screen space not the state specific coordinate system.
template <typename T>
double QuadTree<T>::GetPixelDistance(const QTElement * testing_element, const Pt & clicked_point){

  //  double absdist = testing_element->GetAbsoluteDistance(clicked_point);
  
  ///if(testing_element->PointInside(clicked_point)){
  //return 0.0;
  //}	

#if 0
  float click_x, click_y;
  click_x = clicked_point.x;
  click_y = clicked_point.y;
  click_y -= 524288*0.83105;
  
  AZMatrix3 image_to_screen = g_xforms.GetTransformation("image", "screen");
  image_to_screen.Transform(click_x, click_y);
#endif

  //Pt newPt = Pt(click_x, click_y);
  
  return testing_element->DistanceFrom(clicked_point);
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// QUERY "DRIVER FUNCTION"

template <typename T>
std::vector<std::pair<T*,double> > QuadTree<T>::Query(const BoundingBox2f & _bounds, bool visibility) {

  /*
  T* val_469 = root.findvalwithid(469);
  T* val_478 = root.findvalwithid(478);

  assert (val_469 != NULL);
  assert (val_478 != NULL);
 
  std::cout << "\n\n";
  bool tmp = root.PrintNodesValIsIn(val_469);
  assert (tmp);
  std::cout << "\n\n";
  tmp = root.PrintNodesValIsIn(val_478);
  assert(tmp);
  */

  last_query = _bounds;
  std::vector<std::pair<T*,double> > answer;
  Query_helper(answer, &root, _bounds, visibility);
  return answer;
}


// RECURSIVE QUERY HELPER FUNCTION

template <typename T>
void QuadTree<T>::Query_helper(std::vector<std::pair<T*,double> > &answer, TreeNode<T> *node, const BoundingBox2f & _bounds, bool visibility) {

  double tempdist;
  double mindist = _bounds.Radius();
  double bestdist = -1;

  if (!_bounds.Overlap(node->GetBounds())) { return; }

  const std::vector<T*> *occupants = node->GetOccupants();

  TreeNode<T>** children = node->GetChildren();
  if (children != NULL) {
    assert (occupants->size() == 0);
    for (int i = 0; i < 4; i++) {
      Query_helper(answer, children[i],_bounds,visibility);
    }
  }
  
  else {

    /*
    std::cout << std::setw(16) << node->getName() << "  Num Occupants: " << occupants->size();
    std::cout << "    min dist " << mindist << std::endl;
    */
    for(unsigned int i = 0; i < occupants->size(); ++i) {

      QTElement* current_element = (*occupants)[i];

      if (!current_element->isVisible()) continue;

      tempdist = GetPixelDistance( current_element, _bounds.GetCenter() );
      if (bestdist < 0 || tempdist < bestdist) {
        bestdist = tempdist;
      }

      
      //std::cout << "["<<std::setw(3)<<i<<"] my:";
      if (tempdist <= mindist) {
        //std::cout << " ADD ";
        answer.push_back(std::make_pair(current_element,tempdist));
      } else {
        //std::cout << "     ";
      }
      /* 
      std::cout << std::setw(10) << std::fixed << std::setprecision(1) << tempdist 
                << " best:" 
                << std::setw(10) << std::fixed << std::setprecision(1) << bestdist << " e:" << *current_element;
      */
    }
  }

}



// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


template <typename T>
void QuadTree<T>::DrawTree()
{
  //return;

  glColor3f(0,0,1);
  glLineWidth(2);
  root.Draw();
  glColor3f(1,0,0);
  glLineWidth(10);
  
  last_query.DrawBB();
}

template <typename T>
void QuadTree<T>::Add(T* val) { 

  /*
    if (val->getID() == 469) {
    std::cout << "QuadTree::Add id=" << val->getID() << std::endl;
    std::cout << "    " << val->getBox() << std::endl;
  }
  */

  bool success = root.Add(val); 
  assert (success); 
}


//========================================================================


#endif /* QUADTREE_H_ */
