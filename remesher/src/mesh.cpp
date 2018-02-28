#include <cstdio>
#include "utils.h"
#include "mesh.h"
#include "meshmanager.h"
#include "element.h"
#include "edge.h"
#include "vertex.h"
#include "element.h"
#include "triangle.h"
#include "quad.h"
#include "polygon.h"
#include "collect.h"

#define INITIAL_ELEMENT_BAG 10000
#define PLANE_TOLERANCE 0.999999

extern double EXTRA_LENGTH_MULTIPLIER;
#include "argparser.h"
extern ArgParser *ARGS;

// =======================================================================
// CONSTRUCTORS & DESTRUCTORS
// =======================================================================

Mesh::Mesh() {
  bbox = NULL;
  modified = true;
  area = -1;
  weighted_area = -1;
  zero_area_tolerance = -1;
  longest_edge = -1;
  shortest_edge = -1;
  north_angle_copy = 0;
}

Mesh::~Mesh() {
  Clear();
  assert (vertices.size() == 0);
  assert (edges.size() == 0);
  assert (elements.size() == 0);
  std::cerr << " WARNING: MESH IS BEING DESTROYED " << std::endl;
}

unsigned int Mesh::numLines() const {
  unsigned int answer = 0;
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    if (e->isALine())
      answer++;
    else {
      assert (e->isATriangle() || e->isALine());
    }
  }
  return answer;
}

unsigned int Mesh::numTriangles() const {
  unsigned int answer = 0;
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    if (e->isATriangle())
      answer++;
    else {
      assert (e->isAQuad() || e->isALine());
    }
  }
  return answer;
}


unsigned int Mesh::numQuads() const {
  unsigned int answer = 0;
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    if (e->isAQuad())
      answer++;
    else {
      assert (e->isATriangle() || e->isALine());
    }
  }
  return answer;
}

void Mesh::addMaterial(const MeshMaterial &m) {
  (*ARGS->output) << "add material " << m.getName() << std::endl;
  assert (materials.find(m.getName()) == materials.end());
  materials[m.getName()] = m;
}


unsigned int Mesh::numBadElements() const {
  unsigned int answer = 0;
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    if (e->IsBad())
      answer++;
  }
  return answer;
}


// =======================================================================
// =======================================================================

void Mesh::Clear() {
  Modified();
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    RemoveEdges(e);
    delete e;
  }
  elements.clear();
  edges.clear();
  for (unsigned int i = 0; i < vertices.size(); i++) {
    delete vertices[i];
  }
  vertices.clear();
  vertex_parents.clear();
  clearMaterials();
  delete bbox;
  bbox = NULL;
  modified = true;
}


void Mesh::CompressVertices() {
  (*ARGS->output) << "compress vertices" << std::endl;
  Modified();

  std::vector<Vertex*> vertices2 = vertices;
  vertices.clear();
  elementshashtype elements2 = elements;
  elements.clear();
  edges.clear();
  vertex_parents.clear();

  std::vector<int> vert_mapping(vertices2.size(),-1);

  for (elementshashtype::const_iterator foo = elements2.begin();
       foo != elements2.end();
       foo++) {
    Element *e = foo->second;
    assert (e->numVertices() == 3);
    int a = (*e)[0];
    int b = (*e)[1];
    int c = (*e)[2];

    if (vert_mapping[a] == -1) { vert_mapping[a] = addVertex(vertices2[a]->get(),-1,-1,0,0); }
    if (vert_mapping[b] == -1) { vert_mapping[b] = addVertex(vertices2[b]->get(),-1,-1,0,0); }
    if (vert_mapping[c] == -1) { vert_mapping[c] = addVertex(vertices2[c]->get(),-1,-1,0,0); }

    addElement(new Triangle(vert_mapping[a],vert_mapping[b],vert_mapping[c],this,e->getRealMaterialName()));

    //RemoveEdges(e);
    delete e;
  }
  //elements.clear();
  //edges.clear();
  //for (unsigned int i = 0; i < vertices.size(); i++) {
  //delete vertices[i];
  //}/
  //vertices.clear();
  //vertex_parents.clear();
  //clearMaterials();
  //delete bbox;
  //bbox = NULL;
  //modified = true;

}


void Mesh::Check() {
  (*ARGS->output) << "MESH CHECK ... ";
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *elem = foo->second;
    int n = elem->numVertices();
    for (int i = 0; i < n; i++) {
      unsigned int v  = (*elem)[i];
      unsigned int v2 = (*elem)[(i+1)%n];
      assert (v < numVertices());
      Edge *edge = GetEdge(v,v2,elem);
      assert (edge != NULL);
      assert (edge->getElement() == elem);
    }
  }
  (*ARGS->output) << "done" << std::endl;
}

// =================================================================
// HELPER FUNCTIONS FOR EDGES
// =================================================================

const std::vector<Edge*>& Mesh::GetEdges(int a, int b) const {
  static std::vector<Edge*> emptyvec;
  edgeshashtype::const_iterator foo;
  foo = edges.find(std::make_pair(a,b));
  if (foo == edges.end()) {
    return emptyvec; 
  } else {
    return foo->second;
  }
}

