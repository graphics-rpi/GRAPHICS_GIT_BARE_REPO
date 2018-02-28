#include "remesh.h"
#include "mesh.h"
#include "meshmanager.h"
#include "collect.h"
#include "triangle.h"

// =============================================================================
// =============================================================================
// MOVE VERTICES

int ReMesh::MoveVertices(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  mesh->RecomputeStats();
  assert (mesh != NULL);

  // compute new positions
  int answer = 0;
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    //Iterator<Element*> *iter = mesh->getElements()->StartIteration();
    //while (Element *e = iter->GetNext()) {
    Element *e = foo->second;
    int num_verts = e->numVertices();
    for (int i = 0; i < num_verts; i++) {
      Vertex *v = mesh->getVertex((*e)[i]);
      if (v->hasMovedPosition()) continue;
      answer += MoveVertex(meshes->args,e,(*e)[i]); 
    } 
  }
  //  mesh->getElements()->EndIteration(iter);
  
  // copy those positions
  // (do this later since normals will need to be recomputed)
  unsigned int num_verts = mesh->numVertices();
  for (unsigned int i = 0; i < num_verts; i++) {
    mesh->getVertex(i)->MovePosition(); }
  
  // some update stuff
  mesh->RecomputeStats();
  return answer;
}

bool ReMesh::MoveVertexHelper(Mesh *mesh, ArgParser *args, std::vector<Element*> &element_vec, int vert, std::vector<int> &verts,
			      int &edge_v0, int &edge_v1, double &shortest_edge, const Vec3f &normal, 
			      bool zero_area_flag) {

  bool answer = true;
  std::string mat = "NOT SET";

  // create an std::vector of all neighboring vertices, also:
  for (unsigned int ev = 0; ev < element_vec.size(); ev++) {
    Element *e = element_vec[ev];
    //Triangle *t = (Triangle*)element_vec[ev];
    assert (e != NULL);

    // make sure they all have the same material
    if (mat == "NOT SET") mat = e->getFakeMaterial();
    if (mat != e->getFakeMaterial()) answer = false; 

    // find the shortest edge
    double se = e->ShortestMovedEdge();
    if (shortest_edge < 0 || se < shortest_edge)
      shortest_edge = se;

    // check to see if the normal is within the tolerance
    // NOTE: this does not verify that the normals will also be good afterwards!
    Vec3f n;
    e->computeMovedNormal(n);
    double area = e->MovedPositionArea();
    if (area < mesh->ZeroAreaTolerance()) {
      zero_area_flag = true;
    } else if (normal.Dot3(n) < args->remesh_normal_tolerance  && !args->equal_edge_and_area) {
      answer = false;
    }
    
    int num_verts = e->numVertices();
    // detect if this vertex is on the boundary
    for (int i = 0; i < num_verts; i++) {
      // HACK FOR NOW
      if (mesh->IsNonManifoldEdge(e->get_my_edge(i))) /*,(*e)[i],(*e)[(i+1)%num_verts]))*/ return false;	
      if (!mesh->IsManifoldInteriorEdge(e->get_my_edge(i))) /*,(*e)[i],(*e)[(i+1)%num_verts]))*/ {
	//std::vector<Element*> neighbors = t->getNeighbors(i);
	//if (neighbors.size() != 1) {  //t->getNeighbor(i) == NULL) {	
	if ((*e)[i] == vert) {
	  assert (edge_v0 == -1);
	  edge_v0 = (*e)[(i+1)%3];
	} else if ((*e)[(i+1)%3] == vert) {
	  if (edge_v1 != -1) {
	    return false;
	  }
	  assert (edge_v1 == -1);
	  edge_v1 = (*e)[i];
	}
      }
      int v = (*e)[i];
      if (v == vert) 
	verts.push_back((*e)[(i+1)%3]);
    }
  }
  //bag.EndIteration(iter);

  return answer;
}

