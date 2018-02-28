#include "remesh.h"
#include "quad.h"
#include "mesh.h"
#include "meshmanager.h"
#include "collect.h"
#include "triangle.h"
#include "edge.h"
#define QUIET
// =============================================================================
// =============================================================================
// COLLAPSE EDGE

int ReMesh::CollapseEdges(MeshManager *meshes, double length_threshold) {
  Mesh *mesh = meshes->getMesh();
  mesh->clearParents();
  assert (mesh != NULL);
  int answer = 0;
  // make a vector of all the element ids
  std::vector<ElementID> ids;
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    ids.push_back(foo->second->getID());
  }
  // go through the ids looking for edges to collapse
  int count = ids.size();
  for (int i = 0; i < count; i++) {
    Element *e = Element::GetElement(ids[i]);
    if (e == NULL) continue;
    if (!e->isATriangle()) continue;
    Triangle *t = (Triangle*)e;
    assert (t == Element::GetElement(t->getID()));
    for (int i = 0; i < 3; i++) {
      //int a = (*t)[i];
      //int b = (*t)[(i+1)%3];
      Edge *ed = t->get_my_edge(i);
      // try to collapse each edge
      if (TryCollapse(meshes->args,t,ed,false,length_threshold) || 
          TryCollapse(meshes->args,t,ed,true,length_threshold)) { 
	answer++;
	break;
      }
    }
  }
  mesh->RecomputeStats();
  return answer;
}

// =====================================================================================================