Edge* Mesh::GetEdge(int a, int b, const Element *e) const {  
  int numVertices = e->numVertices();
  for (int i = 0; i < numVertices; i++) {
    if ((*e)[i] == a && (*e)[(i+1)%numVertices] == b)
      return e->get_my_edge(i);
  }
  assert(0);
  return NULL;
  /*
  std::vector<Edge*> matching_edges = GetEdges(a,b);
  for (unsigned int i = 0; i < matching_edges.size(); i++) {
    Edge *edge = matching_edges[i];
    if (edge->getElement() == e) 
      return edge;
  }
  assert(0);
  return NULL;
  */
}

Edge* Mesh::AddEdge(int a, int b, Element *e) {
  assert (e != NULL);
  //(*ARGS->output) << "AddEdge " << a << " " << b << " " << *e << std::endl;
  
  Modified();
  Edge *answer = new Edge(a,b,e);
  const std::vector<Edge*>& shared = GetEdges(a,b);

  if (shared.size() > 0) {
    //(*ARGS->output) << "ADD EDGE ON NON MANIFOLD MESH (shared) " << a << " " << b << std::endl;
  }
  for (unsigned int i = 0; i < shared.size(); i++) {
    shared[i]->addSharedEdge(answer);
    answer->addSharedEdge(shared[i]);
  }
  const std::vector<Edge*>& opposites = GetEdges(b,a);
  if (opposites.size() > 1) {
    //(*ARGS->output) << "ADDEDGE ON NON MANIFOLD MESH (opposite) " << a << " " << b << std::endl;
  }
  for (unsigned int i = 0; i < opposites.size(); i++) {
    opposites[i]->addOpposite(answer);
    answer->addOpposite(opposites[i]);
  }
  edges[std::make_pair(a,b)].push_back(answer); 

  //(*ARGS->output) << "ADDED AN EDGE:\n" << *answer << std::endl;

  const std::vector<Edge*>& shared2 = answer->getSharedEdges();
  const std::vector<Edge*>& opposites2 = answer->getOpposites();
  
  if (shared2.size() == 0 && shared.size() == 0) {
  } else {
    assert (shared2.size()+1 == shared.size());
  }
  assert (opposites2.size() == opposites.size());

  answer->Check();

  return answer;
}

void Mesh::RemoveEdge(Edge *e) { 

  

  //(*ARGS->output) << "remove edge " << *e << std::endl;
  e->Check();

  //std::vector<Edge*> others;

  const std::vector<Edge*> shared2 = e->getSharedEdges();
  const std::vector<Edge*> opposites2 = e->getOpposites();

  Modified();
  assert (e != NULL);
  int a = (*e)[0];
  int b = (*e)[1];
  const std::vector<Edge*>& shared = GetEdges(a,b);

  assert (shared.size() == shared2.size() + 1);

  //assert (shared.size() >= 1);
  //int found = false;
  //(*ARGS->output) << "num shared edges " << shared2.size() << std::endl;
  for (unsigned int i = 0; i < shared2.size(); i++) {
    //td::cout << "i " << i << " " << shared2.size() << std::endl;
    assert (shared2[i] != e);
      //assert (found == false); 
      //found = true;
      //} else {
      //(*ARGS->output) << "REMOVE EDGE ON NON MANIFOLD MESH (shared) " << std::endl;
    //others.push_back(shared2[i]);
    shared2[i]->removeSharedEdge(e);
    //(*ARGS->output) << "removing shared edge" << std::endl;
    e->removeSharedEdge(shared2[i]);
    //}
  }
  //assert (found == true);

  const std::vector<Edge*>& opposites = GetEdges(b,a);

  assert (opposites.size() == opposites2.size());

  if (opposites.size() > 1) {
    //(*ARGS->output) << "REMOVE EDGE ON NON MANIFOLD MESH (opposite) " << std::endl;
  }
  for (unsigned int i = 0; i < opposites2.size(); i++) {
    //others.push_back(opposites2[i]);
    //(*ARGS->output) << "removing opposite " << std::endl;
    opposites2[i]->removeOpposite(e);
    e->removeOpposite(opposites2[i]);
  }

  //(*ARGS->output) << "going to erase edge" << std::endl;
  edgeshashtype::iterator foo;
  foo = edges.find(std::make_pair((*e)[0],(*e)[1]));
  assert (foo != edges.end());
  std::vector<Edge*> &edgevec = foo->second;
  bool found2 = false;
  for (std::vector<Edge*>::iterator itr = edgevec.begin(); itr < edgevec.end(); itr++) {
    if (*itr == e) {
      edgevec.erase(itr);
      assert (found2 == false);
      found2 = true;      
      delete e;
      break;
    }
  }
  assert (found2 == true);
  if (edgevec.size() == 0) {
    edges.erase(foo);
  }
  //(*ARGS->output) << "edge erased" << std::endl;


  for (unsigned int i = 0; i < shared2.size(); i++) {
    //(*ARGS->output) << "checking edge " << *shared2[i] << std::endl;
    shared2[i]->Check();
  }
  for (unsigned int i = 0; i < opposites2.size(); i++) {
    //(*ARGS->output) << "checking edge " << *opposites2[i] << std::endl;
    opposites2[i]->Check();
  }


  //(*ARGS->output) << "done remove edge\n" << std::endl;
}

