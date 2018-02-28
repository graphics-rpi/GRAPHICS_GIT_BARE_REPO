#include "remesh.h"
#include "mesh.h"
#include "meshmanager.h"
#include "triangle.h"
#include "quad.h"
#include "edge.h"

// =============================================================================
// =============================================================================
// SPLIT EDGES

int ReMesh::SplitEdges(MeshManager *meshes, double length_threshold) {
  Evaluate(meshes);
  Mesh *mesh = meshes->getMesh();
  assert (mesh != NULL);

  int answer = 0;
  std::vector<ElementID> ids;
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    ids.push_back(foo->second->getID());
  }
  
  int count = ids.size();
  for (int i = 0; i < count; i++) {
    Element *e = Element::GetElement(ids[i]);
    if (e == NULL) continue;
    answer += SplitLongestEdge(e,length_threshold); 
  }

  mesh->RecomputeStats();
  return answer;
}


int ReMesh::SplitLongestEdge(Element *e,double length_threshold) {

  // find the longest edge of this element
  int i;
  int a,b;
  double longest;
  e->LongestEdge(i,a,b,longest);

  // if it's not longer than the max edge length, do nothing
  Mesh *mesh = e->getMesh();
  double max_edge_length;
  if (length_threshold < 0)
    max_edge_length = GLOBAL_DESIRED_LENGTH * 2.0;
  else
    max_edge_length = length_threshold;
  if (e->getMaterialPtr()->isExtra() ||
      e->getMaterialPtr()->isExterior()) {
    max_edge_length *= EXTRA_LENGTH_MULTIPLIER;
  }
  if (e->getMaterialPtr()->isSSS()) {
    max_edge_length /= SSS_EXTRA_LENGTH_MULTIPLIER;
  }

  if (longest <= max_edge_length) return 0;

  // if the edge has already been split...
  if (mesh->getChildVertex(a,b) != -1) {
    if (e->isALine()) {
      //continue;
    } else if (!e->isAQuad()) {
      std::cout << "uh oh, why isn't it a quad" << std::endl;      
      return 0;
    } else {
      assert (e->isAQuad());
      assert (a == (*e)[i]);
      assert (b == (*e)[(i+1)%4]);
      int tmp_a = (*e)[(i+2)%4];
      int tmp_b = (*e)[(i+3)%4];
      if (mesh->getChildVertex(tmp_a,tmp_b) == -1) {
	a = tmp_a;
	b = tmp_b;
	i = (i+2)%4;
      } else {
	return 0;
      }
    }
  }

  // look at all the neighbors of that edge, if any of them has a
  // longer edge, must split that edge first
  std::vector<Element*> vec = e->getNeighbors(i);
  for (unsigned int k = 0; k < vec.size(); k++) {
    Element *e2 = vec[k];
    assert (e2 != NULL);
    int _i;
    int _a,_b;
    double _longest;
    e2->LongestEdge(_i,_a,_b,_longest);
    if (_longest > longest) return 1;
  }

  std::set<ElementID> removed,added;
  return SplitEdge(e,i,a,b,0.5,removed,added);
}

int ReMesh::SplitEdge(Mesh *m, int a, int b, double fraction, std::set<ElementID> &removed, std::set<ElementID> &added) {

  const std::vector<Edge*>& edges = m->GetEdges(a,b);
  if (edges.size() == 0) return 0;
  Edge *edge = edges[0];

  Element *e = edge->getElement();
  assert (e != NULL);
  assert (Element::GetElement(e->getID()) == e);
  int which_a = e->WhichVertex(a);
  int which_b = e->WhichVertex(b);

  assert ((which_a+1)%e->numVertices() == which_b);
  return SplitEdge(e,which_a,a,b,fraction,removed,added);

}


int ReMesh::TriSect(Mesh *mesh, Triangle *tri, Vec3f pt, std::set<ElementID> &removed, std::set<ElementID> &added) {

  assert (tri != NULL);
  assert (Element::GetElement(tri->getID()) == tri);

  assert (tri->PointInside(pt));
  
  int va = (*tri)[0];
  int vb = (*tri)[1];
  int vc = (*tri)[2];

  // make a new vertex in the middle of the edge
  Vertex *verta = mesh->getVertex(va);
  Vertex *vertb = mesh->getVertex(vb);
  Vertex *vertc = mesh->getVertex(vc);

  double alpha, beta, gamma;
  bool test = BarycentricCoordinates(verta->get(),vertb->get(),vertc->get(),pt,alpha,beta,gamma);
  if (!test) return 0;

  if (alpha < 0.01 || beta < 0.01 || gamma < 0.01) return 0;

      //  std::cout << "BARY " << alpha << " " << beta << " " << gamma << std::endl;


      //> 0.99 || beta > 0.99 || gamma > 0.99) < 0.99 && beta < 0.99 && gamma < 0.99)

  double sa,ta,sb,tb,sc,tc;
  verta->getTextureCoordinates(sa,ta);
  vertb->getTextureCoordinates(sb,tb);
  vertc->getTextureCoordinates(sc,tc);

  double s = alpha*sa + beta*sb + gamma*sc;
  double t = alpha*ta + beta*tb + gamma*tc;

  int new_v = mesh->addVertex(pt,-1,-1,s,t);

 std::string mat = tri->getFakeMaterial();
 //  const Material* mat = tri->getMaterialPtr();

  removed.insert(tri->getID());
  mesh->removeElement(tri);

  Triangle *t1 = new Triangle(va,vb,new_v,mesh,mat);
  Triangle *t2 = new Triangle(vb,vc,new_v,mesh,mat);
  Triangle *t3 = new Triangle(vc,va,new_v,mesh,mat);
  mesh->addElement(t1);
  mesh->addElement(t2);
  mesh->addElement(t3);
  added.insert(t1->getID());
  added.insert(t2->getID());
  added.insert(t3->getID());
  return 1;
}