bool ReMesh::TryCollapse(ArgParser *args, Element *e, Edge *ed, bool collapse_opposite, /*int del_vert, int repl_vert, */double length_threshold, bool bad_triangle_flag) {
  int del_vert  = (*ed)[0]; //za(*t)[i];
  int repl_vert = (*ed)[1]; //(*t)[(i+1)%3]
  assert (del_vert != -1 && repl_vert != -1);
  if (collapse_opposite) { int tmp = repl_vert; repl_vert = del_vert; del_vert = tmp; }

  //  std::cout << "\n" << "collapse " << del_vert << " to " << repl_vert << std::endl;

  if (!e->isATriangle()) return false;
  Triangle *t = (Triangle*)e;
  assert (t != NULL);
  Mesh *m = t->getMesh();

  // CHECK THE MINIMUM LENGTH
  double min_edge_length;
  if (length_threshold < 0) {
    min_edge_length = GLOBAL_DESIRED_LENGTH/2.0; 
  } else {
    min_edge_length = length_threshold;
  }
  if (t->getMaterialPtr()->isExtra() ||
      t->getMaterialPtr()->isExterior()) {
    min_edge_length *= EXTRA_LENGTH_MULTIPLIER;
  }
  if (e->getMaterialPtr()->isSSS()) {
    min_edge_length /= SSS_EXTRA_LENGTH_MULTIPLIER;
  }
  
  double d = DistanceBetweenTwoPoints(m->getVertex(del_vert)->get(),m->getVertex(repl_vert)->get());
  if (d > min_edge_length) return false;
  
  // COLLECT ALL TRIANGLES AFFECTED BY THE COLLAPSE
  std::vector<Element*> element_vec;
  Collect::CollectElementsWithVertex(t,del_vert,element_vec);
  assert(Collect::VectorContains(element_vec,t) && element_vec.size() > 0);
  

  // INTERIOR COLLAPSE (locally manifold, no boundaries or non-manifold edges)
  if (m->IsManifoldInteriorEdge(ed) &&
      m->IsManifoldInteriorVertex(e,del_vert,&element_vec)) {
#ifndef QUIET
    std::cout << "interior collapse" << std::endl;
#endif
  }

  // BOUNDARY
  else if (m->IsSimpleBoundaryEdge(ed) &&
	   m->IsSimpleBoundaryVertex(e,del_vert,&element_vec)) {
#ifndef QUIET
    std::cout << "simple boundary collapse" << std::endl;	
#endif
  

    int other_edge_vert; // = -1;
    bool bound = IsBoundaryCollapse(m,element_vec,del_vert,repl_vert,other_edge_vert);
    if (!bound) {
      std::cout << "COLLAPSE LOOKS LIKE BOUNDARY BUT ISN'T" << std::endl;
      return false;
    }
      assert (bound); 

    /*
    std::cout << "del vert " << del_vert << std::endl;
    std::cout << "repl vert " << repl_vert << std::endl;
    std::cout << "other_edge_vert " << other_edge_vert << std::endl;
    */

    if (!IsOkBoundaryCollapse(m,element_vec,del_vert,repl_vert,other_edge_vert,args)) {
      return false;
    }
  }

  // NON MANIFOLD
  else {
#ifndef QUIET
    std::cout << "non manifold collapse" << std::endl;
#endif
  }


  std::vector<Element*> pile1;
  std::vector<Element*> pile2;
  int other_edge_vert = -1; 
  bool sort_success = SortIntoTwoMaterialPiles(element_vec,del_vert,pile1,pile2,repl_vert,other_edge_vert,args,bad_triangle_flag);
  // fails if there are >2 materials surrounding this vertex or if they alternate
  if (sort_success && pile2.empty() && !args->equal_edge_and_area) {
    sort_success = SortIntoTwoNormalPiles(element_vec,del_vert,pile1,pile2,repl_vert,other_edge_vert,args,bad_triangle_flag);
  }
  if (!sort_success && !args->equal_edge_and_area) {
    if (bad_triangle_flag) {
      //cout << " ignore normal problem" << endl;
    } else {
      return false;
    }
  }
  assert (del_vert != -1 && repl_vert != -1);
  if (!SimilarTriangleRegion(m,pile1,del_vert,repl_vert,args) && !args->equal_edge_and_area) {
    if (bad_triangle_flag) {
      // cout << " bad region, non boundary " << endl;
    } else {
      return false;
    }
  }
  if (!pile2.empty() && !args->equal_edge_and_area) {
    //if (repl_vert != edge_vert1 && repl_vert != edge_vert2) {
    //cout << " repl vert not a edge vert (material/normal)... " << endl;
    //if (bad_triangle_flag) cout << "NOT SUCCESS3" << endl;
    return false;
    //}
    if (!SimilarTrajectory(m->getVertex(repl_vert)->get(),
			   m->getVertex(del_vert)->get(),
			   m->getVertex(other_edge_vert)->get())) {
      //cout << " problem with trajectory material/normal" << endl;
      //if (bad_triangle_flag) cout << "NOT SUCCESS4" << endl;
      return false;
    }
    assert (del_vert != -1 && repl_vert != -1);
    if (!SimilarTriangleRegion(m,pile2,del_vert,repl_vert,args)) {
      //cout << " bad region, non boundary " << endl;
      //if (bad_triangle_flag) cout << "NOT SUCCESS5" << endl;
      return false;
    }
  }
  
  int ok_vertex1 = -1;
  int ok_vertex2 = -1;
  
  int abort = 0;
  for (unsigned int ev = 0; ev < element_vec.size(); ev++) { 
    Element *e = element_vec[ev]; 
    assert(e->HasVertex(del_vert));
    if (e->HasVertex(repl_vert)) {
      continue;
    }
    int num_verts = e->numVertices();
    for (int i = 0; i < num_verts; i++) {
      int v = (*e)[i];
      if (v == del_vert) continue;
      assert (v != repl_vert);
      if (m->GetEdges(v,repl_vert).size() != 0 ||
	  m->GetEdges(repl_vert,v).size() != 0) {
	if (ok_vertex1 == -1 || ok_vertex1 == v) {
	  ok_vertex1 = v;
	} else if (ok_vertex2 == -1 || ok_vertex2 == v) {
	  ok_vertex2 = v;
	} else {
	  abort = 1;
	  break;
	}
      }
    }
  }
  if (abort == 1) { 
    return false; 
  }
    
  DoCollapse(t->getMesh(),element_vec,del_vert,repl_vert);
  return true;
}


bool IsBoundaryCollapse(Mesh *m, std::vector<Element*> &element_vec, const int del_vert, const int repl_vert, 
			int &other_edge_vert) { 
  other_edge_vert = -1;
  for (unsigned int ev = 0; ev < element_vec.size(); ev++) {
    Element *e = element_vec[ev]; 
    assert(e->HasVertex(del_vert));
    // look for edges with no neighbor (boundary) or edges that are non manifold (> 1 neighbor or inconsistent orientation)
    int num_verts = e->numVertices();
    for (int j = 0; j < num_verts; j++) {
      int v  = (*e)[j];
      int v2 = (*e)[(j+1)%num_verts];
      bool interior = m->IsManifoldInteriorEdge(e->get_my_edge(j)); //,v,v2);
      bool non_manifold = m->IsNonManifoldEdge(e->get_my_edge(j)); //) { //,v,v2)) {
      if (!interior || non_manifold) {
	if (v != del_vert && v2 != del_vert) continue;
	if (v2 == del_vert) { v2 = v; v = del_vert; }
	assert (v == del_vert && v2 != del_vert);
	if (v2 == repl_vert) continue;
	else if ( other_edge_vert == -1 ) {
	  other_edge_vert = v2;
	} else if (other_edge_vert != v2) {
	  return false;
	} 
      }
    }
  }
  if (other_edge_vert == -1) {
    return false;
  }
  return true;
}