bool Mesh::IsSimpleBoundaryEdge(Edge *e) const { //, int a, int b) const {
  //assert ((*e)[0] == a && (*e)[1]==b);
  int num_shared = e->getSharedEdges().size()+1; 
  int num_opp = e->getOpposites().size(); 
  //int num_shared = GetEdges(a,b).size();
  //int num_opp = GetEdges(b,a).size();
  assert (num_shared >= 1);
  if (num_shared == 1 && num_opp == 0) return true; // it's a simple boundary
  return false;
}

bool Mesh::IsManifoldInteriorEdge(Edge *e) const { ///, int a, int b) const {
  //assert ((*e)[0] == a && (*e)[1]==b);
  int num_shared = e->getSharedEdges().size()+1; 
  int num_opp = e->getOpposites().size(); 
  //  int num_shared = GetEdges(a,b).size();
  //int num_opp = GetEdges(b,a).size();
  assert (num_shared >= 1);
  if (num_shared == 1 && num_opp == 1) return true;  // it's a proper interior edge
  return false;
}

bool Mesh::IsNonManifoldEdge(Edge *e) const { //, int a, int b) const {
  //assert ((*e)[0] == a && (*e)[1]==b);
  int num_shared = e->getSharedEdges().size()+1; 
  int num_opp = e->getOpposites().size(); 
  //int num_shared = GetEdges(a,b).size();
  //int num_opp = GetEdges(b,a).size();
  assert (num_shared >= 1);
  if (num_shared == 1 && num_opp == 0) return false;  // it's a simple boundary
  if (num_shared == 1 && num_opp == 1) return false;  // it's a proper interior edge
  return true;  // it's non manifold!
}


bool Mesh::IsSimpleBoundaryVertex(Element *e, int vert, std::vector<Element*> *elements) const {
  /*
  std::vector<Element*> my_elements;
  if (elements == NULL) {
    Collect::CollectElementsWithVertex(e,vert,my_elements);
    elements = &my_elements;
  }
  for (int i = 0; i < elements->size(); i++) {
    Element *elem = (*elements)[i];
    if (!elem->HasVertex(vert)) continue;
    assert (elem->isATriangle());
    Triangle *t = (Triangle*)elem;
    for (int j = 0; j < 3; j++) {
      Edge *ed = t->get_my_edge(j);
      if ((*ed)[0] != vert && (*ed)[1] != vert) continue;
      if (!IsManifoldInteriorEdge) return false;
    }
  }
  */
  return true;
}

bool Mesh::IsManifoldInteriorVertex(Element *e, int vert, std::vector<Element*> *elements) const {
  std::vector<Element*> my_elements;
  if (elements == NULL) {
    Collect::CollectElementsWithVertex(e,vert,my_elements);
    elements = &my_elements;
  }
  for (unsigned int i = 0; i < elements->size(); i++) {
    Element *elem = (*elements)[i];
    if (!elem->HasVertex(vert)) continue;
    assert (elem->isATriangle());
    Triangle *t = (Triangle*)elem;
    for (int j = 0; j < 3; j++) {
      Edge *ed = t->get_my_edge(j);
      if ((*ed)[0] != vert && (*ed)[1] != vert) continue;
      if (!IsManifoldInteriorEdge(ed)) return false;
    }
  }
  return true;
}


// =================================================================
// =================================================================
// MODIFIERS:   ADD & REMOVE
// =================================================================
// =================================================================

int min_non_neg(int a, int b, int c) {
  if (a == -1) {
    if (b == -1) {
      if (c == -1) 
	printf ("whoops c  == -1 too\n");
      //assert(c != -1);
      return c;
    } else if (c == -1) {
      assert(b != -1);
      return b;
    } else {
      return min2(b,c);
    }
  } 
  assert (a != -1);
  if (b == -1) {
    if (c == -1) {
      return a;
    } else {
      return min2(a,c);
    }
  }
  assert (b != -1);
  if (c == -1) {
    return min2(a,b);
  }
  assert (c != -1);
  return min3(a,b,c);
}

void vv_reorder(int &a, int &b, int &c) {
  // make sure there are no repeats!
  assert (a != b || (a== -1 && b == -1));
  assert (b != c || (b== -1 && c == -1));
  assert (c != a || (c== -1 && a == -1));
  int min = min_non_neg(a,b,c);
  if (min < 0) printf ("whoops its < 0\n");
  //  assert (min >= 0);

  assert (min == a || min == b || min == c);
  if (min == a) {
    return;
  } else if (min == b) {
    int tmp = a;
    a = b;
    b = c;
    c = tmp;
  } else {
    assert (min == c);
    int tmp = c;
    c = b;
    b = a;
    a = tmp;
  }
}