int ReMesh::SplitEdge(Element *e, int i, int a, int b, double fraction, std::set<ElementID> &removed, std::set<ElementID> &added) {

  Mesh *mesh = e->getMesh();

  assert ((*e)[i] == a);
  assert ((*e)[(i+1)%e->numVertices()] == b);

  // add this element to the std::vector, and split the edge!
  std::vector<Element*> vec = e->getNeighbors(i);
  vec.push_back(e);

  // make a new vertex in the middle of the edge
  Vertex *vert = mesh->getVertex(a);
  Vertex *vert2 = mesh->getVertex(b);

  double s1,t1,s2,t2;
  vert->getTextureCoordinates(s1,t1);
  vert2->getTextureCoordinates(s2,t2);

  Vec3f pos = (1-fraction)*vert->get() + fraction*vert2->get();
  double s = (1-fraction)*s1 + fraction*s2;
  double t = (1-fraction)*t1 + fraction*t2;

  //  Vec3f normal; e->computeNormal(normal);
  //if (normal.Dot3(Vec3f(0,1,0)) > 0.9) {
    //    std::cout << "split: adding vert " << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;
  //}

  int new_v = mesh->addVertex(pos,a,b,s,t);

  // look over all the triangles at that edge
  for (unsigned int k = 0; k < vec.size(); k++) {
    Element *e2 = vec[k];
    assert (e2->HasVertex(a));
    assert (e2->HasVertex(b));
    std::string mat = e2->getFakeMaterial();
    if (e2->isALine()) {
      continue;

    } else if (e2->isATriangle()) {
      int w_a = e2->WhichVertex(a);
      int w_b = e2->WhichVertex(b);
      int w_c = 3 - w_a - w_b;
      int va = (*e2)[w_a];
      int vb = (*e2)[w_b];
      int vc = (*e2)[w_c];
      assert (va != vb && va != vc && vb != vc);
      removed.insert(e2->getID());
      mesh->removeElement(e2);
      if (w_b - w_a == 1 || w_b - w_a == -2) {
	Triangle *t1 = new Triangle(va,new_v,vc,mesh,mat);
	Triangle *t2 = new Triangle(new_v,vb,vc,mesh,mat);
	mesh->addElement(t1);
	mesh->addElement(t2);
	added.insert(t1->getID());
	added.insert(t2->getID());
      } else {
	Triangle *t1 = new Triangle(vc,new_v,va,mesh,mat);
	Triangle *t2 = new Triangle(new_v,vc,vb,mesh,mat);
	mesh->addElement(t1);
	mesh->addElement(t2);
	added.insert(t1->getID());
	added.insert(t2->getID());
      }
    } else {
      assert (e2->isAQuad());
      //int foo = rand();
      //std::cout << foo << std::endl;
      //continue;//if (foo % 2 == 1) continue;
 
      int w_a = e2->WhichVertex(a);
      int w_b = e2->WhichVertex(b);
      assert ((w_a+1)%4 == w_b ||
	      (w_b+1)%4 == w_a);
      int w_c,w_d;
      if ((w_a+1)%4 == w_b) {
	w_c = (w_a+2)%4;
	w_d = (w_a+3)%4;
      } else {
	w_c = (w_a+1)%4;
	w_d = (w_a+2)%4;
      }
      int va = (*e2)[w_a];
      int vb = (*e2)[w_b];
      int vc = (*e2)[w_c];
      int vd = (*e2)[w_d];
      int mid_opposite = mesh->getChildVertex(vc,vd);
      if (mid_opposite == -1) continue;
      assert (va != vb && va != vc && vb != vc &&
			vd != va && vd != vb && vd != vc);
      removed.insert(e2->getID());
      mesh->removeElement(e2);

      Quad *q1, *q2;
      if ((w_a+1)%4 == w_b) {
	q1 = new Quad(va,new_v,mid_opposite,vd,mesh,mat);
	q2 = new Quad(new_v,vb,vc,mid_opposite,mesh,mat);
      } else {
	q1 = new Quad(vb,new_v,mid_opposite,vd,mesh,mat);
	q2 = new Quad(new_v,va,vc,mid_opposite,mesh,mat);
      }
      mesh->addElement(q1);
      mesh->addElement(q2);
      added.insert(q1->getID());
      added.insert(q2->getID());

      /*
      if ((w_a+1)%4 == w_b) {
	mesh->addElement(new Triangle(va,new_v,vd,mesh,mat));
	mesh->addElement(new Quad(new_v,vb,vc,vd,mesh,mat));
      } else {
	mesh->addElement(new Triangle(vb,new_v,vd,mesh,mat));
	mesh->addElement(new Quad(new_v,va,vc,vd,mesh,mat));
	}
      */
    }
  }

  return 1;
}


// =============================================================================
