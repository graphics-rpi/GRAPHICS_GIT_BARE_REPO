#include "element.h"
#include "hash.h"
#include "edge.h"
#include "mesh.h"
#include "vertex.h"
#include "triangle.h"
#include "argparser.h"
#include "accelerationgrid.h"
#include "collect.h"
#include <algorithm>

#include <iomanip>

// ===========================================================================
// ===========================================================================

// static class variable for MARKABLE
unsigned int Markable::currentMark = 1;

// static class variables for ELEMENT
ElementID Element::currentID = 1;
elementshashtype Element::all_elements; 
AccelerationGrid<Element*,ElementID>* Element::acceleration_grid = NULL;

// ===========================================================================
// ===========================================================================

Element::Element(Mesh *m, std::string mat) { 
  myID = currentID;
  currentID++; 
  assert (currentID < INT_MAX);
  all_elements[this->getID()] = this; //->Add(this);
  assert (GetElement(this->getID()) == this);
  mesh = m;

  std::string material_name;

  std::string::size_type fake_pos = mat.find("_FAKE",0);
  if (fake_pos == std::string::npos) {
    material_name = mat;
    sidedness = "";
  } else {
    material_name = mat.substr(0,fake_pos);
    int pos = fake_pos+5;
    int len = mat.length()-fake_pos-5-5;
    sidedness = mat.substr(pos,len);
  }

  material_ptr = &m->getMaterial(material_name);

  for (int i = 0; i < 6; i++) { acceleration_bb[i] = -1; }
  



  // JOSH ADDED
  //  for (int i=0; i < 3; i++)
  //{
  //  for (int j=0; j<NORMAL_GROUP_SIZE; j++)
  //  {
  //     normal_group_by_index[i][j]=myID;
  //     normal_group_weight_by_index[i][j]=1.0f/(float)NORMAL_GROUP_SIZE;
  //  }
  //}




  // TERRIBLE UGLY HACK
  // can't be done in the element constructor!
  // must be done in the triangle constructor for now  please fix me
  //  if (acceleration_grid != NULL) { acceleration_grid->AddElement(this); }

}

void Element::setMaterial(const std::string &s) {
  //material_name = s;
  material_ptr = &mesh->getMaterial(s); //material_name);
}

std::string Element::getRealMaterialName() const {
  return material_ptr->getName(); 
}


void Element::setSidedness(unsigned int i, char c) {
  while (i >= sidedness.length()) {
    sidedness.push_back('?');
  }
  assert (i < sidedness.size());
  sidedness[i] = c;
}

char Element::getSidedness(unsigned int i) const {
  if (i >= sidedness.length()) return 'b';
  assert (i < sidedness.length());
  return sidedness[i];
}

Element::~Element() { 
  elementshashtype::iterator foo = all_elements.find(this->getID());
  assert (foo != all_elements.end());

  if (acceleration_grid != NULL) { acceleration_grid->RemoveElement(this); }

  //cout << " successful erase " << endl;
  all_elements.erase(foo); 
}

Element* Element::GetElement(ElementID id) {
  assert(id > 0 && id < currentID);
  elementshashtype::iterator foo = all_elements.find(id); 
  if (foo == all_elements.end()) {
    //cout << " uh oh, element not here to return " << endl;
    return NULL;
  }
  assert (foo != all_elements.end());
  return foo->second;
  //return elements->Get(id);
}

// ===========================================================================

std::vector<Element*> Element::getNeighbors(int i) const {

  std::vector<Element*> answer;
  if (isALine()) return answer;

  assert (i >= 0 && i < numVertices()); 
  assert (numVertices() > 0);
  //int a = (*this)[i];
  //int b = (*this)[(i+1)%numVertices()];
  //Edge *e = getMesh()->GetEdge(a,b,this);
  Edge *e = this->get_my_edge(i); //getMesh()->GetEdge(a,b,this);
  assert (e != NULL);

  //std::vector<Edge*> opposites = this->getMesh()->GetEdges(b,a);
  //std::vector<Edge*> shared_edges = this->getMesh()->GetEdges(a,b);

  const std::vector<Edge*> &opposites = e->getOpposites();
  const std::vector<Edge*> &shared_edges = e->getSharedEdges();

  for (int i = 0; i < (int)opposites.size(); i++) {
    Element *n = opposites[i]->getElement();
    answer.push_back(n);
    assert (n != this);
  }

  //bool found_me = false;
  for (int i = 0; i < (int)shared_edges.size(); i++) {
    Element *n = shared_edges[i]->getElement();
    //if (n == this) { assert (found_me == false); found_me = true; continue; }
    answer.push_back(n);
  }
  //assert (found_me = true);

  //assert (answer.size() == opposites.size() + shared_edges.size() - 1);
  assert (answer.size() == opposites.size() + shared_edges.size());

  return answer;
}