int Mesh::getChildVertex(int parent_a, int parent_b) const {
  vphashtype::const_iterator foo;
  if (parent_a < parent_b) {
    foo = vertex_parents.find(std::make_pair(parent_a,parent_b));
  } else {
    foo = vertex_parents.find(std::make_pair(parent_b,parent_a));
  }
  if (foo == vertex_parents.end()) {
    return -1;
  }
  return foo->second;
}

int Mesh::addVertex(const Vec3f &position, int parent_a, int parent_b, double s, double t) {
  Modified();
  int answer = numVertices();
  Vertex *v = new Vertex(position,s,t);
  if (parent_a != -1) {
    assert (parent_b != -1);
    assert (parent_a >= 0 && parent_a < (int)numVertices());
    assert (parent_b >= 0 && parent_b < (int)numVertices());

    //assert (getChildVertex(parent_a,parent_b) == -1);
    //HACK
    int test = getChildVertex(parent_a,parent_b);
    if (test != -1) return test;
    assert (getChildVertex(parent_a,parent_b) == -1);

    if (parent_a < parent_b) {
      vertex_parents[std::make_pair(parent_a,parent_b)] = answer;
    } else {
      vertex_parents[std::make_pair(parent_b,parent_a)] = answer;
    }
    assert (getChildVertex(parent_a,parent_b) == answer);
  }
  vertices.push_back(v);
  if (bbox == NULL) 
    bbox = new BoundingBox(position,position);
  else 
    bbox->Extend(position);
  
  return answer;
}


bool Mesh::addElement(Element *e) {


  assert (e != NULL);
  Modified();
  elementshashtype::iterator foo = elements.find(e->getID());
  assert (foo == elements.end());
  elements[e->getID()] = e;
  bool success = AddEdges(e);
  if (!success) {
    (*ARGS->output) << "ERROR couldn't add element" << std::endl;
    exit(0);
  }
  return success;
}


bool Mesh::AddEdges(Element *e) {
  if (e->isALine()) return true;

  assert (e != NULL);
  int n = e->numVertices();
  assert (n >= 3); 
  int i;
  Modified();
  for (i = 0; i < n; i++) {
    Edge *edge = AddEdge((*e)[i],(*e)[(i+1)%n],e);
    e->my_edges.push_back(edge);
  }
  return true;
}

void Mesh::RemoveEdges(Element *e) {

  if (e->isALine()) return;

  Modified();
  assert (e != NULL);
  int n = e->numVertices();
  assert (n >= 3);
  for (int i = 0; i < n; i++) {
    Edge *edge = GetEdge((*e)[i],(*e)[(i+1)%n],e);
    assert (edge != NULL);
    assert (edge->getElement() == e);
    RemoveEdge(edge);
  }
  e->my_edges.clear();
}

void Mesh::removeElement(Element *e) {
  Modified();
  //  bool a = isTriangleMesh();
  //bool b = e->isATriangle();
  //bool c = isPolygonMesh();
  //bool d = e->isAPolygon();
  //assert ((a&&b)||(c&&d));
  //assert((isTriangleMesh() && e->isATriangle()) ||
  //	   (isPolygonMesh()  && e->isAPolygon()));
  assert (e != NULL);
  RemoveEdges(e);
  elementshashtype::iterator foo = elements.find(e->getID()); 
  assert (foo != elements.end());
  elements.erase(foo);
  delete e; 
  e = NULL;
}


// -----------------------------------------------------------------
// -----------------------------------------------------------------

// UGLY HACK :(
extern ArgParser *ARGS;

void Mesh::ComputeNormals() {
  // calculate the vertex normals
  // first clear the normals
  unsigned int i;
  elementshashtype::const_iterator foo;
  for (i = 0; i < numVertices(); i++) {
    Vertex *v = getVertex(i);
    v->ClearNormal();
  }  
  // then increment the normals per face
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;    
    if (e->isALine()) continue;
    if (e->NearZeroArea()) continue;
    Vec3f normal; e->computeNormal(normal);
    for (int j = 0; j < e->numVertices(); j++) {
      int vert = (*e)[j];
      Vertex *v = getVertex(vert);
      double a = e->Angle(j);
      v->incrNormal(normal,a);
    }
  }
  // finally normalize them
  for (i = 0; i < numVertices(); i++) {
    Vertex *v = getVertex(i);
    v->normalizeNormal();
  }
}

