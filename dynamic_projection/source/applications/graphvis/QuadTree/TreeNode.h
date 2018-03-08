/*
 * TreeNode.h
 *
 *  Created on: Jul 18, 2011
 *      Author: phipps
 */

#ifndef TREENODE_H_
#define TREENODE_H_

#include "../../paint/gl_includes.h"

#include <vector>
#include <iostream>
#include <assert.h>

#include "../../paint/BoundingBox2f.h"

#define TREENODE_MAX_OCCUPANT 50
#define TREENODE_MAX_DEPTH 10

template <class T> class QuadTree;

//===========================================================
//===========================================================

template <class T>
class TreeNode{

public:

  //Constructors Destructors
  TreeNode(const Pt & _a, const Pt & _b, int _depth);
  virtual ~TreeNode();
  
  // ACCESSORS
  TreeNode<T>  **GetChildren() const { return children; }
  const std::vector<T*> *GetOccupants() const { return &occupants; }
  const BoundingBox2f& GetBounds() const { return bounds; }

  // MODIFIERS
  bool Add(T *val);
  
  void Draw() const;
  friend std::ostream& operator<< (std::ostream &ostr, const TreeNode<T> &t) {
    ostr << "Tree Node" << t.GetBounds() << " " << t.GetOccupants()->size() << std::endl;
    return ostr;
  }

  bool PrintNodesValIsIn(T* val) const;
  T* findvalwithid(unsigned int id) const;

  const std::string& getName() const { return name; }
	
private:
  
  friend class QuadTree<T>;

  // HELPER MODIFIERS
  void AddToChildren(T *val);
  void MakeChildren(); 
  void Clear();	//Deletes all children

  // REPRESENTATION
  std::vector<T*> occupants;
  
  //Children will be an array of 4 elements on run time
  TreeNode<T> **children;
  unsigned int depth;
  BoundingBox2f bounds;

  std::string name;


  void setName(const std::string& s) { name = s; }
};

//===========================================================
//===========================================================


template <typename T>
TreeNode<T>::TreeNode(const Pt & _a, const Pt & _b, int _depth) 
  : bounds(_a, _b) {
  depth = _depth;
  children = NULL;
  occupants.reserve(50);
  name = "root";
}


template <typename T>
TreeNode<T>::~TreeNode() {
  Clear();
}


template <typename T> bool TreeNode<T>::Add(T *val) {
  /*
  if (val->getID() == 469 && getName() == "root" ) { 
    std::cout << "checking bounds for 469 " << getName() << std::endl;
    std::cout << " node  " << bounds << std::endl;
    std::cout << " val   " << val->getBox() << std::endl;
  }
  */
  if (!val->Overlap(bounds)) 
    return false;
  if (children != NULL)  {
    AddToChildren(val);
  }
  else {
    //if (val->getID() == 469) { std::cout << "ADD 469 to " << getName() << std::endl; }
    occupants.push_back(val);
    if (occupants.size() > TREENODE_MAX_OCCUPANT && depth < TREENODE_MAX_DEPTH) {
      MakeChildren();
      for (unsigned int i = 0; i < occupants.size(); i++) {
        AddToChildren(occupants[i]);
      }
      occupants.clear();
    }
  }
  return true;
}

template <typename T>
bool TreeNode<T>::PrintNodesValIsIn(T* val) const {
  bool overlap = val->Overlap(bounds);
  bool inlist = false;
  for (unsigned int i = 0; i < occupants.size(); i++) {
    if (occupants[i]->getID() == val->getID()) inlist = true;
  }
  if (children != NULL) {
    inlist |= children[0]->PrintNodesValIsIn(val);
    inlist |= children[1]->PrintNodesValIsIn(val);
    inlist |= children[2]->PrintNodesValIsIn(val);
    inlist |= children[3]->PrintNodesValIsIn(val);
  }
  if (overlap != inlist) {
    std::cout << val->getID() << " & " << getName() << " SCREWED UP    overlap:" << 
      overlap << "   inlist:" << inlist << std::endl;
  }

  assert (overlap == inlist);
  return inlist;
}

template <typename T>
T* TreeNode<T>::findvalwithid(unsigned int id) const {
  T* answer = NULL;
  for (unsigned int i = 0; i < occupants.size(); i++) {
    if (occupants[i]->getID() == id) return occupants[i];
  }  
  if (children != NULL) {
    answer = children[0]->findvalwithid(id); if (answer != NULL) return answer;
    answer = children[1]->findvalwithid(id); if (answer != NULL) return answer;
    answer = children[2]->findvalwithid(id); if (answer != NULL) return answer;
    answer = children[3]->findvalwithid(id); if (answer != NULL) return answer;
  }
  return NULL;
}


template <typename T>
void TreeNode<T>::MakeChildren() {
  Pt a = bounds.GetMin();
  Pt b = bounds.GetMax();
  double dx = abs(a.x - b.x)/2.0;
  double dy = abs(a.y - b.y)/2.0;
  children = new TreeNode<T>*[4];
  children[0] = new TreeNode<T>(Pt(a.x     , a.y     ), Pt(a.x + dx, a.y + dy), depth + 1);
  children[1] = new TreeNode<T>(Pt(a.x + dx, a.y     ), Pt(b.x     , a.y + dy), depth + 1);
  children[2] = new TreeNode<T>(Pt(a.x     , a.y + dy), Pt(a.x + dx, b.y     ), depth + 1);
  children[3] = new TreeNode<T>(Pt(a.x + dx, a.y + dy), Pt(b.x     , b.y     ), depth + 1);

  children[0]->setName(getName()+"_0");
  children[1]->setName(getName()+"_1");
  children[2]->setName(getName()+"_2");
  children[3]->setName(getName()+"_3");
}


template <typename T>
void TreeNode<T>::AddToChildren(T *val) {
  assert (children != NULL);
  //if (val->getID() == 469) { std::cout << "ADD 469 to children of " << getName() << std::endl; }
  int count = 0;
  for (unsigned int j = 0; j < 4; ++j) {
    if (children[j]->Add(val)) {
      count++;
    }
  }
  //  if (val->getID() == 469) { std::cout << "ADDED 469 to " << count << " children of " << getName() << std::endl; }
  assert (count >= 1);
}



template <typename T>
void TreeNode<T>::Clear() {
  if(children == NULL) {
    occupants.clear();
    // not deleting the occupants because they could be in multiple children
    // (this might be a leak.... not sure where they are/should be deleted)
  } else {
    assert (occupants.size() == 0);
    for(int i = 0; i < 4; ++i) {
      delete children[i];
    }
    delete [] children;
    children = NULL;
  }
}


template <typename T>
void TreeNode<T>::Draw() const {
  glLineWidth(2);

  /*
  if (findvalwithid(478)) {
    glColor3f(1,1,0);
  } else {
    glColor3f(1,0,1);
  }
  */

  if (children != NULL) {
    for(unsigned int i = 0; i < 4; ++i) {
      assert (children[i] != NULL);
      children[i]->Draw();
    }
  } else {
    bounds.DrawBB();
  }
}


#endif /* TREENODE_H_ */