bool IsOkBoundaryCollapse(Mesh *m, std::vector<Element*> &element_vec, int del_vert, int repl_vert, int &other_edge_vert, ArgParser *args) {
  assert (other_edge_vert != -1);
  assert (del_vert != other_edge_vert);
  assert (repl_vert != other_edge_vert);
  assert (del_vert != repl_vert);
  if (!SimilarTriangleRegion(m,element_vec,del_vert,repl_vert,args)) {
    //std::cout << " problem with similarness " << std::endl;
    return false;
  }
  if (!SimilarTrajectory(m->getVertex(repl_vert)->get(),
			 m->getVertex(del_vert)->get(),
			 m->getVertex(other_edge_vert)->get())) {
    //std::cout << " problem with trajectory boundary " << std::endl;
    return false;
  }
  //std::cout << " ok boundary collapse " << std::endl;
  return true;
}

void LargestTriangleNormal(std::vector<Element*> &element_vec, Vec3f &normal, std::string &material) {
  assert (!element_vec.empty());
  double area = -1;
  for (unsigned ev = 0; ev < element_vec.size(); ev++) {//Iterator<Element*> *iter = bag.StartIteration();
    Element *e = element_vec[ev]; //while (Element *e = iter->GetNext()) {
    //assert (e->isATriangle());
    //Triangle *t = (Triangle*)e;
    double a = e->Area();
    if (a > area) {
      area = a;
      e->computeNormal(normal);
      material = e->getFakeMaterial();
    }
  }
  //bag.EndIteration(iter);
}

bool NormalTolerance(Element *e, const Vec3f &normal, ArgParser *args) {
  if (e->NearZeroArea()) return true;
  Vec3f normal2; e->computeNormal(normal2);
  double dot = normal.Dot3(normal2);
  if (dot < args->remesh_normal_tolerance - 0.000001) {
    return false;
  }
  return true;
}

bool MostDifferentNormals(std::vector<Element*> &element_vec, Vec3f &normal1, Vec3f &normal2, ArgParser *args,bool bad_triangle_flag) {
  assert (!element_vec.empty()); 
  // find triangle with largest area
  double area = -1;
  for (unsigned ev = 0; ev < element_vec.size(); ev++) {
    Element *e = element_vec[ev]; 
    double a = e->Area();
    if (a > area) {
      area = a;
      e->computeNormal(normal1);
    }
  }

  double smalldot = 2;
  for (unsigned ev = 0; ev < element_vec.size(); ev++) { 
    Element *e = element_vec[ev]; 
    if (!bad_triangle_flag && NormalTolerance(e,normal1,args)) continue;
    Vec3f normal;
    e->computeNormal(normal);
    double dot = normal.Dot3(normal1);
    if (!bad_triangle_flag && dot < smalldot) {
      smalldot = dot;
      normal2 = normal;
    }
  }

  if (smalldot > 1) return false; // not 2 different normals
  assert (bad_triangle_flag == false);
  return true;  // yes, 2 different normals
}

