#include "remesh.h"
#include "mesh.h"
#include "meshmanager.h"
#include "triangle.h"

// FLIP EDGE

int ReMesh::FlipEdges(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  assert (mesh != NULL);
  assert (mesh != NULL);

  // put all the element ids in an vector (since we're going to be changing things)
  int answer = 0;
  std::vector<ElementID> ids;
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    ids.push_back(foo->second->getID());
  }

  // go through the ids
  for (unsigned int i = 0; i < ids.size(); i++) {
    // skip ids for triangles that have already been deleted
    Element *e = Element::GetElement(ids[i]);
    if (e == NULL) continue;

    if (!e->isATriangle()) continue;

    Triangle *t = (Triangle*)e;
    assert (t == Element::GetElement(t->getID()));

    // consider each edge of the triangle
    for (unsigned int i = 0; i < 3; i++) {
      assert (t == Element::GetElement(t->getID()));
      if (mesh->IsManifoldInteriorEdge(t->get_my_edge(i))) { //,(*t)[i],(*t)[(i+1)%3])) {
	std::vector<Element*> neighbors = t->getNeighbors(i);
	// only consider edges with exactly one neighbor
	assert (neighbors.size() == 1);
	Triangle *t2 = (Triangle*)neighbors[0];
	assert (t2 == Element::GetElement(t2->getID()));
	if (t2 != NULL && Flipped(i,t,t2,meshes->args)) { 
	  answer++;
	  break;
	}
      }
    }
  }
  mesh->RecomputeStats();
  return answer;
}

bool minAngleCheck(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d) {
  double min_angle_abc = MinAngle(a,b,c);  // t1 before
  double min_angle_abd = MinAngle(a,b,d);  // t2 before
  double min_angle_cad = MinAngle(c,a,d);  // after
  double min_angle_cdb = MinAngle(c,d,b);  // after

  double min_before = min2(min_angle_abc,min_angle_abd);
  double min_after  = min2(min_angle_cad,min_angle_cdb);

  if (min_after < min_before &&
      min_after < 2 * M_PI / 180) {
    return false;
  }

  return true;
}

int ReMesh::Flipped(int i, Triangle *t, Triangle *t2, ArgParser *args) { 

  // materials of the two triangles must be the same
  std::string mat = t->getFakeMaterial();
  std::string mat2 = t2->getFakeMaterial();
  if (mat != mat2) return 0;

  // gather the 4 vertices involved
  int a = (*t)[i];
  int b = (*t)[(i+1)%3];
  assert (t2->HasVertex(a) && t2->HasVertex(b));
  int c = (*t)[(i+2)%3];
  int j = t2->WhichVertex(b);
  int d = (*t2)[(j+2)%3];
  if (c == d) return 0; //assert (c != d);

  
  // make sure the neighborhood isn't too connected 
  std::vector<Element*> na = t->getNeighbors((i+1)%3);
  std::vector<Element*> nb = t->getNeighbors((i+2)%3);
  std::vector<Element*> n2a = t2->getNeighbors((j+1)%3);
  std::vector<Element*> n2b = t2->getNeighbors((j+2)%3);
  assert (!CommonElement(na,nb));
  assert (!CommonElement(n2a,n2b));
  if (CommonElement(na,n2a) ||
      CommonElement(na,n2b) ||
      CommonElement(nb,n2a) ||
      CommonElement(nb,n2b)) {
    return 0;
  }

  // if one of the new triangles has near zero area, abort
  Mesh *m = t->getMesh();
  if ((Triangle::NearZeroArea2(c,a,d,m) || Triangle::NearZeroArea2(c,d,b,m))
      && !args->equal_edge_and_area) {
    return 0;
  }
  
  if (!minAngleCheck(m->getVertex(a)->get(),
		     m->getVertex(a)->get(),
		     m->getVertex(a)->get(),
		     m->getVertex(a)->get())
      && !args->equal_edge_and_area) {
    return 0;
  }
  
  // if the new edges are already in the model, abort
  if (m->GetEdges(c,d).size() != 0 || m->GetEdges(d,c).size() != 0) return 0;

  // if the new edge isn't shorter, abort
  double d0 = DistanceBetweenTwoPoints(m->getVertex(a)->get(),m->getVertex(b)->get());
  double d1 = DistanceBetweenTwoPoints(m->getVertex(c)->get(),m->getVertex(d)->get());
  if (d0 <= 1.01*d1) return 0;

  // do the normals checking (for triangles with non zero area)
  // if the surface is not flat enough, abort
  Vec3f normal0, normal1, normal2, normal3;
  if (!t->NearZeroArea()) { t->computeNormal(normal0); }
  if (!t2->NearZeroArea()) { t2->computeNormal(normal1); }
  Element::computeNormal(m,c,a,d,normal2);
  Element::computeNormal(m,c,d,b,normal3);
  if (!t->NearZeroArea() && !t2->NearZeroArea()) {
    if (normal0.Dot3(normal1) < args->remesh_normal_tolerance && !args->equal_edge_and_area) return 0;
  }
  if (!t->NearZeroArea()) { 
    if (normal0.Dot3(normal2) < args->remesh_normal_tolerance && !args->equal_edge_and_area) return 0;
    if (normal0.Dot3(normal3) < args->remesh_normal_tolerance && !args->equal_edge_and_area) return 0;
  }
  if (!t2->NearZeroArea()) { 
    if (normal1.Dot3(normal2) < args->remesh_normal_tolerance && !args->equal_edge_and_area) return 0;
    if (normal1.Dot3(normal3) < args->remesh_normal_tolerance && !args->equal_edge_and_area) return 0;
  }
  if (normal2.Dot3(normal3) < args->remesh_normal_tolerance && !args->equal_edge_and_area) return 0;

  // do the flip
  //cout << "FLIP START" << endl;
  m->removeElement(t);
  m->removeElement(t2);
  m->addElement(new Triangle(c,a,d,m,mat));
  m->addElement(new Triangle(c,d,b,m,mat));
  //cout << "FLIP END" << endl;
  return 1;
}