bool MoveVertexBoundary(ArgParser *args, const Vec3f &P, const Vec3f &A, const Vec3f &B, 
			Vec3f &new_pos, double &new_s, double &new_t) {
  return false;
  assert (!P.isNaN());
  assert (!A.isNaN());
  assert (!B.isNaN());
  double old_area = AreaOfTriangle(P,A,B);
  assert(!my_isnan(old_area));
  new_pos = P;
  Vec3f vAB = B - A; double dAB = vAB.Length(); vAB.Normalize();
  Vec3f vAP = P - A; /*double dAP = vAP.Length();*/ vAP.Normalize();
  Vec3f vPB = B - P; /*double dPB = vPB.Length();*/ vPB.Normalize();
  assert(!vAB.isNaN());
  assert(!vAP.isNaN());
  assert(!vPB.isNaN());
  if (vAP.Dot3(vPB) < args->remesh_normal_tolerance) {
    return 0;
  }
  new_pos = 0.5 * (A + B);
  if (args->preserve_volume) {
    Vec3f crossA;
    Vec3f::Cross3(crossA,vAB,vAP);
    crossA.Normalize();
    assert(!crossA.isNaN());
    Vec3f perp;
    Vec3f::Cross3(perp,crossA,vAB);
    perp.Normalize();
    assert(!perp.isNaN());
    assert(!my_isnan(dAB));
    double dist = 2 * old_area / dAB;
    assert(!my_isnan(dist));
    Vec3f diff = dist * perp;
    new_pos += diff;
  }
  assert(!new_pos.isNaN());
  //double new_area = AreaOfTriangle(new_pos,A,B);
  return 1;
}