bool SortIntoTwoMaterialPiles(std::vector<Element*> &element_vec, const int middle_vert, 
                              std::vector<Element*> &pile1, std::vector<Element*> &pile2, 
                              int &left_edge_vert, 
			      int &right_edge_vert, 
			      ArgParser *args, bool bad_triangle_flag) {
  //  bad_triangle_flag = false;
  if (bad_triangle_flag) {
    std::cout << "SORTING MATERIALS INTO TWO PILES W/ BAD TRIANGLE" << std::endl;
  }

  bool answer=true;
  std::string material, material2;

  
  for (unsigned int ev = 0; ev < element_vec.size(); ev++) {
    Element *e = element_vec[ev]; 
    int which = e->WhichVertex(middle_vert);
    std::vector<Element*> neighbors = e->getNeighbors(which);
    if (neighbors.size() == 0) return false;
    assert (neighbors.size() >= 1);
    
    for (unsigned int j = 0; j < neighbors.size(); j++) {
      //if (neighbors.size() != 1) return false;
      Element *e2 = neighbors[j];
      assert (e2 != NULL); 
      
      std::string emat  = (bad_triangle_flag) ? e->getRealMaterialName()  : e->getFakeMaterial();
      std::string e2mat = (bad_triangle_flag) ? e2->getRealMaterialName() : e2->getFakeMaterial();
      
      if (material == "") material = emat; 
      if (material == emat) { 
	if (j == 0) pile1.push_back(e); 
	if (e2mat != emat) {
	  if (left_edge_vert != -1) answer = false;
	  left_edge_vert = (*e)[(which+1)%3];
	}
      } else {
	if (material2 == "") material2 = e2mat; //e->getFakeMaterial();
	//if (material2 != e->getFakeMaterial()) answer = false;
	if (material2 != emat) answer = false;
	if (j == 0) pile2.push_back(e); 
	//if (e2->getFakeMaterial() != e->getFakeMaterial()) {
	if (e2mat != emat) {
	  if (right_edge_vert != -1) answer = false;
	  right_edge_vert = (*e)[(which+1)%3];
	}
      }

    }
  }

  assert (!pile1.empty()); 
  if (!pile2.empty()) { 
    assert (left_edge_vert != -1 && right_edge_vert != -1);
  } else {
    //assert (left_edge_vert == -1 && right_edge_vert == -1);
    assert (right_edge_vert == -1);
  }
  if (!answer) {
    assert (left_edge_vert != -1 && right_edge_vert != -1);
  }
  return answer;
}


bool SortIntoTwoNormalPiles(std::vector<Element*> &element_vec, const int middle_vert, std::vector<Element*> &pile1, std::vector<Element*> &pile2, 
			    int &left_edge_vert, 
			    int &right_edge_vert, 
			    ArgParser *args,bool bad_triangle_flag) {

  //left_edge_vert = right_edge_vert = -1;

  assert (pile2.empty());
  bool answer=true;
  Vec3f normal1,normal2;
  bool different = MostDifferentNormals(element_vec,normal1,normal2,args,bad_triangle_flag);
  if (!different) {
    assert (pile1.size() == element_vec.size());
    return true;
  }
  pile1.clear();
  
  int zero_area_tris = 0;
  for (unsigned int ev = 0; ev < element_vec.size(); ev++) {
    Element *e = element_vec[ev]; 
    Vec3f normal_A; 
    if (e->NearZeroArea()) { 
      zero_area_tris++; 
      // use normal of left neighbor
      int which = e->WhichVertex(middle_vert);
      int num_verts = e->numVertices();
      std::vector<Element*> neighbors = e->getNeighbors((which+num_verts-1)%num_verts);
      assert (neighbors.size() >= 1);
      //for (unsigned int j = 0; j < neighbors.size(); j++) {
      //assert (neighbors.size() == 1); 
      Element *e2 = neighbors[0];
      assert (e2 != NULL);
      if (e2->NearZeroArea()) {
	answer = false;
	break;
      }
      e2->computeNormal(normal_A);
    } else {
      // use my normal
      e->computeNormal(normal_A);
    }
    // find right neighbor
    int which = e->WhichVertex(middle_vert);
    std::vector<Element*> neighbors = e->getNeighbors(which);
    assert (neighbors.size() >= 1);
    
    for (unsigned int j = 0; j < neighbors.size(); j++) {
      Element *e2 = neighbors[j];
      Vec3f normal_B; e2->computeNormal(normal_B);
      double dot_A1 = normal_A.Dot3(normal1);
      double dot_A2 = normal_A.Dot3(normal2);
      double dot_B1 = normal_B.Dot3(normal1);
      double dot_B2 = normal_B.Dot3(normal2);
      if (dot_A1 < dot_A2) {
	if (j == 0) pile1.push_back(e);
	if (dot_B1 >= dot_B2) {
	  if (left_edge_vert != -1) answer = false;
	  left_edge_vert = (*e)[(which+1)%3];
	}
      } else {
	if (j == 0) pile2.push_back(e);      
	if (dot_B1 < dot_B2) {
	  if (right_edge_vert != -1) answer = false;
	  right_edge_vert = (*e)[(which+1)%3];
      }
      }
    }
  }
  if (zero_area_tris > 1) {
    //cout << "too many zero area tris at vertex to collapse" << endl;
    answer = false;
  }
  if (answer == false) return answer;
  assert (!pile1.empty());
  assert (!pile2.empty());
  assert (left_edge_vert != -1 && right_edge_vert != -1);

  if (answer) {
    assert (left_edge_vert != -1 && right_edge_vert != -1);
  }

  return answer;
}