void Mesh::ComputeBlendWeights() {

  (*ARGS->output) << "COMPUTE BLENDING WEIGHTS" << std::endl;

  if (ARGS->blending_file != "") return;

  // BLEND WEIGHTS
  elementshashtype::iterator foo;
  unsigned int num_projectors = ARGS->projector_names.size();
  if (num_projectors == 0) return;

  // create vectors for the projector centers & directions
  std::vector<Vec3f> projector_centers;
  std::vector<Vec3f> projector_directions;
  for (unsigned int j = 0; j < num_projectors; j++) {
    projector_centers.push_back(ARGS->projectors[j].getVec3fCenterReplacement());
    projector_directions.push_back(ARGS->projectors[j].getProjDirectionReplacement());
  }

  /*
  // HACK
  if (ARGS->no_blending_hack) {
    for (foo = elements.begin();foo != elements.end();foo++) {
      Element *e = foo->second;
      assert (e->isATriangle());
      for (unsigned int j = 0; j < num_projectors; j++) {
	e->setBlendWeight(0,j,1);
	e->setBlendWeight(1,j,1);
	e->setBlendWeight(2,j,1);
      }
    }
    return;
  }
  */

  // set the initial blend weights
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;
    //e->clearBlendWeights();  // not doing this, we're assuming that the number of projectors does not change!
    if (e->isALine()) continue;
    assert (e->isATriangle());
    if (!e->getMaterialPtr()->isProjection()) {
      // set all non-projection blend weights to zero
      for (unsigned int j = 0; j < num_projectors; j++) {
	e->setBlendWeight(0,j,0);
	e->setBlendWeight(1,j,0);
	e->setBlendWeight(2,j,0);
      }
    } else {
      // set all projection blend weights appropriately
      const std::string &sidedness = e->getSidedness();
      assert (e->isATriangle());
      int num_vertices = 3;
      for (int k = 0; k < num_vertices; k++) {
	Vec3f element_centroid; e->computeCentroid(element_centroid);
	Vec3f element_normal; e->computeNormal(element_normal);
	for (unsigned int j = 0; j < num_projectors; j++) {
	  if (sidedness[j] == 'b') {
	    e->setBlendWeight(k,j,0);
	  } else {
	    Vec3f to_proj = projector_centers[j] - element_centroid;
	    to_proj.Normalize();
	    double dot1 = element_normal.Dot3(to_proj);
	    double dot2 = -element_normal.Dot3(projector_directions[j]);
	    //if (dot < 0) dot = 0.01; ////assert (dot > 0.1);
	    //if (dot < 0.8) 
	    //(*ARGS->output) << "dot " << dot1 << " " << dot2 << std::endl;
	    if (dot1 < 0.15 || dot2 < 0.1) {
	      e->setBlendWeight(k,j,0);
	    } else {
	      //dot = 1;
	      if (ARGS->test_color_calibration) {
		e->setBlendWeight(k,j,1.0);
	      } else {
		e->setBlendWeight(k,j,dot1);
		//e->setBlendWeight(k,j,dot1);
	      }
	    }
	  }
	}
      }
    }
  }

  int num_verts = numVertices();
  std::vector<std::vector<Element*> > elements_with_vertex(num_verts);
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;
    assert (e != NULL);
    if (e->isALine()) continue;
    assert (e->isATriangle());
    int num_verts2 = 3;
    //int num_verts2 = e->numVertices();
    //(*ARGS->output) << "numverts " << num_verts2 << std::endl;
    for (int k = 0; k < num_verts2; k++) {
      int vert = (*e)[k];
      //(*ARGS->output) << "vert " << vert << std::endl;
      elements_with_vertex[vert].push_back(e);
    }
  }

  if (!ARGS->test_color_calibration) {
    // propagate to vertex neighbors
    // set the initial blend weights
    for (foo = elements.begin();foo != elements.end();foo++) {
      Element *e = foo->second;
      if (e->isALine()) continue;
      assert (e->isATriangle());
      if (!e->getMaterialPtr()->isProjection()) { continue; }
      int num_verts = 3;
      for (int k = 0; k < num_verts; k++) {
	const std::vector<Element*> &elements2 = elements_with_vertex[(*e)[k]];
	for (unsigned int j = 0; j < elements2.size(); j++) {
	  Element *e2 = elements2[j];
	  int w = e2->WhichVertex((*e)[k]);
	  assert (e2->isATriangle());
	  int num_verts2 = 3;
	  assert (w >= 0 && w < num_verts2);
	  for (unsigned int j = 0; j < num_projectors; j++) {
	    if (e->getBlendWeight(k,j) < e2->getBlendWeight(w,j)) {
	      //#if 1
	      e2->setBlendWeight(w,j,max2(0,e->getBlendWeight(k,j)));
	      //#else
	      //e2->setBlendWeight(w,j,max2(0.01,e->getBlendWeight(k,j)));
	      //#endif
	    }	  
	  }
	}
      }
    }
  }

  // normalize the blend weights
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;
    if (e->isALine()) continue;
    e->normalizeBlendWeights(ARGS);
  }

  //(*ARGS->output) << "SET BLEND WEIGHTS WITH DISTANCE" << std::endl;
  // compute all blend weights with distance!
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;
    if (e->isALine()) continue;
    int num_verts = e->numVertices();
    for (int v = 0; v < num_verts; v++) {
      Vertex *vert = this->getVertex((*e)[v]);
      std::vector<double> distances(num_projectors);
      std::vector<double> weights(num_projectors);
      std::vector<double> distanceweights(num_projectors);
      double distance_sum = 0;
      double weight_sum = 0;
      double distanceweight_sum = 0;
      for (unsigned int p = 0; p < num_projectors; p++) {
	distances[p] = 10*std::max(0.001,std::min(0.1,vert->getBlendDistanceFromOcclusion(p)));
	weights[p] = e->getBlendWeight(v,p);
	distanceweights[p] = distances[p]*weights[p];
	distance_sum += distances[p];
	weight_sum += weights[p];
	distanceweight_sum += distanceweights[p];
      }
      if (distanceweight_sum > 0) {
	assert (distanceweight_sum < 1.1);
	
	//(*ARGS->output) << "distanceweight_sum " << distanceweight_sum << std::endl;
	double tmp = distanceweight_sum; //max2(distanceweight_sum,0.2);
	for (unsigned int p = 0; p < num_projectors; p++) {
	  distanceweights[p] /= tmp; //distanceweight_sum;
	}
      }
      for (unsigned int p = 0; p < num_projectors; p++) {
	if (!ARGS->test_color_calibration) {
	  e->setBlendWeightWithDistance(v,p,distanceweights[p]);
	} else {
	  e->setBlendWeightWithDistance(v,p,weights[p]);
	}
      }
    }
  }
}

