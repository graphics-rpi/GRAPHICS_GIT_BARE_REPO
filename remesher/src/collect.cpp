#include "collect.h"
#include "element.h"

// ==========================================================================
// a general purpose polygon collection routine

unsigned long collect_adds = 0;

void Collect::CollectElements(Element *seed, 
                              std::vector<Element*> &elements,
                              const std::vector<int> &vertices,
                              int (*test_func)(Element *, const std::vector<int> &)) {

  assert (seed != NULL);
  assert(elements.empty());
  Element::NextMark();  

  // the "recursive" calls yet to do
  std::vector<Element*> todo;
  seed->Mark(); 
  todo.push_back(seed);

  int current = 0;
  while (current < (int)todo.size()) {
    // grab each element 
    Element *element = todo[current];
    current++;
    if (element == NULL) continue;
    assert(element->isMarked());
    // test for inclusion in the element bag
    if (!test_func(element,vertices)) continue;
    assert (!Collect::VectorContains(elements,element));
    elements.push_back(element);
    collect_adds++;

    // queue up the neighbors
    for (int i = 0; i < element->numVertices(); i++) {
      std::vector<Element*> vec = element->getNeighbors(i);
      for (unsigned int j = 0; j < vec.size(); j++) {
	Element *neighbor = vec[j];
	if (!neighbor->isMarked()) { 
	  neighbor->Mark();
	  todo.push_back(neighbor);
	} else {
	  assert(neighbor->isMarked());
	}
      }
    }
  }
}


// ==========================================================================

