#include "edge.h"
#include "mesh.h"
#include "vertex.h"
#include "element.h"
#include "utils.h"

#include "argparser.h"

// =========================================================================
// =========================================================================

void Edge::Print(const char *s) {
  printf ("%s edge %7d %7d\n", s, verts[0],verts[1]);
  element->Print("  my parent");
  printf ("e done\n");
}

double Edge::getLength() const {
  assert (element != NULL);
  Mesh *m = element->getMesh();
  return DistanceBetweenTwoPoints(m->getVertex(verts[0])->get(),
				  m->getVertex(verts[1])->get());
}


std::ostream& operator<<(std::ostream &ostr, const Edge &e) {
  assert (e.element != NULL);
  ostr << "EDGE: ";
  e.print(ostr);
  ostr << "  num sharededges " << e.sharedEdges.size() << std::endl;
  for (unsigned int i = 0; i < e.sharedEdges.size(); i++) {
    ostr << "   " << i << "  " << e.sharedEdges[i] << " ";
    assert (e.sharedEdges[i] != NULL);
    e.sharedEdges[i]->print(ostr);
    //ostr << std::endl;
  }
  ostr << "  num opposites   " << e.opposites.size() << std::endl;
  for (unsigned int i = 0; i < e.opposites.size(); i++) {
    ostr << "   " << i << "  " << e.opposites[i]  << " ";
    assert (e.opposites[i] != NULL);
    e.opposites[i]->print(ostr);
    //ostr << std::endl;
  }
  return ostr;
}

void Edge::print(std::ostream &ostr) const {
  //std::cout << "IN PRINT " << this << std::endl;
  assert (element != NULL);
  ostr << "Edge " << verts[0] << " " << verts[1] << " " << *element; // << std::endl;
}



void Edge::Check() const {
  //std::cout << "in edge check " << *this << std::endl;  

  Mesh *m = getElement()->getMesh();

  const std::vector<Edge*>& shared2 = m->GetEdges(verts[0],verts[1]);
  const std::vector<Edge*>& opposites2 = m->GetEdges(verts[1],verts[0]);

  assert (opposites.size() == opposites2.size());
  assert (sharedEdges.size()+1 == shared2.size());

  for (unsigned int i = 0; i < opposites.size(); i++) {
    assert (opposites[i] != NULL);
    opposites[i]->isOpposite(this);
  }
  for (unsigned int i = 0; i < sharedEdges.size(); i++) {
    assert (sharedEdges[i] != NULL);
    sharedEdges[i]->isSharedEdge(this);
  }
  //std::cout << "leaving edge check" << std::endl;
}