int ReMesh::MoveVertex(ArgParser *args, Element *e, int vert) {

  if (!e->isATriangle()) return 0;
  Triangle *t = (Triangle*)e;

  Mesh *mesh = (Mesh*)t->getMesh();
  Vertex *vrt = mesh->getVertex(vert);
  if (vrt->hasMovedPosition()) return 0;

  // collect all the elements that share this vertex
  std::vector<Element*> element_vec; 
  Collect::CollectElementsWithVertex(t,vert,element_vec);
  std::vector<int> verts;
  int edge_v0 = -1;
  int edge_v1 = -1;
  double shortest_edge = -1;
  Vec3f normal = vrt->getNormal();

  Vec3f normal2; t->computeMovedNormal(normal2);
  double dot = normal.Dot3(normal2);
  if (dot < 0.1) {
    //cout << " DOT BAD " << dot << endl;
  }

  bool zero_area_flag = true;
  bool ok = MoveVertexHelper(mesh,args,element_vec,vert, verts,edge_v0,edge_v1,shortest_edge,normal,zero_area_flag);
  
  assert (!vrt->hasMovedPosition());
  Vec3f old_pos = vrt->get();
  if (!ok && !args->equal_edge_and_area) {
    // don't move the vertex
    double s,t;
    vrt->getTextureCoordinates(s,t);
    vrt->setMovedPosition(old_pos.x(),old_pos.y(),old_pos.z(),s,t);
    return 0;
  }

  Vec3f new_pos = old_pos;
  bool answer = false;
  double new_s = -1;
  double new_t = -1;
  vrt->getTextureCoordinates(new_s,new_t);

  // if this vertex is on the boundary..
  if (edge_v0 != -1) {
    if (edge_v1 == -1) {
      return false;
    }

    assert (edge_v1 != -1);    
    assert(vert != edge_v0 && vert != edge_v1 && edge_v1 != edge_v0);
    Vec3f p = mesh->getVertex(vert)->getPositionOrMovedPosition(); 

    Vec3f p0 = mesh->getVertex(edge_v0)->getPositionOrMovedPosition(); 
    Vec3f p1 = mesh->getVertex(edge_v1)->getPositionOrMovedPosition(); 
    answer = MoveVertexBoundary(args,p,p0,p1,new_pos,new_s,new_t);
    assert (answer == false);
    //cout << "might not have done boundary right" << endl;

  } else {
    if (edge_v1 != -1) {
      return false;
    }
    assert (edge_v1 == -1);
    assert (verts.size() == element_vec.size() && verts.size() > 0);  
    new_s = 0;
    new_t = 0;
    new_pos = Vec3f(0,0,0);

    for (unsigned int i = 0; i < verts.size(); i++) {
      Vertex *tmp_vert = mesh->getVertex(verts[i]);
      assert (tmp_vert != NULL);
      new_pos += tmp_vert->getPositionOrMovedPosition();
      double tmp_s,tmp_t;
      tmp_vert->getTextureCoordinates(tmp_s,tmp_t);
      //cout << "        " << tmp_s << " " << tmp_t << endl;
      //tmp_s = 0.5;
      //tmp_t = 0.5;
      //if (tmp_s < -0.01 || tmp_s > 1.01) {std::cout << "CRAP" << std::endl; }//exit(0);}
      //if (tmp_t < -0.01 || tmp_t > 1.01) {std::cout << "CRAP" << std::endl; }//exit(0);}
      new_s += tmp_s;
      new_t += tmp_t;
    }

    assert (verts.size() > 0);
    new_pos /= double(verts.size());
    new_s /= double(verts.size());
    new_t /= double(verts.size());

    // to reduce volume loss/gain
    // (not exact)
    if (args->preserve_volume == 1) {
      
      if (normal.Dot3(normal) < 0.99) {
        //cout << "normal " << normal.Dot3(normal) << endl;
      }
      double d1 = normal.Dot3(old_pos);
      double d2 = normal.Dot3(new_pos);
      double d = d1-d2;
      new_pos += normal * d;
      //cout << " might not do the coordinates right here either" << endl;
    }
    double dist = DistanceBetweenTwoPoints(new_pos,old_pos);
    if (!args->equal_edge_and_area) 
      answer = (dist > 0.001 * shortest_edge);
  }

  if (zero_area_flag == true) {
    //cout << "check afterwards!" << endl;
    // make sure that the normals are good afterwards!
    for (unsigned int ev = 0; ev < element_vec.size(); ev++) {// eIterator<Element*> *iter = element_vec.StartIteration();

      Element *e = element_vec[ev];
      if (!e->isATriangle())
	return 0;

      Triangle *t = (Triangle*)e;//element_vec[ev]; //iter->GetNext()) {    
      assert (t != NULL);
      Mesh *mesh = t->getMesh();
      
      Vec3f v0 = mesh->getVertex((*t)[0])->getPositionOrMovedPosition();
      Vec3f v1 = mesh->getVertex((*t)[1])->getPositionOrMovedPosition();
      Vec3f v2 = mesh->getVertex((*t)[2])->getPositionOrMovedPosition();
      if ((*t)[0] == vert) {
        v0 = new_pos;
      } else if ((*t)[1] == vert) {
        v1 = new_pos;
      } else {
        assert ((*t)[2] == vert);
        v2 = new_pos;        
      }
      Vec3f normal2;
      ::computeNormal(v0,v1,v2,normal2);

      double dot = normal.Dot3(normal2);      
      //double area = AreaOfTriangle(v0,v1,v2);
      
      //if (dot < args->remesh_normal_tolerance && !Triangle::NearZeroArea(v0,v1,v2)) {
      if (dot < 0.9 && !Triangle::NearZeroArea2(v0,v1,v2,mesh)) {
	  //area > zero_area_tolerance && dot < 0.9) {
	  // dont move
	  new_pos = old_pos;
	  vrt->getTextureCoordinates(new_s,new_t);
	  //cout << " DONT MOVE " << endl;
	} else {
	  // ok to move
	}
	}
    //bag.EndIteration(iter);
  }

  vrt->setMovedPosition(new_pos.x(),new_pos.y(),new_pos.z(),new_s,new_t);
  return answer;
}