bool Element::hasNeighbor(const Element *e) const {
  for (int j = 0; j < numVertices(); j++) {
    std::vector<Element*> vec = getNeighbors(j);
    for (int i = 0; i < (int)vec.size(); i++) {
      if (vec[i] == e) return true;
    }
  }
  return false;
}

#if 0
Element* Element::getNeighbor(int i) const {
  assert (i >= 0 && i < numVertices()); 
  assert (numVertices() > 0);
  int a = (*this)[i];
  int b = (*this)[(i+1)%numVertices()];
  Edge *e = getMesh()->GetEdge(a,b);
  assert (e != NULL);

  std::vector<Edge*> opposites = e->getOpposites();
  std::vector<Edge*> shared_edges = e->getSharedEdges();
  
  if (opposites.size() == 0 && shared_edges.size() == 0) {
    return NULL;
  }

  if (opposites.size() == 1 && shared_edges.size() == 0) {
    Element *n = opposites[0]->getElement();
    assert (n != NULL);
    return n;
  }

  cout << "multiple neighbor possibilities!" << endl;
  exit(0);
}
#endif


void Element::getBoundingBox(BoundingBox &bbox) const {
  int n = numVertices();
  //  printf ("num verts %d\n", n);
  if (n == 0) {
    printf ("UNHAPPY ELEMENT!!!\n");
    return;
    
  }
  assert (n >= 1);
  

  Vec3f v = getMesh()->getVertex((*this)[0])->get();  
  bbox = BoundingBox(v,v);
  for (int i = 1; i < n; i++) {
    v = getMesh()->getVertex((*this)[i])->get();
    bbox.Extend(v);
  }
}

void Element::computeNormal(Vec3f &normal) const {
  if (isALine()) { return; }

  assert (numVertices() >= 3);
  computeNormal(getMesh(),(*this)[0],(*this)[1],(*this)[2],normal); 
  //if (fabs(normal.Length() - 1) < 0.1) return;
}

void Element::computeNormal(Mesh *m, int a, int b, int c, Vec3f &normal) {
  ::computeNormal(m->getVertex(a)->get(),
                  m->getVertex(b)->get(),
                  m->getVertex(c)->get(),
                  normal);


}

/*
double LongestEdge(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  return max3(DistanceBetweenTwoPoints(a,b),
	      DistanceBetweenTwoPoints(a,c),
	      DistanceBetweenTwoPoints(b,c));
}

double LongestEdge(Mesh *m, int a, int b, int c) {
  return LongestEdge(m->getVertex(a)->get(),
		     m->getVertex(b)->get(),
		     m->getVertex(c)->get());
}
*/

void Element::computeCentroid(Vec3f &centroid) const {

  if (isALine()) {
    Vertex *v0 = getMesh()->getVertex((*this)[0]);
    Vertex *v1 = getMesh()->getVertex((*this)[1]);
    centroid = (v0->get() + v1->get()) * 0.5;
    
  } else {
    Vertex *v0 = getMesh()->getVertex((*this)[0]);
    Vertex *v1 = getMesh()->getVertex((*this)[1]);	
    Vertex *v2 = getMesh()->getVertex((*this)[2]);
    double x = (v0->get().x() + v1->get().x() + v2->get().x()) / 3.0;
    double y = (v0->get().y() + v1->get().y() + v2->get().y()) / 3.0;
    double z = (v0->get().z() + v1->get().z() + v2->get().z()) / 3.0;
    
    centroid = Vec3f(x,y,z);
  }
}

/*
double Element::LongestEdge() {
  if (isATriangle()) {
    Vec3f a = getMesh()->getVertex((*this)[0])->get();
    Vec3f b = getMesh()->getVertex((*this)[1])->get();
    Vec3f c = getMesh()->getVertex((*this)[2])->get();
    return LongestEdge (a,b,c);
  } else {
    assert (isAQuad());

  }
}
*/