#if 1

bool adjustBlendDistanceFromOcclusion2(Vertex *a, Vertex *b, Vertex *c, int j) { 
  double distab = DistanceBetweenTwoPoints(a->get(),b->get());
  double distbc = DistanceBetweenTwoPoints(b->get(),c->get());
  double distac = DistanceBetweenTwoPoints(c->get(),a->get());

  assert (distab <= distbc + distac + 0.00001);
  assert (distac <= distbc + distab + 0.00001);
  assert (distbc <= distac + distab + 0.00001);

  double vala = a->getBlendDistanceFromOcclusion(j);
  double valb = b->getBlendDistanceFromOcclusion(j);
  double valc = c->getBlendDistanceFromOcclusion(j);
  assert (a != b && a != c && b != c);
  assert (vala <= valb && valb <= valc);

  //(*ARGS->output) << std::endl;
  //cout << "vals " << vala << " " << valb << " " << valc << std::endl;
  //cout << "dists " << distab << " " << distbc << " " << distac << std::endl;

  //cout << (distab*distab + distac*distac - distbc*distbc) << " / " 
  //   << (distab*distac) << " = " 
  //   << (distab*distab + distac*distac - distbc*distbc) / (distab*distac) << std::endl;

  double alpha  = acos(max2(-1.0,min2(1.0,(distab*distab+distac*distac-distbc*distbc)/(2*distab*distac))));
  //double alpha1 = acos(max2(-1.0,min2(1.0,(distab*distab+distbc*distbc-distac*distac)/(2*distab*distbc))));
  //double alpha2 = acos(max2(-1.0,min2(1.0,(distbc*distbc+distac*distac-distab*distab)/(2*distbc*distac))));

  //  cout << "alphas " << alpha << " " << alpha1 << " " << alpha2 << std::endl;
  //cout << "alphasum " << alpha+alpha1+alpha2 << std::endl;

  // HACK COMMENTED OUT BECAUSE IT WAS CAUSING A PROBLEM
  //  assert (fabs(alpha + alpha1 + alpha2 - M_PI) < 0.0001);

  double beta = asin( (valb - vala) / distab );  

  //cout << "alpha " << alpha << "  beta " << beta << std::endl;

  double my_c = vala + distac * sin (alpha + beta);
  double new_val = min3(valc,vala+distac,valb+distbc);
  //cout << my_c << " " << new_val << std::endl;
  
  if (alpha + beta <= M_PI / 4.0) {
    new_val = min2(my_c,new_val);
  }

  bool answer = false;
  
  if (valc > new_val+0.0001) answer=true;
  
  c->setBlendDistanceFromOcclusion(j,new_val); //min3(valc,vala+distac,val1+dist12));
  return answer;
}

bool adjustBlendDistanceFromOcclusion(Vertex *v0, Vertex *v1, Vertex *v2, int j) {
  double val0 = v0->getBlendDistanceFromOcclusion(j);
  double val1 = v1->getBlendDistanceFromOcclusion(j);
  double val2 = v2->getBlendDistanceFromOcclusion(j);

  if (val0 <= val1 && val1 <= val2)
    return adjustBlendDistanceFromOcclusion2(v0,v1,v2,j);
  if (val0 <= val2 && val2 <= val1)
    return adjustBlendDistanceFromOcclusion2(v0,v2,v1,j);

  if (val1 <= val0 && val0 <= val2)
    return adjustBlendDistanceFromOcclusion2(v1,v0,v2,j);
  if (val1 <= val2 && val2 <= val0)
    return adjustBlendDistanceFromOcclusion2(v1,v2,v0,j);

  if (val2 <= val0 && val0 <= val1)
    return adjustBlendDistanceFromOcclusion2(v2,v0,v1,j);
  if (val2 <= val1 && val1 <= val0)
    return adjustBlendDistanceFromOcclusion2(v2,v1,v0,j);
  exit(0);
}