bool SimilarTriangleRegion(Mesh *m, std::vector<Element*> &element_vec, int del_vert, int repl_vert, ArgParser *args) {

  assert (del_vert != -1 && repl_vert != -1);
  Vec3f normal;
  std::string material;
  LargestTriangleNormal(element_vec,normal,material);  
  bool answer = true;

  int zero_area_before = 0;
  int zero_area_after = 0;

  for  (unsigned int ev = 0; ev < element_vec.size(); ev++) {
    Element *e = element_vec[ev]; 
    if (material != e->getFakeMaterial()) {
      //std::cout << " materials don't match " << std::endl;
      answer = false;
    }
    if (!NormalTolerance(e,normal,args)) {
      //std::cout << " normal before not similar enough " << std::endl;
      answer = false;
    }
    if (e->NearZeroArea()) zero_area_before++;
    assert (e->HasVertex(del_vert));
    if (!e->HasVertex(repl_vert)) {
      // check normal after collapse
      int a = (*e)[0]; if (a == del_vert) a = repl_vert;
      int b = (*e)[1]; if (b == del_vert) b = repl_vert;
      int c = (*e)[2]; if (c == del_vert) c = repl_vert;
      Vec3f normal3; Element::computeNormal(m,a,b,c,normal3);

      double d = normal.Dot3(normal3);
      bool zero_after = Triangle::NearZeroArea2(a,b,c,m);
      if (zero_after) {
	zero_area_after++;
	//std::cout << " introduced zero area " << std::endl;
	// HACK this was commented out before???
	answer = false;
      }
      if (!zero_after && d < args->remesh_normal_tolerance) { 
	//	std::cout << " normal after not similar enough " << std::endl;
	answer = false;
      }
      //double longest = Element::LongestEdge(m,a,b,c);
      //if (longest > 2 * min_edge_length) { 
	//cout << " edge will be too long " << endl;
	//answer = false;
      //}
    }
  }

  if (zero_area_after > zero_area_before) {
    //    std::cout << " introduced zero area " << std::endl;
    answer = false;
  }

  return answer;
}


bool SimilarTrajectory(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  Vec3f tmp1 = b-a;
  Vec3f tmp2 = c-b;
  double length1 = tmp1.Length();
  double length2 = tmp2.Length();
  // if one of the edges is very very short, ignore trajectory
  if (length1 > 1000*length2 || length2 > 1000*length1) {
    //    cout << " differing lengths " << endl;
    return true;
  }
  // otherwise they should be approximately parallel
  tmp1.Normalize();
  tmp2.Normalize();  
  double dot = tmp1.Dot3(tmp2);
  if (dot < 0.99) { 
    //cout << " trajectory is creased " << endl;
    return false;
  }
  //cout << " ok trajectory " << endl;
  return true;
}


void DoCollapse(Mesh *m, std::vector<Element*> &element_vec, int del_vert, int repl_vert) {

  std::vector<Element*> new_elements;
  // create the new elements, but don't add them yet...
  for (unsigned int i = 0; i < element_vec.size(); i++) {
    Element *e = element_vec[i]; 
    std::string mat = e->getFakeMaterial();
    if (e->HasVertex(del_vert) && e->HasVertex(repl_vert)) {
      if (!e->isATriangle()) {
	assert (e->isAQuad());
	int a = e->WhichVertex(del_vert);
	int b = e->WhichVertex(repl_vert);
	if (a == (b+1)%4) {
	  new_elements.push_back(new Triangle((*e)[b],(*e)[(b+2)%4],(*e)[(b+3)%4],m,mat));
	} else if (b == (a+1)%4) {
	  new_elements.push_back(new Triangle((*e)[b],(*e)[(b+1)%4],(*e)[(b+2)%4],m,mat));
	} else {
	  std::cout << "failure in collapse " << del_vert << " " << repl_vert << std::endl;
	  e->Print();
	  exit(0);
	}
      }
    } else {
      assert(e->HasVertex(del_vert) && !e->HasVertex(repl_vert));
      int a = (*e)[0]; if (a == del_vert) a = repl_vert;
      int b = (*e)[1]; if (b == del_vert) b = repl_vert;
      int c = (*e)[2]; if (c == del_vert) c = repl_vert;
      if (e->isAQuad()) {
	int d = (*e)[3]; if (d == del_vert) d = repl_vert;
	new_elements.push_back(new Quad(a,b,c,d,m,mat));
      } else {
	new_elements.push_back(new Triangle(a,b,c,m,mat));
      }
    }
    // remove each element as you go
    m->removeElement(e);     
  }

  // now add all of the new elements
  for (unsigned int i = 0; i < new_elements.size(); i++) {
    m->addElement(new_elements[i]);
  }
}