double Element::ShortestEdge() const {
  Vec3f a = getMesh()->getVertex((*this)[0])->get();
  Vec3f b = getMesh()->getVertex((*this)[1])->get();
  Vec3f c = getMesh()->getVertex((*this)[2])->get();
  return min3(DistanceBetweenTwoPoints(a,b),
	      DistanceBetweenTwoPoints(a,c),
	      DistanceBetweenTwoPoints(b,c));
}


void Element::LongestEdge(int &i, int &a, int &b, double &longest) const {
  Mesh *m = getMesh();

  int num_verts = numVertices();
  i = -1;

  for (int j = 0; j < num_verts; j++) {
    int p = (*this)[j];
    int q = (*this)[(j+1)%num_verts];
    Vertex *vp = m->getVertex(p);
    Vertex *vq = m->getVertex(q);
    double dist = DistanceBetweenTwoPoints(vp->get(),vq->get());
    if (i == -1 || longest < dist) {
      i = j;
      a = p;
      b = q;
      longest = dist;
    }
  }
}
 
  /*
    Vertex *v2 = m->getVertex((*this)[2]);
    
    double d0 = DistanceBetweenTwoPoints(v0->get(),v1->get());
    double d1 = DistanceBetweenTwoPoints(v1->get(),v2->get());
    double d2 = DistanceBetweenTwoPoints(v2->get(),v0->get());
    
    if (d0 >= d1 && d0 >= d2) {
      i = 0;
      a = (*this)[0];
      b = (*this)[1];
      longest = d0;
    } else if (d1 >= d0 && d1 >= d2) {
      i = 1;
      a = (*this)[1];
      b = (*this)[2];
    longest = d1;
  } else {
    assert (d2 >= d0 && d2 >= d1);
    i = 2;
    a = (*this)[2];
    b = (*this)[0];
    longest = d2;
  }

}
  */

double Element::Angle(int input) { //const {
  if (1) { //angles.size() == 0) {
    angles.clear();
    int n = numVertices();
    for (int j = 0; j < numVertices(); j++) { 
      int v = (*this)[j];
      int a = (*this)[(j+1)%n];
      int b = (*this)[(j+n-1)%n];
      Vec3f va = getMesh()->getVertex(a)->get(); 
      va -=      getMesh()->getVertex(v)->get(); 
      Vec3f vb = getMesh()->getVertex(b)->get(); 
      vb -=      getMesh()->getVertex(v)->get(); 
      double angle = AngleBetween(va,vb);
      angles.push_back(angle);
    }
  } 
  assert ((int)angles.size() == numVertices());
  return angles[input];
}

void Element::computeMovedNormal(Vec3f &normal) const {
  Mesh *m = getMesh();
  ::computeNormal(m->getVertex((*this)[0])->getPositionOrMovedPosition(),
                  m->getVertex((*this)[1])->getPositionOrMovedPosition(),
                  m->getVertex((*this)[2])->getPositionOrMovedPosition(),
                  normal);
}

double Element::MovedPositionArea() const {
  Mesh *m = getMesh();
  int num_verts = numVertices();
  double answer = 0;
  for (int i = 0; i < num_verts-2; i++) {
    answer += ::AreaOfTriangle(m->getVertex((*this)[i])->getPositionOrMovedPosition(),
			       m->getVertex((*this)[i+1])->getPositionOrMovedPosition(),
			       m->getVertex((*this)[i+2])->getPositionOrMovedPosition());
  }
  return answer;
}

double Element::ShortestMovedEdge() const {
  double answer = -1;
  Mesh *m = getMesh();
  int num_verts = numVertices();
  for (int i = 0; i < num_verts; i++) {
    Vec3f a = m->getVertex((*this)[i])->getPositionOrMovedPosition();
    Vec3f b = m->getVertex((*this)[(i+1)%num_verts])->getPositionOrMovedPosition();
    double dist = DistanceBetweenTwoPoints(a,b);
    if (answer < 0 || dist < answer)
      answer = dist;
  }
  return answer;
}

