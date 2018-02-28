#ifndef _COLLECT_H_
#define _COLLECT_H_

#include "vectors.h"
#include "utils.h"
#include <vector>
#include <cassert>
#include "element.h"
#include "boundingbox.h"

class Vertex;
class VoronoiPlane;
class Edge;

// =================================================================
// COLLECTION ROUTINES

class Collect {

public:

  // the things you call:
  static void CollectElementsWithVertex(Element *e, int v, std::vector<Element*> &elements) {
    std::vector<int> vertices; vertices.push_back(v);
    CollectElements(e,elements,vertices,CollectHasAnyVertex); }
  static void CollectElementsWithEitherVertex(Element *e, int v1, int v2, std::vector<Element*> &elements) {
    std::vector<int> vertices; vertices.push_back(v1); vertices.push_back(v2);
    CollectElements(e,elements,vertices,CollectHasAnyVertex); }
  static void CollectElementsWithBothVertices(Element *e, int v1, int v2, std::vector<Element*> &elements) {
    std::vector<int> vertices; vertices.push_back(v1); vertices.push_back(v2);
    CollectElements(e,elements,vertices,CollectHasAllVertices); }
  static void CollectElementsWithAnyVertex(Element *e, const std::vector<int> &vertices, std::vector<Element*> &elements) {
    CollectElements(e,elements,vertices,CollectHasAnyVertex); }
private:
  // COLLECTION HELPER FUNCTIONS
  static void CollectElements(Element *seed, 
			      std::vector<Element*> &elements,
			      const std::vector<int> &vertices,
			      int (*test_func)(Element *, const std::vector<int>&));
  static int CollectHasAnyVertex(Element *e, const std::vector<int> &vertices) {
    assert (e != NULL);
     int count = vertices.size();
    int num_elem_vertices = e->numVertices();
    for (int i = 0; i < count; i++) {
      for (int j = 0; j < num_elem_vertices; j++) {
	if ((*e)[j] == vertices[i]) { return 1; } } }
    return 0; }
  static int CollectHasAllVertices(Element *e, const std::vector<int> &vertices) {
    assert (e != NULL);
    int count = vertices.size();
    for (int i = 0; i < count; i++) {
      if (!e->HasVertex(vertices[i])) return 0; }
    return 1; }  

public:
  static bool VectorContains(const std::vector<Element*> &v, Element *e) {
    for (unsigned int i = 0; i < v.size(); i++) {
      if (v[i] == e) return true;
    }
    return false;
  }


};




#endif 
