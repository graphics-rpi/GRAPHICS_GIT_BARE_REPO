#ifndef EDGE_H
#define EDGE_H

#include <climits>
#include <cstdio>
#include <vector>
#include <cassert>
#include <iostream>
#include "element.h"

class Element;
class ArgParser;

#define LENGTH_HISTORY 5

// ===================================================================
// half-edge data structure

class Edge { 

public:

  // ========================
  // CONSTRUCTORS & DESTRUCTOR
  Edge(int a, int b, Element *e) {
    assert (a != b);
    assert (e != NULL);
    verts[0] = a;
    verts[1] = b;
    element = e;
  }
  ~Edge() {  
  }

  // =========
  // ACCESSORS
  int operator [] (int i) const { 
    assert (element != NULL);
    assert (i == 0 || i == 1); return verts[i]; }
  int HasVertex(int v) const { 
    assert (element != NULL);
    return (v == verts[0] || v == verts[1]); }
  Element* getElement() const { 
    assert(element != NULL); 
    return element; }
  double getLength() const;
  void Print(const char *s = "");
  void print(std::ostream &ostr) const;
  
  const std::vector<Edge*>& getOpposites() { return opposites; }
  const std::vector<Edge*>& getSharedEdges() { return sharedEdges; }

  bool isOpposite(const Edge *e) const { 
    assert (e != this);
    for (std::vector<Edge*>::const_iterator i = opposites.begin(); i != opposites.end(); i++) {
      if (*i == e) return true;
    }
    return false;
  }
  bool isSharedEdge(const Edge *e) const { 
    //std::cout << "isSharedEdge " << *this << " " << *e << std::endl;
    assert (e != this);
    assert (e->getElement() != NULL);
    for (std::vector<Edge*>::const_iterator i = sharedEdges.begin(); i != sharedEdges.end(); i++) {
      if (*i == e) return true;
    }
    return false;
  }
  void addOpposite(Edge *e) { 
    //std::cout << "ADD OPPOSITE " << this << " " << e << std::endl; 
    //std::cout << "  this " << *this << std::endl;
    //std::cout << "  e    " << *e << std::endl;
    assert (!isOpposite(e)); opposites.push_back(e); }
  void addSharedEdge(Edge *e) { 
    //std::cout << "ADD SHARED" << std::endl; 
    assert (!isSharedEdge(e)); sharedEdges.push_back(e); }
  void removeOpposite(Edge *e) { 
    assert (isOpposite(e));
    int found = false;
    for (std::vector<Edge*>::iterator i = opposites.begin(); i != opposites.end(); i++) {
      if (*i == e) {
	assert (found == false);
	found = true;
	opposites.erase(i); 
	break;
      }
    }
    assert (found == true);
    assert (!isOpposite(e));
  }
  void removeSharedEdge(Edge *e) { 
    //std::cout << "REMOVE SHARED EDGE from ";
    //this->print(std::cout);
    //std::cout << " to ";
    //e->print(std::cout);
    //std::cout << std::endl;
    assert (isSharedEdge(e));
    bool found = false;
    for (std::vector<Edge*>::iterator i = sharedEdges.begin(); i != sharedEdges.end(); i++) {
      if (*i == e) {
	sharedEdges.erase(i); 
	found = true; 
	break;
      }
    }
    assert (found == true);
    assert (!isSharedEdge(e));
  }

  void Check() const;

  friend std::ostream& operator<<(std::ostream &ostr, const Edge &e);

private:
  Edge(const Edge&) { assert(0); }
  Edge& operator=(const Edge&) { assert(0); return *this; }

  // ==============
  // REPRESENTATION
  int verts[2];
  Element *element;

  std::vector<Edge*> opposites;
  std::vector<Edge*> sharedEdges;
};

// ===================================================================

#endif