void Mesh::ComputeBlendDistanceFromOcclusion() {

  (*ARGS->output) << "in compute blend distance" << std::endl;

  unsigned int num_projectors = ARGS->projector_names.size();
  if (num_projectors == 0) return;
  elementshashtype::iterator foo;

  // first set all blend distances to zero
  for (unsigned int i = 0; i < numVertices(); i++) {
    Vertex *v = getVertex(i);
    v-> clearBlendDistanceFromOcclusion();
    for (unsigned int j = 0; j < num_projectors; j++) {
      v->setBlendDistanceFromOcclusion(j,0);
    }
  }
  // then set all **projection** blend distances to one
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;
    if (e->isALine()) continue;
    if (!e->getMaterialPtr()->isProjection()) continue; 
    assert (e->isATriangle());
    int num_verts = 3;
    for (int k = 0; k < num_verts; k++) {
      Vertex *v = getVertex((*e)[k]);
      for (unsigned int j = 0; j < num_projectors; j++) {
	//v->setBlendDistanceFromOcclusion (j,100000.0);
	v->setBlendDistanceFromOcclusion (j,100000.0);
      }
    }
  }

  Vec3f mind,maxd;
  bbox->Get(mind,maxd);
  double height = maxd.y()-mind.y();
  (*ARGS->output) << height << std::endl;

  // finally, zero out the blend weights that set all projection vertex blend weights to one
  for (foo = elements.begin();foo != elements.end();foo++) {
    Element *e = foo->second;
    if (e->isALine()) continue;
    if (!e->getMaterialPtr()->isProjection()) continue; 
    const std::string &sidedness = e->getSidedness();
    assert (e->isATriangle());
    int num_verts = 3;
    for (int k = 0; k < num_verts; k++) {
      Vertex *v = getVertex((*e)[k]);
      for (unsigned int j = 0; j < num_projectors; j++) {
	if (sidedness[j] == 'b') {
	  //v->setBlendDistanceFromOcclusion(j,0);
	  v->setBlendDistanceFromOcclusion(j,height/1000.0);
	}
      }
    }
  }

#if 1
  // loop repeatedly over all triangles
  int change;
  int foo2 = 0;
  do { 
    foo2++;
    if (foo2 > 20) break;
    //if (foo2 > 2) break;
    change = 0;
    for (foo = elements.begin();foo != elements.end();foo++) {
      Element *e = foo->second;
      if (e->isALine()) continue;
      Vertex *v0 = getVertex((*e)[0]);
      Vertex *v1 = getVertex((*e)[1]);
      Vertex *v2 = getVertex((*e)[2]);
      for (unsigned int j = 0; j < num_projectors; j++) {
	if (adjustBlendDistanceFromOcclusion(v0,v1,v2,j))
	  change++;
      }
    }
  } while (change > 0);
#endif

}

#endif


// -----------------------------------------------------------------
// -----------------------------------------------------------------

int SameFace(int x1, int x2, int x3, int x4, int y1, int y2, int y3, int y4) {
  if ((x1==y1 && x2==y2 && x3==y3 && x4 == y4) ||
      (x1==y2 && x2==y3 && x3==y4 && x4 == y1) ||
      (x1==y3 && x2==y4 && x3==y1 && x4 == y2) ||
      (x1==y4 && x2==y1 && x3==y2 && x4 == y3))
    return 1;
  return 0;
}

bool Element::NearZeroArea() const { 
  Mesh *m = getMesh();
  if (this->isALine()) { 
    return false;
  } else if (this->isATriangle()) {
    return Triangle::NearZeroArea2(m->getVertex((*this)[0])->get(),
				  m->getVertex((*this)[1])->get(),
				  m->getVertex((*this)[2])->get(),
				  m); 
  } else {
    assert (this->isAQuad());
    return Quad::NearZeroArea2(m->getVertex((*this)[0])->get(),
			      m->getVertex((*this)[1])->get(),
			      m->getVertex((*this)[2])->get(),
			      m->getVertex((*this)[3])->get(),
			      m); 
  }
}


bool Element::NearZeroAngle() const { 
  Mesh *m = getMesh();
  if (this->isALine()) { 
    return false;
  } else if (this->isATriangle()) {
    return Triangle::NearZeroAngle2(m->getVertex((*this)[0])->get(),
				  m->getVertex((*this)[1])->get(),
				  m->getVertex((*this)[2])->get(),
				  m); 
  } else {
    assert (this->isAQuad());
    return Quad::NearZeroAngle2(m->getVertex((*this)[0])->get(),
			      m->getVertex((*this)[1])->get(),
			      m->getVertex((*this)[2])->get(),
			      m->getVertex((*this)[3])->get(),
			      m); 
  }
}


bool Triangle::NearZeroArea2(const Vec3f &a, const Vec3f &b, const Vec3f &c, Mesh *m) {
  double zero_area_tolerance = m->ZeroAreaTolerance();
  return (::AreaOfTriangle(a,b,c) < zero_area_tolerance);
}

bool Triangle::NearZeroArea2(int a, int b, int c, Mesh *m) {
  return NearZeroArea2(m->getVertex(a)->get(),
		       m->getVertex(b)->get(),
		       m->getVertex(c)->get(),m); 
}