void Element::normalizeBlendWeights(ArgParser *args) {
  assert (this->isATriangle());
  for (int i = 0; i < 3; i++) {
    double total = 0;
    int best_proj = -1;
    double best_value = 0;
    for (int j = 0; j < num_projectors; j++) {
      double tmp = getBlendWeight(i,j);
      if (tmp < 0.001) continue;
      total += tmp;
      if (best_proj == -1 || best_value < tmp) {
	best_proj = j;
	best_value = tmp; 
      }
    }
    if (total < 0.001) continue;
    if (args->single_projector_blending) {
      for (int j = 0; j < num_projectors; j++) {
	if (j == best_proj) 
	  setBlendWeight(i,j,1);
	else
	  setBlendWeight(i,j,0);
      }
    } else {
      for (int j = 0; j < num_projectors; j++) {
	setBlendWeight(i,j,getBlendWeight(i,j)/total);
      }
    }
  }
}

bool weight_sorter(const std::pair<ElementID,double> &a, const std::pair<ElementID,double> &b) {

  //  std::cout << "compare " << a.second << " " << b.second << std::endl;
  if (a.second >= b.second) return true;
  return false;

  //  return true;
}
                   


// ADDED FOR LSVO
// return all elements that have same material & similar normal
// and share this vertex, the weights will sum to 1.0
std::vector<std::pair<ElementID,double> > Element::getNeighborsForInterpolation(int corner, int num_neighbors) {

  assert (corner >= 0 && corner < numVertices());

  std::vector<std::pair<ElementID,double> > answer;


  std::vector<Element*> neighbors;
  Collect::CollectElementsWithVertex(this, (*this)[corner], neighbors);
  assert (neighbors.size() >= 1);
  
  double sum = 0;
  Vec3f mynormal;
  computeNormal(mynormal);
  std::string myname = this->getMaterialPtr()->getSimpleName();
  for (unsigned int n = 0; n < neighbors.size(); n++) {
    Element *e = neighbors[n];
    Vec3f enormal;
    e->computeNormal(enormal);
    double ang = AngleBetweenNormalized(mynormal,enormal);    
    std::string ename = e->getMaterialPtr()->getSimpleName();
    if (e != this &&
	(ename != myname ||
	 ang > 25*M_PI/180.0)) {
      continue;      
    }
    double angle = e->Angle((e->WhichVertex((*this)[corner])+0)%numVertices());
    sum += angle;
    assert (angle > 0);
    answer.push_back(std::pair<ElementID,double>(e->getID(),angle));  
  }
  assert (answer.size() > 0);
  assert (sum > 0);

  if (num_neighbors != -1) {
    assert (num_neighbors >= 1);
    std::sort(answer.begin(),answer.end(),weight_sorter);
    // remove extra elements
    while (num_neighbors < (int)answer.size()) {
      sum -= answer.back().second;
      answer.pop_back();
    }
    // fill with zeros
    while (num_neighbors > (int)answer.size()) {
      answer.push_back(std::pair<ElementID,double>(this->getID(),0));
    }
  }

  // normalize the weights
  for (unsigned int i = 0; i < answer.size(); i++) {
    answer[i].second /= double(sum);
  }

  return answer;
}


void PrintAllElements(Mesh *mesh) {

  std::cout << "PRINT ALL ELEMENTS " << mesh->numElements() << std::endl;


  elementshashtype::const_iterator foo;

  for (foo = mesh->getElements().begin();foo != mesh->getElements().end();foo++) {
    Element *e = foo->second;    
    std::cout << "ELEMENT " << e->getID() << std::endl;
    for (int i = 0; i < e->numVertices(); i++) {

      std::cout << "   " << i << ":  vertex " << (*e)[i] << std::endl;

      //std::vector<std::pair<ElementID,double> > neighbors = e->getNeighborsForInterpolation(i,8);
      std::vector<std::pair<ElementID,double> > neighbors = e->getNeighborsForInterpolation(i,3);
      std::cout << "  vert " << std::setw(3) << i << " has " << neighbors.size() << " neighbors:  " << std::endl;
      double sum = 0;
      for (unsigned int j = 0; j < neighbors.size(); j++) {
        std::cout << "    " << std::setw(3) 
                  << j << "  " << std::setw(3) 
                  << neighbors[j].first << "  " << std::setw(5) << neighbors[j].second << std::endl; 
	sum += neighbors[j].second;
      }
      std::cout << "SUM = " << sum << std::endl;
      assert (fabs(sum-1.0) < 0.00001);
    }
    
  }
  

}