bool Quad::NearZeroArea2(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d, Mesh *m) {
  double zero_area_tolerance = m->ZeroAreaTolerance();
  return (Area(a,b,c,d) < zero_area_tolerance);
}

//const double near_zero_angle = 1.0 * M_PI / 180.0;

bool Triangle::NearZeroAngle2(const Vec3f &a, const Vec3f &b, const Vec3f &c, Mesh *m) {
  return false;
  /*
  // don't know if this works....
  double minangle = MinAngle(a,b,c);
  if (minangle < near_zero_angle) {
    cout << "IS NEAR ZERO ANGLE" << std::endl;
    return true;
  }
  return false;
  */
}

bool Quad::NearZeroAngle2(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d, Mesh *m) {
  return false; 
  //double zero_area_tolerance = m->ZeroAreaTolerance();
  //return (Area(a,b,c,d) < zero_area_tolerance);
}


bool Element::BadNormal() const {
  if (this->NearZeroArea()) return false;//true;
  Vec3f normal,normal2;
  computeNormal(normal);
  for (int i = 0; i < numVertices(); i++) {
    std::vector<Element*> neighbors = getNeighbors(i);
    for (unsigned int j = 0; j < neighbors.size(); j++) {
      Element *e = neighbors[j];
      //assert (e->isATriangle());
      //Triangle *t = (Triangle*)e;
      if (e->NearZeroArea()) continue; //return false;//true;
      e->computeNormal(normal2);
      double dot = normal.Dot3(normal2);
      if (dot < -0.99) {
	// if it's a non-manifold edge, then it's ok!
	int v = (*this)[i];
	int w = (*this)[(i+1)%3];
	for (int k = 0; k < 3; k++) {
	  int v2 = (*e)[k];	 
	  int w2 = (*e)[(k+1)%3];	 
	  //(*ARGS->output) << "CHECK " << v << " " << w << " compare  " << v2 << std::endl;
	  if (v == v2 && w == w2) {
	    //(*ARGS->output) << "WINNER " << k << std::endl;
	    continue;
	  }
	  if (v == w2 && w == v2) {
	    //(*ARGS->output) << "LOSER  " << k << std::endl;
	    return true;
	  }
	}
	//	return true;
      }
    }
  }
  return false;
}

bool Element::HasBadNeighbor() const {
  if (this->IsBad()) return true;
  for (int i = 0; i < 3; i++) {
    std::vector<Element*> neighbors = getNeighbors(i);
    for (unsigned int i = 0; i < neighbors.size(); i++) {
      Element *e = neighbors[i];
      //      assert (e->isATriangle());
      //Triangle *t = (Triangle*)e;
      if (e->IsBad()) return true;
    }
  }
  return false;
}



void Mesh::RecomputeStats() {
  ComputeNormals();

  // find longest & shortest edges
  longest_edge = -1;
  shortest_edge = -1;
  for (edgeshashtype::const_iterator foo = edges.begin(); 
       foo != edges.end();
       foo++) {
    assert (foo->second.size() > 0);
    for (std::vector<Edge*>::const_iterator bar = foo->second.begin();
	 bar < foo->second.end();
	 bar++) {
      Edge *e = *bar;
      double l = e->getLength();
      if (longest_edge < 0 || l > longest_edge) longest_edge = l;
      if (shortest_edge < 0 || l < shortest_edge) shortest_edge = l;
    }
  }

  // find total surface area
  area = 0;
  weighted_area = 0;
  for (elementshashtype::const_iterator foo = elements.begin();
       foo != elements.end();
       foo++) {
    Element *e = foo->second;
    area += e->Area();
    if (e->getMaterialPtr()->isProjection())
      weighted_area += e->Area();
    else
      weighted_area += e->Area()/double(EXTRA_LENGTH_MULTIPLIER*EXTRA_LENGTH_MULTIPLIER);
  }
  // calculate tolerances
  if (numVertices() == 0) 
    zero_area_tolerance = 0.000001;
  else
    //    zero_area_tolerance = 0.0001 * area / double(numVertices());
    zero_area_tolerance = 0.001 * area / double(numVertices());

  // compute scene bounding box
  if (bbox != NULL) {
    assert (numVertices() > 0);
    Vec3f v = getVertex(0)->get();
    bbox->Set(v,v);
    for (int i = 0; i < (int)numVertices(); i++) {
      v = getVertex(i)->get();
      bbox->Extend(v);
    }
  }

  modified = false;
}

// -----------------------------------------------------------------


std::set<std::string> Mesh::getSimpleMaterialNames() const {
  std::set<std::string> answer;
  const std::map<std::string, MeshMaterial>& mats = getMaterials(); 
  for (std::map<std::string,MeshMaterial>::const_iterator itr = mats.begin();
       itr != mats.end(); itr++) {
    std::string name = itr->second.getSimpleName();
    if (name.substr(0,6) == "EXTRA_") continue;
    answer.insert(name);
  }
  return answer;
}
