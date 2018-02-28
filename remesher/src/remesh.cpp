#include <iostream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string>

#include "vertex.h"
#include "element.h"
#include "triangle.h"
#include "mesh.h"
#include "meshmanager.h"
#include "collect.h"
#include "remesh.h"
#include "argparser.h"
#include "edge.h"
#include "quad.h"
#include "accelerationgrid.h"
#include "glCanvas.h"
#include "walls.h"
#include "wall.h"

int GLOBAL_DESIRED_COUNT = -1;
double GLOBAL_DESIRED_LENGTH = -1;
double GLOBAL_LONGEST_EDGE = -1;
double GLOBAL_SHORTEST_EDGE = -1;
double EXTRA_LENGTH_MULTIPLIER = 10;
double SSS_EXTRA_LENGTH_MULTIPLIER = 3;

#define CHECK_PROJECTOR_VIEW_FRUSTUM 1

bool OnLine(const Vec3f &pt, const Vec3f &endpt1, const Vec3f &endpt2);


#include "argparser.h"
extern ArgParser *ARGS;


// =============================================================================
// =============================================================================

void ReMesh::Evaluate(MeshManager *meshes) {
  static int counter = -1;

  //(*ARGS->output) << "EVALUATE" << counter << std::endl;

  EXTRA_LENGTH_MULTIPLIER = meshes->args->extra_length_multiplier;

  Mesh *mesh = meshes->getMesh();
  assert (mesh != NULL);

  double longest_edge = mesh->LongestEdge();
  double shortest_edge = mesh->ShortestEdge();
  //double area = mesh->Area();
  double area = mesh->WeightedArea();
  int desired_count = meshes->args->desired_tri_count;
  double desired_length = sqrt (2*area / double(desired_count));
  int current_count = mesh->numElements();

  //(*ARGS->output) << "des len" << desired_length << std::endl;
  
  double adjuster = current_count / double(desired_count);
  //adjuster = (adjuster+4) / 5.0;
  //(*ARGS->output) << "des:" << desired_count << "  cur:" << current_count << "  adj:" << adjuster << std::endl;

  
  if (GLOBAL_DESIRED_COUNT != desired_count) {
    GLOBAL_DESIRED_COUNT = desired_count;
    GLOBAL_DESIRED_LENGTH = desired_length;
    counter = 1;
  } else {
    counter++;
  }

  //(*ARGS->output) << "GLOBAL_DESIRED_LENGTH " << GLOBAL_DESIRED_LENGTH << std::endl;

  if (counter >= 10) {
    double foo = GLOBAL_DESIRED_LENGTH * adjuster;
    GLOBAL_DESIRED_LENGTH = 0.9 * GLOBAL_DESIRED_LENGTH + 0.1 * foo;
  }

  //  (*ARGS->output) << "GLOBAL_DESIRED_LENGTH " << GLOBAL_DESIRED_LENGTH << std::endl;

  GLOBAL_DESIRED_LENGTH = max2(0.5*desired_length,min2(2.0*desired_length,GLOBAL_DESIRED_LENGTH));
  
  //  (*ARGS->output) << "GLOBAL_DESIRED_LENGTH " << GLOBAL_DESIRED_LENGTH << std::endl;

  GLOBAL_LONGEST_EDGE = longest_edge;
  GLOBAL_SHORTEST_EDGE = shortest_edge;
}

// =============================================================================
// =============================================================================
double ReMesh::WorstAngleOnProjectionTriangle(MeshManager *meshes) {
  double answer = M_PI;
  Mesh *mesh = meshes->getMesh();

  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    if (!e->isATriangle()) continue;
    if (!e->getMaterialPtr()->isProjection()) continue;
    //if (!mesh->getMaterial(e->getRealMaterial()).isProjection()) continue;
    Triangle *t = (Triangle*)e;
    Vec3f a = mesh->getVertex((*t)[0])->get();
    Vec3f b = mesh->getVertex((*t)[1])->get();
    Vec3f c = mesh->getVertex((*t)[2])->get();
    double tmp = min3(AngleBetween(b-a,c-a),
		      AngleBetween(c-b,a-b),
		      AngleBetween(a-c,b-c));
    if (tmp < answer)
      answer = tmp;
  }
  return answer;
}

// =============================================================================
// =============================================================================
void ReMesh::BadTrianglesStatus(MeshManager *meshes) {
  int num = meshes->getMesh()->numBadElements();
  (*ARGS->output) << "We have " << num << " bad triangles." << std::endl;
  Mesh *mesh = meshes->getMesh();

  int bad_count = 0;
  int zero_area_count = 0;
  int bad_normal_count = 0;
  int bad_neighbor_count = 0;

  // put all element ids in vector
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    //Iterator<Element*> *iter = mesh->getElements()->StartIteration();
    //while (Element *e = iter->GetNext()) {
    Element *e = foo->second;
    //  Iterator<Element*> *iter = mesh->getElements()->StartIteration();
    //while (Element *e = iter->GetNext()) {
    Triangle *t = (Triangle*)e;
    if (t->HasBadNeighbor()) 
      bad_neighbor_count++;
    if (t->IsBad()) {
      bad_count++;
      if (t->NearZeroArea())
	zero_area_count++;
      if (t->BadNormal())
	bad_normal_count++;
    }
  }
  //  mesh->getElements()->EndIteration(iter);
  
  assert (num == bad_count);
  (*ARGS->output) << "zero area count " << zero_area_count << std::endl;
  (*ARGS->output) << "bad normal count " << bad_normal_count << std::endl;
  (*ARGS->output) << "bad neighbor count " << bad_neighbor_count << std::endl;


#if 0
  iter = mesh->getElements()->StartIteration();
  while (Triangle *t = iter->GetNext()) {
    //t->Print();
    (*ARGS->output) << "triangle " << (int)t << std::endl;
    for (int i = 0; i < 3; i++) {
      Edge *edge = mesh->GetEdge((*t)[i],(*t)[(i+1)%3],t);
      assert (edge != NULL);
      (*ARGS->output) << "    " << (*edge)[0] << " " << (*edge)[1] << "    " << (int)edge->getElement() << std::endl;
     
      std::vector<Edge*> se = edge->getSharedEdges();
      std::vector<Edge*> oe = edge->getOpposites();

      for (unsigned int j = 0; j < se.size(); j++) {
	Edge *e2 = se[j];
	(*ARGS->output) << "  se " << (*e2)[0] << " " << (*e2)[1] << "    " << (int)e2->getElement() << std::endl;
      }

      for (unsigned int j = 0; j < oe.size(); j++) {
	Edge *e2 = oe[j];
	(*ARGS->output) << "  oe " << (*e2)[0] << " " << (*e2)[1] << "    " << (int)e2->getElement() << std::endl;
      }
 
    }
  }
  mesh->getElements()->EndIteration(iter);
#endif

}


bool ReMesh::EliminateBadTriangles(MeshManager *meshes) {
  int num_before = meshes->getMesh()->numBadElements();
  Mesh *mesh = meshes->getMesh();

  // put all element ids in vector
  std::vector<ElementID> ids;
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    ids.push_back(foo->second->getID());
  }

  // for each id...  if it's a bad triangle...
  int count = ids.size();
  for (int i = 0; i < count; i++) {
    Element *e = Element::GetElement(ids[i]);
    if (e == NULL) continue;
    //assert (e->isATriangle());
    //Triangle *t = (Triangle*)e;
    int num_verts = e->numVertices();
    if (e->BadNormal()) {
      // try to collapse
      int index = e->ShortestEdgeIndex();
      for (int q = 0; q < num_verts; q++) {
	//printf("i %d q %d \n",i,q);
	//int a = (*e)[(index+q)%num_verts];
	//int b = (*e)[(index+q+1)%num_verts];
	int tmp = (q+index)%num_verts;
	if (TryCollapse(meshes->args,e,e->get_my_edge(tmp),false,true) ||
	    TryCollapse(meshes->args,e,e->get_my_edge(tmp),true,true)) {
	  goto done_element;
	}
      }      
      
    } else if (e->NearZeroArea()) {
      // try to collapse
      int index = e->ShortestEdgeIndex();
      for (int q = 0; q < num_verts; q++) {
	//int a = (*e)[(index+q)%num_verts];
	//int b = (*e)[(index+q+1)%num_verts];
	int tmp = (q+index)%num_verts;
	//printf("i %d q %d \n",i,q);
	if (TryCollapse(meshes->args,e,e->get_my_edge(tmp),false,true) ||
	    TryCollapse(meshes->args,e,e->get_my_edge(tmp),true,true)) {
	  //if (Collapse(meshes->args,e,a,b,true) ||
	  // Collapse(meshes->args,e,b,a,true)) {
	  goto done_element;
	}
      }      
      
      // try to flip
      
    }

  done_element: {}        
  } 
  mesh->RecomputeStats();

  int num_after = meshes->getMesh()->numBadElements();
  //assert (num_after <= num_before);
 
  // return true if there are still some bad triangles, and repeating this might help
  return (num_after != 0 && num_after < num_before);
}



// =============================================================================
// =============================================================================

void FixSeamsHelper(Mesh *mesh, int i, int j) {
  assert (i < j);
  assert (i >= 0 && j < (int)mesh->numVertices());
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    int numverts = e->numVertices();
    for (int k = 0; k < numverts; k++) {
      if ((*e)[k] == j) {
	mesh->RemoveEdges(e);
	assert (e->isATriangle());
	((Triangle*)e)->setVertex(k,i);
	mesh->AddEdges(e);
      }
    }
  }
}

void ReMesh::FixSeams(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  double max_dim = mesh->getBoundingBox()->maxDim();
  int count = 0;
  for (int i = 0; i < (int)mesh->numVertices(); i++) {
    Vec3f vi = mesh->getVertex(i)->get();
    count++;
    if (count%100==0) { printf ("."); fflush(stdout); }
    for (int j = i+1; j < (int)mesh->numVertices(); j++) {
      Vec3f vj = mesh->getVertex(j)->get();
      double d = DistanceBetweenTwoPoints(vi,vj);
      if (d < 0.000001 * max_dim) {
	FixSeamsHelper(mesh,i,j);
      }
    }
  }
  mesh->RecomputeStats();
}



void ReMesh::Triangulate(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();

  // put all element ids in vector
  std::vector<ElementID> ids;
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    ids.push_back(foo->second->getID());
  }

  int count = ids.size();
  for (int i = 0; i < count; i++) {
    Element *e = Element::GetElement(ids[i]);
    assert (e != NULL);
    if (!e->isAQuad()) continue;
    assert (e->isAQuad());
    int a = (*e)[0];
    int b = (*e)[1];
    int c = (*e)[2];
    int d = (*e)[3];
    Vertex *va = mesh->getVertex(a);
    Vertex *vb = mesh->getVertex(b);
    Vertex *vc = mesh->getVertex(c);
    Vertex *vd = mesh->getVertex(d);
    double ac = (va->get()-vc->get()).Length();
    double bd = (vb->get()-vd->get()).Length();
    std::string mat = e->getFakeMaterial();
    mesh->removeElement(e);
    if (ac < bd) {
      mesh->addElement(new Triangle(a,b,c,mesh,mat));
      mesh->addElement(new Triangle(a,c,d,mesh,mat));
    } else {
      mesh->addElement(new Triangle(d,a,b,mesh,mat));
      mesh->addElement(new Triangle(d,b,c,mesh,mat));
    }
  }
  assert (mesh->numQuads() == 0);
  mesh->RecomputeStats();
}






class Triple {
public:
  Triple() : a(-1), b(-1), c(-1), mat("") {}
  Triple (int _a, int _b, int _c, const std::string& m) :
    a(_a), b(_b), c(_c), mat(m) {}
  friend std::ostream& operator<< (std::ostream &ostr, const Triple &t) {
    ostr << "Triple: " << t.a << " " << t.b << " " << t.c << " " << t.mat << std::endl;
    return ostr;
  }
  int a, b, c;
  std::string mat;
};


bool CutEdgesHelper(Mesh *mesh, int vert, std::vector<Element*> &vec, double angle) {

  if (vec.size() < 2) return false;
  assert (vec.size() >= 2);
  
  Triple keep[150];     int keep_count = 0;
  Triple change[150];   int change_count = 0;
    
  double max = 1;
  Triangle *tri_a = NULL;
  Triangle *tri_b = NULL;

  for (unsigned int i = 0; i < vec.size(); i++) {
    Triangle *tri_i = (Triangle*)vec[i];
    Vec3f normal_i; tri_i->computeNormal(normal_i);
    for (unsigned int j = 0; j < vec.size(); j++) {
      if (i == j) continue;
      Triangle *tri_j = (Triangle*)vec[j];
      Vec3f normal_j; tri_j->computeNormal(normal_j);
      double dot = normal_i.Dot3(normal_j);
      if (tri_a == NULL || dot < max) {
        tri_a = tri_i;
        tri_b = tri_j;
        max = dot;
      }
    }
  }

  if (max >= angle) return false;

  Vec3f normal_a; tri_a->computeNormal(normal_a);
  Vec3f normal_b; tri_b->computeNormal(normal_b);

  (*ARGS->output) << std::endl;
  for (unsigned int i = 0; i < vec.size(); i++) {
    Triangle *tri = (Triangle*)vec[i];
    Vec3f normal; tri->computeNormal(normal);
    double dot_a = normal.Dot3(normal_a);
    double dot_b = normal.Dot3(normal_b);
    (*ARGS->output) << "dots " << dot_a << " " << dot_b << std::endl;
    if (tri == tri_a ||
        (tri != tri_b && dot_a > dot_b)) {
      // join a & keep old vert
      keep[keep_count] = Triple((*tri)[0],(*tri)[1],(*tri)[2],tri->getFakeMaterial());
      keep_count++;
      assert (keep_count < 100);
    } else {
      // join b & change 
      change[change_count] = Triple((*tri)[0],(*tri)[1],(*tri)[2],tri->getFakeMaterial());
      change_count++;
      assert (change_count < 100);
    }      
  }

  assert (keep_count > 0);
  assert (change_count > 0);
  
  for (unsigned int i = 0;  i < vec.size(); i++) {
    Triangle *tri = (Triangle*)vec[i];
    mesh->removeElement(tri);
  }

  for (int i = 0; i < keep_count; i++) {
    mesh->addElement(new Triangle(keep[i].a,keep[i].b,keep[i].c,mesh,keep[i].mat));
  }
  
  Vec3f pos = mesh->getVertex(vert)->get();

  (*ARGS->output) << " NOT DOING TEXTURE COORDINATES CORRECTLY IN CUT EDGES " << std::endl;
  int vert2 = mesh->addVertex(pos,-1,-1, 0, 0);
  for (int i = 0; i < change_count; i++) {
    if (change[i].a == vert) {
      mesh->addElement(new Triangle(vert2,change[i].b,change[i].c,mesh,change[i].mat));
    } else if (change[i].b == vert) {
      mesh->addElement(new Triangle(change[i].a,vert2,change[i].c,mesh,change[i].mat));
    } else {
      assert (change[i].c == vert);
      mesh->addElement(new Triangle(change[i].a,change[i].b,vert2,mesh,change[i].mat));
    }
  }
  
  // did something
  return true;
}

void ReMesh::CutEdges(MeshManager *meshes) {
  (*ARGS->output) << "in cut edges..." << std::endl;

  Mesh *mesh = meshes->getMesh();
  double angle = meshes->args->cut_normal_tolerance;

  while (1) {
    int count = 0;
    for (unsigned int i = 0; i < mesh->numVertices(); i++) {
      std::vector<Element*> vec;
      for (elementshashtype::const_iterator foo = mesh->getElements().begin();
	   foo != mesh->getElements().end();
	   foo++) {
	Element *e = foo->second;
	assert (e->isATriangle());
	Triangle *t = (Triangle*)e;
	if ((*t)[0] == (int)i ||
	    (*t)[1] == (int)i ||
	    (*t)[2] == (int)i)
          vec.push_back(t);
      }
      
      bool tmp = CutEdgesHelper(mesh,i,vec,angle);
      if (tmp) {
        count++;
      }
    }
    (*ARGS->output) << "split " << count << " vertices" << std::endl;
    if (count == 0) break;
  }
  
  mesh->RecomputeStats();
  (*ARGS->output) << "done with cut edges" << std::endl;
}



void ReMesh::MoveVerticesRandomly(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  ArgParser *args = meshes->args;
  unsigned int num_verts = mesh->numVertices();

  for (unsigned int i = 0; i < num_verts; i++) { 
    Vertex *v = mesh->getVertex(i);     
    Vec3f pos = v->get();
    pos += Vec3f(args->RandomVector()); //next(),args->random.next(),args->random.next());
    (*ARGS->output) << " NOT DOING TEXTURE COORDINATES CORRECTLY FOR MOVE VERTICES RANDOMLY" << std::endl;
    v->set2(pos,0,0);
  }
  
  mesh->RecomputeStats();
  (*ARGS->output) << "done with move vertices" << std::endl;
}


void ReMesh::CompressVertices(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  mesh->CompressVertices();
  //  ArgParser *args = meshes->args;

  //  unsigned int num_verts = mesh->numVertices();

  //  for (unsigned int i = 0; i < num_verts; i++) { 
  //Vertex *v = mesh->getVertex(i);     
  // Vec3f pos = v->get();
  //pos += Vec3f(args->RandomVector()); //next(),args->random.next(),args->random.next());
  //(*ARGS->output) << " NOT DOING TEXTURE COORDINATES CORRECTLY FOR MOVE VERTICES RANDOMLY" << std::endl;
  //v->set2(pos,0,0);
  //  }
  
  mesh->RecomputeStats();
  (*ARGS->output) << "done with compress vertices" << std::endl;
}


// --------------------------------------------------------------------
// --------------------------------------------------------------------
// --------------------------------------------------------------------

void MakePlanes(const BasicWall& wall, Vec3f &proj_center, int which_projector, MeshManager *meshes) {
  double scene_radius = meshes->getWalls()->getSceneRadius();
  Vec3f height = Vec3f(0,wall.getMaxHeight(),0);
  Vec3f bottom = Vec3f(0,wall.getBottomEdge(),0);
  //unsigned int num_quads = wall.numConvexQuads();
  unsigned int num_good_verts = wall.numGoodVerts();  
  for (unsigned int t = 0; t < num_good_verts; t++) {
    Vec3f A = wall.getGoodVert((t+num_good_verts-1)%num_good_verts); 
    Vec3f B = wall.getGoodVert(t);
    Vec3f C = wall.getGoodVert((t+1)%num_good_verts); 
    Vec3f my_normal;
    computeNormal(A,B,C,my_normal);
    double my_dot = my_normal.Dot3(Vec3f(0,-1,0));
    for (int k = 0; k < 3; k++) {
      Vec3f a,b,pt_left,pt_right;
      Vec3f normal_left,normal_right;
      if (k == 0) {
	// top edge
	a        = A + height; 
	b        = B + height; 
	pt_left  = C + height; 
	pt_right = A + bottom; 
	computeNormal(b,a,pt_left,normal_left);
	computeNormal(a,b,pt_right,normal_right);

	if (my_dot < 0)
	  normal_left = -normal_left;
	assert (normal_left.Dot3(Vec3f(0,1,0)) > 0.9);
	assert (fabs(normal_right.Dot3(Vec3f(0,1,0))) < 0.1);
      } else if (k == 1) {
	// bottom edge
	a        = A + bottom; 
	b        = B + bottom; 
	pt_left  = A + height; 
	pt_right = C + bottom; 
	computeNormal(b,a,pt_left,normal_left);
	computeNormal(a,b,pt_right,normal_right);
	assert (fabs(normal_left.Dot3(Vec3f(0,1,0))) < 0.1);
	if (my_dot < 0)
	  normal_right = -normal_right;
	assert (normal_right.Dot3(Vec3f(0,-1,0)) > 0.9);
      } else {
	assert (k == 2);
	// side edges
	a        = B + bottom; 
	b        = B + height; 
	pt_left  = A + bottom; 
	pt_right = C + bottom; 
	computeNormal(b,a,pt_left,normal_left);
	computeNormal(a,b,pt_right,normal_right);
	assert (fabs(normal_left.Dot3(Vec3f(0,1,0))) < 0.1);
	assert (fabs(normal_right.Dot3(Vec3f(0,1,0))) < 0.1);
      }
      
      Vec3f dir = a-proj_center;
      dir.Normalize();
      double test_d2 = dir.Dot3(normal_left);
      double test_d3 = dir.Dot3(normal_right);
      if (test_d2*test_d3 > 0) { continue; }
      CutPlane tmp;
      tmp.quad[0] = a;
      tmp.quad[1] = b;
      tmp.quad[2] = proj_center + 5*scene_radius*((tmp.quad[1]-proj_center).Normalize());
      tmp.quad[3] = proj_center + 5*scene_radius*((tmp.quad[0]-proj_center).Normalize());
      computeNormal(tmp.quad[0],tmp.quad[1],tmp.quad[2],tmp.normal);
      tmp.which_projector = which_projector;
      tmp.projector_center = proj_center;
      tmp.d = -tmp.normal.Dot3(tmp.quad[0]);
      meshes->planes.push_back(tmp);
    }
  }
}
      
bool plane_sorter(const CutPlane &a, const CutPlane &b) {
  //double da = DistanceBetweenTwoPoints(a.quad[0],a.quad[1]);
  //double db = DistanceBetweenTwoPoints(b.quad[0],b.quad[1]);

  double angle_a = AngleBetween(a.quad[0]-a.projector_center,b.quad[1]-b.projector_center);
  double angle_b = AngleBetween(a.quad[0]-a.projector_center,b.quad[1]-b.projector_center);

  if (angle_a > angle_b) return true;

  //  if (da > db) return true;
  return false;
}

void ReMesh::CutThroughPlanes(MeshManager *meshes) {

  (*ARGS->output) << "CUT THROUGH PLANES" << std::endl;
  //if (meshes->args->projectors.size() == 0 ) return;
  if (meshes->args->triangle_textures_and_normals == false) return;

  (*ARGS->output) << "cut on silhouettes ... "; 
  fflush(stdout);
  Mesh *mesh = meshes->getMesh();
  int num_els = mesh->numElements();
  double scene_radius = meshes->getWalls()->getSceneRadius();

  ArgParser *args = meshes->args;

#if 1
  while (1) {
    if (meshes->args->no_blending_hack) {
      SplitEdges(meshes,scene_radius/1.1);


    } else {

      SplitEdges(meshes,scene_radius/args->blending_subdivision);
      //SplitEdges(meshes,scene_radius/50);
      //SplitEdges(meshes,scene_radius/20);
      //SplitEdges(meshes,scene_radius/12.1);

      //SplitEdges(meshes,scene_radius/5.4);
      //SplitEdges(meshes,scene_radius/4.1);  // last in empac?

      //SplitEdges(meshes,scene_radius/2);

      //SplitEdges(meshes,scene_radius/10.1);
      //SplitEdges(meshes,scene_radius/3.1);
    }
    int tmp = mesh->numElements();
    if (tmp == num_els) break;
    num_els = tmp;
  }
#endif

  meshes->planes.clear();
  std::vector<Vec3f> projector_centers;
  //ConstructPlanes(meshes,projector_centers); 
  ConstructPlanes2(meshes,projector_centers); 
  (*ARGS->output) << "num planes " << meshes->planes.size() << std::endl;
  //  if (meshes->planes.size() == 0) return;
#if 1
  sort(meshes->planes.begin(),meshes->planes.end(),plane_sorter);
#endif

  (*ARGS->output) << "FINISHED SORT" << std::endl;

  if (meshes->planes.size() != 0) {

#if 0
  CutOnPlanes(meshes,projector_centers.size());
#else
  // THIS IS THE EMPAC WAY?
  CutOnPlanes2(meshes);
  RemoveIfUnderWall(meshes);  
#endif
  //CutOnPlanes2(meshes);
  }

  (*ARGS->output) << "FINISHED CUT & REMOVE" << std::endl;


  // HORRIBLE UGLY TEXTURE DISTORTION IN EMPAC
#if 0

  MoveVertices(meshes);
  CollapseEdges(meshes,scene_radius/args->blending_subdivision*0.6);
  MoveVertices(meshes);
  FlipEdges(meshes);
  MoveVertices(meshes);
  SplitEdges(meshes,scene_radius/args->blending_subdivision);
  MoveVertices(meshes);
  FlipEdges(meshes);
  MoveVertices(meshes);
  CollapseEdges(meshes,scene_radius/args->blending_subdivision*0.2);
  MoveVertices(meshes);
  FlipEdges(meshes);
  MoveVertices(meshes);
  MoveVertices(meshes);
  MoveVertices(meshes);
  MoveVertices(meshes);


#else
  FlipEdges(meshes);
  SplitEdges(meshes,scene_radius/args->blending_subdivision);
  FlipEdges(meshes);
  SplitEdges(meshes,scene_radius/args->blending_subdivision);
  FlipEdges(meshes);
  SplitEdges(meshes,scene_radius/args->blending_subdivision);

#endif



  meshes->getMesh()->RecomputeStats();

#if 0
  AssignFakeMaterials(meshes,projector_centers);
#else 
  // THIS IS THE EMPAC WAY?
  AssignFakeMaterials2(meshes,projector_centers);
#endif




  mesh->ComputeBlendDistanceFromOcclusion();
  mesh->ComputeBlendWeights();
  
  (*ARGS->output) << "done cutting " << meshes->getMesh()->numTriangles() << " triangles" << std::endl;
}


void ReMesh::RemoveIfUnderWall(MeshManager *meshes) {

  //  return;

  Mesh *mesh = meshes->getMesh();
  Walls *walls = meshes->getWalls();

  int num_walls = walls->numWalls();

  std::vector<int> to_delete;

  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    if (!e->isATriangle()) continue;
    
    
    std::string material_name = e->getRealMaterialName();
    if (material_name.substr(0,5) != "floor") continue;
    //(*ARGS->output) << material_name << std::endl;
    
    Triangle *t = (Triangle*)e;
    
    Vec3f centroid;
    t->computeCentroid(centroid);

    //(*ARGS->output) << *t << std::endl;
    
    for (int i = 0; i < num_walls; i++) {

      BasicWall *wall = walls->getWall(i);
      if (wall->PointInside(centroid) && centroid.y() < 0.00001) {
	//(*ARGS->output) << " should delete " << e->getID() << std::endl;
	to_delete.push_back(e->getID());
      }

    }
    

  }  

  int bad_count = 0;
  for (unsigned int i = 0; i < to_delete.size(); i++) {

    Element *e = Element::GetElement(to_delete[i]);
    if (e == NULL) {  
      // needed to add this for army, it was working before...  
      // not sure what changed :(
      //(*ARGS->output) << "whoops can't delete " << std::endl; 
      bad_count++;
      continue; 
    }


    //(*ARGS->output) << " now delete " << e->getID() << std::endl;

    //(*ARGS->output) << "remove " << i << " " << *e << std::endl;
    mesh->removeElement(e);
  }
  if (bad_count > 0) {
    (*ARGS->output) << "REMOVE UNDER WALL SOME BUGS... " << bad_count << std::endl;
  }
  
}



void ReMesh::ConstructPlanes(MeshManager *meshes, std::vector<Vec3f> &projector_centers) {
  double scene_radius = meshes->getWalls()->getSceneRadius();
  ArgParser *args = meshes->args;
  int num_proj = args->projectors.size();
  for (int i = 0; i < num_proj; i++) {
    double x,y,z;
    meshes->args->projectors[i].getCenterReplacement(x,y,z);
    Vec3f proj_center = Vec3f(x,y,z);
    projector_centers.push_back(proj_center);
    for (int k = 0; k < 4; k++) {
      const GLdouble* near_a = meshes->args->projectors[i].getNearPlaneVertexReplacement(k);
      const GLdouble* near_b = meshes->args->projectors[i].getNearPlaneVertexReplacement((k+1)%4);
      assert (near_a != NULL);
      assert (near_b != NULL);
      CutPlane tmp;
      tmp.quad[0] = Vec3f(near_a[0],near_a[1],near_a[2]);
      tmp.quad[1] = Vec3f(near_b[0],near_b[1],near_b[2]);
      tmp.quad[2] = proj_center + 5*scene_radius*((tmp.quad[1]-proj_center).Normalize());
      tmp.quad[3] = proj_center + 5*scene_radius*((tmp.quad[0]-proj_center).Normalize());
      computeNormal(tmp.quad[0],tmp.quad[1],tmp.quad[2],tmp.normal);
      tmp.which_projector = i;
      tmp.projector_center = proj_center;
      tmp.d = -tmp.normal.Dot3(tmp.quad[0]);
#if CHECK_PROJECTOR_VIEW_FRUSTUM
      meshes->planes.push_back(tmp);
#endif
    }

    // loop through all the edges in the model looking for silhouette edges
    Walls* walls = meshes->getWalls();
    for (int j = 0; j < walls->numWalls(); j++) {
      BasicWall *w = walls->getWall(j);
      MakePlanes(*w,projector_centers[i],i,meshes);
    }

  }
}


// this just cuts each wall straight down...
void ReMesh::ConstructPlanes2(MeshManager *meshes, std::vector<Vec3f> &projector_centers) {
  //double scene_radius = meshes->getWalls()->getSceneRadius();
  ArgParser *args = meshes->args;
  int num_proj = args->projectors.size();
  for (int i = 0; i < num_proj; i++) {
    double x,y,z;
    meshes->args->projectors[i].getCenterReplacement(x,y,z);
    Vec3f proj_center = Vec3f(x,y,z);
    projector_centers.push_back(proj_center);
  }

  // loop through all the edges in the model looking for silhouette edges
  Walls* walls = meshes->getWalls();
  for (int j = 0; j < walls->numWalls(); j++) {
    BasicWall *w = walls->getWall(j);
    int good_verts = w->numGoodVerts();
    for (int k = 0; k < good_verts; k++) {
      Vec3f A = w->getGoodVert(k);
      Vec3f B = w->getGoodVert((k+1)%good_verts);
      Vec3f C = (A+B)*0.5;
      CutPlane tmp;
      tmp.quad[0] = A + Vec3f(0,1,0);
      tmp.quad[1] = B + Vec3f(0,1,0);
      tmp.quad[2] = B + Vec3f(0,-1,0);
      tmp.quad[3] = A + Vec3f(0,-1,0);
      computeNormal(tmp.quad[0],tmp.quad[1],tmp.quad[2],tmp.normal);
      tmp.which_projector = 0;
      tmp.projector_center = C + Vec3f(0,2,0);
      tmp.d = -tmp.normal.Dot3(tmp.quad[0]);
      meshes->planes.push_back(tmp);
    }
  }
}


bool QuadTriIntersect(Triangle *t, Vec3f quad[4]) {
  Vec3f tri[3];
  Mesh *mesh = t->getMesh();
  tri[0] = mesh->getVertex((*t)[0])->get();
  tri[1] = mesh->getVertex((*t)[1])->get();
  tri[2] = mesh->getVertex((*t)[2])->get();
  for (int i = 0; i < 4; i++) {
    if (Triangle::IntersectsSegment(tri[0],tri[1],tri[2],quad[i],quad[(i+1)%4])) return true;
  }
  for (int i = 0; i < 3; i++) {
    if (Triangle::IntersectsSegment(quad[0],quad[1],quad[2],tri[i],tri[(i+1)%3])) return true;
    if (Triangle::IntersectsSegment(quad[0],quad[2],quad[3],tri[i],tri[(i+1)%3])) return true;
  }
  return false;
}


bool SegmentTriIntersect(Triangle *t, const Vec3f &A, const Vec3f &B) {
  Vec3f tri[3];
  Mesh *mesh = t->getMesh();
  tri[0] = mesh->getVertex((*t)[0])->get();
  tri[1] = mesh->getVertex((*t)[1])->get();
  tri[2] = mesh->getVertex((*t)[2])->get();

  if (DistanceBetweenPointAndTriangle(A,tri[0],tri[1],tri[2]) < 0.000001) return true;
  if (DistanceBetweenPointAndTriangle(B,tri[0],tri[1],tri[2]) < 0.000001) return true;

  Vec3f intersect;
  if (SegmentsIntersect(A,B,tri[0],tri[1],intersect)) return true;
  if (SegmentsIntersect(A,B,tri[1],tri[2],intersect)) return true;
  if (SegmentsIntersect(A,B,tri[2],tri[0],intersect)) return true;

  return false;
}


struct CutEdge {
  CutEdge(int a, int b, double l, double f) : vert_a(a),vert_b(b),length(l),frac(f) {}
  int vert_a;
  int vert_b;
  double length;
  double frac;
};

bool operator<(const CutEdge &a, const CutEdge &b) {
  double a_min = min2(a.frac,1-a.frac)*a.length;
  double b_min = min2(b.frac,1-b.frac)*b.length;
  if (a_min > b_min) return true;
  //if (a.length > b.length) return true;
  return false;
}

/*
void ReMesh::CutOnPlanes(MeshManager *meshes,int num_projectors) {
  Mesh *mesh = meshes->getMesh();
  int num_planes = meshes->planes.size();
  //  (*ARGS->output) << num_planes << " planes to cut on " << std::endl;
  std::vector<ElementID> *ids = new std::vector<ElementID>();
  std::vector<ElementID> *recently_added = new std::vector<ElementID>();  

  // do one projector at a time!
  for (int p = 0; p < num_projectors; p++) {
    //   if (p != 0) continue;
    ids->clear();
    assert (recently_added->size() == 0);
    
    // put all element ids in vector
    for (elementshashtype::const_iterator foo = mesh->getElements().begin();
	 foo != mesh->getElements().end();
	 foo++) {
      Element *e = foo->second;
      ids->push_back(e->getID());
    }

    while (1) {
      std::set<CutEdge> cut_edges;
      int num_splits = 0;
      assert (recently_added->size() == 0);
      for (int k = 0; k < num_planes; k++) {
	if (meshes->planes[k].which_projector != p) continue;
	//if (meshes->args->num_planes_to_cut >=0 && k >= meshes->args->num_planes_to_cut) break;
	Vec3f projector_center = meshes->planes[k].projector_center;
	Vec3f p_normal = meshes->planes[k].normal;
	double d = meshes->planes[k].d;

	// go through all the elements, and try to split some of them!
	unsigned int num_ids = ids->size();
	for (unsigned int i = 0; i < num_ids; i++) {
	  Element *e = Element::GetElement((*ids)[i]);
	  if (e == NULL) continue;
	  if (!e->getMaterialPtr()->isProjection()) continue;
	  assert (e->isATriangle());
	  Vec3f normal; e->computeNormal(normal);
	  Vec3f centroid;	e->computeCentroid(centroid);
	  centroid -= projector_center;
	  centroid.Normalize();
	  if (centroid.Dot3(normal) > 0) continue;
	  bool flag = false;
	  int num_verts = 3; 
	  //	if (!QuadTriIntersect((Triangle*)e,meshes->planes[k].quad)) continue;
	  for (int j = 0; j < num_verts; j++) { 
	    Vec3f a = mesh->getVertex((*e)[j])->get();
	    Vec3f b = mesh->getVertex((*e)[(j+1)%num_verts])->get();
	    double length = (a-b).Length();
	    double side_a = a.Dot3(p_normal) + d;
	    double side_b = b.Dot3(p_normal) + d;
	    //if (!Quad::IntersectsSegment(npa,npb,npc,npd,proj_direction,query,projector)) {
	    if (!Quad::IntersectsSegment(meshes->planes[k].quad[0],
					 meshes->planes[k].quad[1], 
					 meshes->planes[k].quad[2],
					 meshes->planes[k].quad[3],normal,a,b)) { continue; }
	    //if (meshes->planes[k].quad.IntersectsSegment(a,b)) { continue; }
	    if ((side_a < -0.00001 && side_b > 0.00001) || (side_b < -0.00001 && side_a > 0.00001)) {
	      double fraction = fabs(side_a/(side_a-side_b));
	      Vec3f pos = (1-fraction)*a + fraction*b;
	      double side = pos.Dot3(p_normal) + d;
	      assert (fabs(side) < 0.00001);
	      std::vector<ElementID> removed;
	      cut_edges.insert(CutEdge((*e)[j],(*e)[(j+1)%e->numVertices()],length,fraction));
	      flag = true;
	      break;
	    }
	  }
	}
      }

      int num_cut_edges = cut_edges.size();      
      (*ARGS->output) << "cut edges " << num_cut_edges; 
      for (std::set<CutEdge>::iterator iter = cut_edges.begin(); iter != cut_edges.end(); iter++) { 
	std::vector<ElementID> removed;
	const CutEdge &c = *iter;
	bool split = SplitEdge(mesh,c.vert_a,c.vert_b,c.frac,removed,*recently_added);
	//(*ARGS->output) << "cut_edges " << c << " " << cut_edges[c].length << " " << cut_edges[c].frac << " " << min2(cut_edges[c].frac,1-cut_edges[c].frac)*cut_edges[c].length << std::endl;
	if (split) num_splits ++;
      }
      cut_edges.clear();
    
      (*ARGS->output) << "  num splits " << num_splits;
      (*ARGS->output) << "  num elements " << mesh->getElements().size() << std::endl;
      if (num_splits == 0) break;
      std::vector<ElementID> *tmp;
      tmp = ids;
      ids = recently_added;
      recently_added = tmp;
      recently_added->clear();
    }
  }
}
*/


void ReMesh::CutOnPlanes2(MeshManager *meshes) {
  //  (*ARGS->output) << "CUTONPLANES" << std::endl;

  if (meshes->args->no_blending_hack) {
    assert (0);
    (*ARGS->output) << "NO BLENDING HACK " << std::endl;
    return;
  }

  Mesh *mesh = meshes->getMesh();
  int num_planes = meshes->planes.size();
  //(*ARGS->output) << "before cutting num elements " << mesh->getElements().size() << std::endl;
  (*ARGS->output) << num_planes << " planes to cut on " << std::endl;

  if (!meshes->args->no_blending_hack) {
    int dimx = 50;
    int dimy = 10;
    int dimz = 50;
    Element::MakeAccelerationGrid(dimx,dimy,dimz);  
  }


  std::set<ElementID> *ids = new std::set<ElementID>();
  std::set<ElementID> *recently_added = new std::set<ElementID>();  

  for (int p = 0; p < num_planes; p++) {

    Vec3f A = (meshes->planes[p].quad[0] + meshes->planes[p].quad[3]) * 0.5;
    Vec3f B = (meshes->planes[p].quad[1] + meshes->planes[p].quad[2]) * 0.5;

    //(*ARGS->output) << "p " << A.x() << " " << A.y() << " " << A.z() << " " << B.x() << " " << B.y() << " " << B.z() << std::endl;

    ids->clear(); 
    recently_added->clear(); 


    if (meshes->args->no_blending_hack) {
      for (elementshashtype::const_iterator foo = mesh->getElements().begin();
	   foo != mesh->getElements().end();
	   foo++) {
	Element *e = foo->second;
	assert (e != NULL);
	ids->insert(e->getID());
      }
    } else {
      Vec3f minx(min2(A.x(),B.x()),
		 min2(A.y(),B.y()),
		 min2(A.z(),B.z()));
      Vec3f maxx(max2(A.x(),B.x()),
		 max2(A.y(),B.y()),
		 max2(A.z(),B.z()));
      Element::GetElementsInBB(minx,maxx,*ids);
    }
    while (1) {
      int num_splits = 0;
      assert (recently_added->size() == 0);
      
      // go through all the elements, and try to split some of them!
      for (std::set<ElementID>::const_iterator iter = ids->begin(); iter != ids->end(); iter++) {
	Element *e = Element::GetElement(*iter);
	if (e == NULL) continue;
	//(*ARGS->output) << "considering " << *iter << std::endl;
	
	assert (e->isATriangle());
	int num_verts = 3;
	bool flag = false;
	std::set<ElementID> removed;
	
	if (!flag) {
	  if (((Triangle*)e)->PointInside(A)) {
	    if (TriSect(mesh,((Triangle*)e),A,removed,*recently_added)) {
	      //(*ARGS->output) << "TRISECT1 " << *iter << std::endl;
	      num_splits++;
	      flag = true;
	    }
	  } 
	}
	
	if (!flag) {
	  if (((Triangle*)e)->PointInside(B)) {
	    if (TriSect(mesh,((Triangle*)e),B,removed,*recently_added)) {
	      //(*ARGS->output) << "TRISECT2 " << *iter << std::endl;
	      num_splits++;
	      flag = true;
	    }
	  }
	}

	for (int j = 0; j < num_verts; j++) { 
	  if (flag) break;
	  Vec3f a = mesh->getVertex((*e)[j])->get();
	  Vec3f b = mesh->getVertex((*e)[(j+1)%num_verts])->get();
	  Vec3f intersection_segment;
	  Vec3f vec_segment = b-a; vec_segment.Normalize(); if (vec_segment.Length() < 0.9) continue;
	  bool test_segment = SegmentsIntersect(A,B,a,b,intersection_segment);
	  if (test_segment)  {
	    double side_a = (a-intersection_segment).Length();
	    double side_b = (b-intersection_segment).Length();
	    double length_ab = (a-b).Length();
	    double fraction = side_a/length_ab;
	    if (side_a + side_b > length_ab*1.01) continue;
	    if (fraction > 0.01 && fraction < 0.99) {
	      bool split = SplitEdge(mesh,(*e)[j],(*e)[(j+1)%e->numVertices()],fraction,removed,*recently_added);
	      if (split) { 
		//(*ARGS->output) << "TRISECT " << *iter << std::endl;
		num_splits++; 
		flag = true; 
	      }
	    }
	  } 
	}

      }

      if (num_splits == 0) break;

      std::set<ElementID> *tmp;
      tmp = ids;
      ids = recently_added;
      recently_added = tmp;
      recently_added->clear();
    }
  }

  delete recently_added;
  delete ids;

  if (!meshes->args->no_blending_hack) {
    Element::DestroyAccelerationGrid();
  }

  (*ARGS->output) << "done cutting num elements " << mesh->getElements().size() << std::endl;
}


void ReMesh::AssignFakeMaterials(MeshManager *meshes, std::vector<Vec3f> &projector_centers) {
  Mesh *mesh = meshes->getMesh();
  // adjust material settings for each...
  std::vector<ElementID> ids;
  int num_walls = meshes->getWalls()->numWalls();
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    
    if (e->isALine()) continue;
    if (!e->getMaterialPtr()->isProjection()) continue;
    Vec3f normal; e->computeNormal(normal);

    for (unsigned int i = 0; i < projector_centers.size(); i++) {
      Vec3f projector = projector_centers[i];
      //Vec3f to_projector = projector - centroid;
      const GLdouble* near_a = meshes->args->projectors[i].getNearPlaneVertexReplacement(0);
      const GLdouble* near_b = meshes->args->projectors[i].getNearPlaneVertexReplacement(1);
      const GLdouble* near_c = meshes->args->projectors[i].getNearPlaneVertexReplacement(2);
      const GLdouble* near_d = meshes->args->projectors[i].getNearPlaneVertexReplacement(3);
      const Vec3f& proj_direction = meshes->args->projectors[i].getProjDirectionReplacement();
      Vec3f npa = Vec3f(near_a[0],near_a[1],near_a[2]);
      Vec3f npb = Vec3f(near_b[0],near_b[1],near_b[2]);
      Vec3f npc = Vec3f(near_c[0],near_c[1],near_c[2]);
      Vec3f npd = Vec3f(near_d[0],near_d[1],near_d[2]);
      
      Vec3f np_center = (npa+npb+npc+npd)*0.25;
      Vec3f to_projector = projector - np_center;
      
      to_projector.Normalize();
      
      bool visible = true;
      Vec3f A = mesh->getVertex((*e)[0])->get();
      Vec3f B = mesh->getVertex((*e)[1])->get();
      Vec3f C = mesh->getVertex((*e)[2])->get();

      for (int k = 0; k < 4; k++) {
	Vec3f query;
	if (k == 0)
	  query = 0.33*A + 0.33*B + 0.34*C;
	else if (k == 1) {
	  query = 0.9*A + 0.05*B + 0.05*C;
	} else if (k == 2) {
	  query = 0.05*A + 0.9*B + 0.05*C;
	} else {
	  assert (k == 3);
	  query = 0.05*A + 0.05*B + 0.9*C;
	}
	
	query += 0.001*to_projector;  // advance the ray slightly towards the projector
	if (to_projector.Dot3(normal) < 0.0) {
	  visible = false;
	} else {
	  if (!Quad::IntersectsSegment(npa,npb,npc,npd,proj_direction,query,projector)) {
	    visible = false;
	  } 
	  else {
	    for (int j = 0; j < num_walls; j++) {
	      if (meshes->getWalls()->SegmentIntersectsWall(j,query,projector)) {
		visible = false;
		break;
	      }
	    }
	  }
	}
	if (visible == false) break;
      }

      if (visible)
	e->setSidedness(i,'a');
      else
	e->setSidedness(i,'b');
    }
  }
  mesh->RecomputeStats();
}


bool RayFromPointReachesProjector(const OpenGLProjector &p, Vec3f pt, MeshManager *meshes,AccelerationGrid<int,int>&quad_grid,std::vector<QuadThing>&occlusions) {

  //(*ARGS->output) << "-----------------------" << std::endl;

  Vec3f projector_center = p.getVec3fCenterReplacement();
  Vec3f proj_direction = -p.getProjDirectionReplacement();
  Vec3f npa = p.getNearPlaneVec3fReplacement(0);
  Vec3f npb = p.getNearPlaneVec3fReplacement(1);
  Vec3f npc = p.getNearPlaneVec3fReplacement(2);
  Vec3f npd = p.getNearPlaneVec3fReplacement(3);
  //Vec3f query = pt + 0.001*proj_direction; // advance the ray slightly towards the projector
  Vec3f query = pt + 0.005*proj_direction; // advance the ray slightly towards the projector

  //bool in_viewfield = Quad::IntersectsSegment(npa,npb,npc,npd,proj_direction,query,projector_center);
  //  bool in_viewfield2 = Quad::IntersectsSegment(npa,npd,npc,npb,-proj_direction,query,projector_center);
  //  assert (in_viewfield2 == in_viewfield);

  // CHECK TO SEE IF POLY IS IN VIEWFIELD OF PROJECTOR
  if (!Quad::IntersectsSegment(npa,npb,npc,npd,proj_direction,query,projector_center)) {
    return false;
  } 

  bool old_way_answer = true;
  bool new_way_answer = true;
  //bool new_way_answer2 = true;

#if 1
  // OLD WAY
  int num_walls = meshes->getWalls()->numWalls();      
  for (int j = 0; j < num_walls; j++) {
    if (meshes->getWalls()->SegmentIntersectsWall(j,query,projector_center)) {
      old_way_answer = false;
      return false;
    }
  }
  return true;
#endif


  //return old_way_answer;

  // assuming an 8 foot tall wall...
  assert (fabs(proj_direction.Length()-1) < 0.0001);
  assert (proj_direction.y() > 0.01);
  assert (query.y() > -0.01);
  double vertical_increase = ((8 * 12 + 2.5) * INCH_IN_METERS - query.y()) / proj_direction.y();
  Vec3f query_end = query + vertical_increase * proj_direction;
  assert (fabs(query_end.y() -(8*12 + 2.5) *INCH_IN_METERS) < 0.0001);

  /*
  query.Print("QUERY");
  query_end.Print("QUERY_END");
  projector_center.Print("projector center");
  */

  //query_end = projector_center;

  int Ai,Aj,Ak,Bi,Bj,Bk,Ci,Cj,Ck;
  quad_grid.whichCell(query,Ai,Aj,Ak);
  quad_grid.whichCell(query_end,Bi,Bj,Bk);
  quad_grid.whichCell(projector_center,Ci,Cj,Ck);
  int min_i = min2(Ai,Bi);
  int min_j = min2(Aj,Bj);
  int min_k = min2(Ak,Bk);
  int max_i = max2(Ai,Bi);
  int max_j = max2(Aj,Bj);
  int max_k = max2(Ak,Bk);
  std::set<int> quads_to_check;
  for (int i = min_i; i <= max_i; i++) {
    for (int j = min_j; j <= max_j; j++) {
      for (int k = min_k; k <= max_k; k++) {
	const std::set<int>& cell = quad_grid.cells[quad_grid.getIndex(i,j,k)];
	for (std::set<int>::const_iterator iter = cell.begin(); iter != cell.end(); iter++) {
	  //std::pair<std::set<int>::iterator,bool> answer = quads_to_check.insert(*iter);
	  quads_to_check.insert(*iter);
	}
      } 
    }
  }  
  //(*ARGS->output) << "quads to check " << quads_to_check.size() << " vs " << occlusions.size() << std::endl;
  for (std::set<int>::iterator iter = quads_to_check.begin();
       iter != quads_to_check.end(); iter++) {
    int index = *iter;
    assert (index >= 0 && index < (int)occlusions.size());
    QuadThing &thing = occlusions[index];
    if (Quad::IntersectsSegment(thing.pts[0],thing.pts[1],thing.pts[2],thing.pts[3],thing.normal,query,projector_center)) {
      //(*ARGS->output) << "blocked A " << index << std::endl;
      //assert (old_way_answer == false);
      new_way_answer = false; 
      return false;
      //return false;
    }
  }
  //#endif

#if 0
  // SECOND CHECK!
  min_i = min2(Ai,Ci);
  min_j = min2(Aj,Cj);
  min_k = min2(Ak,Ck);
  max_i = max2(Ai,Ci);
  max_j = max2(Aj,Cj);
  max_k = max2(Ak,Ck);
  quads_to_check.clear();
  for (int i = min_i; i <= max_i; i++) {
    for (int j = min_j; j <= max_j; j++) {
      for (int k = min_k; k <= max_k; k++) {
	const std::set<int>& cell = quad_grid.cells[quad_grid.getIndex(i,j,k)];
	for (std::set<int>::const_iterator iter = cell.begin(); iter != cell.end(); iter++) {
	  std::pair<std::set<int>::iterator,bool> answer = quads_to_check.insert(*iter);
	}
      } 
    }
  }  
  for (std::set<int>::iterator iter = quads_to_check.begin();
       iter != quads_to_check.end(); iter++) {
    int index = *iter;
    assert (index >= 0 && index < (int)occlusions.size());
    QuadThing &thing = occlusions[index];
    if (Quad::IntersectsSegment(thing.pts[0],thing.pts[1],thing.pts[2],thing.pts[3],thing.normal,query,projector_center)) {
      (*ARGS->output) << "blocked B " << index << std::endl;
      //assert (old_way_answer == false);
      new_way_answer2 = false; 
      //return false;
    }
  }

  //assert (old_way_answer == true);
  //  return true;

  //(*ARGS->output) << old_way_answer << new_way_answer << std::endl;
  if (old_way_answer != new_way_answer) { 
    assert (old_way_answer == new_way_answer2);
    (*ARGS->output) << std::endl << "CRAP ";
    (*ARGS->output) << old_way_answer << new_way_answer << " "; 
    pt.Print("pt");
    query.Print           ("QUERY           ");
    query_end.Print       ("QUERY_END       ");
    projector_center.Print("projector center");
    (*ARGS->output) << " query      " << Ai << " " << Aj << " " << Ak << std::endl;
    (*ARGS->output) << " query end  " << Bi << " " << Bj << " " << Bk << std::endl;
    (*ARGS->output) << "proj center " << Ci << " " << Cj << " " << Ck << std::endl;
    (*ARGS->output) << "min " << min_i << " " << min_j << " " << min_k << std::endl;
    (*ARGS->output) << "max " << max_i << " " << max_j << " " << max_k << std::endl;
  }
  //assert (old_way_answer == new_way_answer);
  return old_way_answer;
#endif

}


void CollectOcclusions(std::vector<QuadThing> &occlusions, MeshManager *meshes) {
  int num_walls = meshes->getWalls()->numWalls();      
  for (int i = 0; i < num_walls; i++) {
    BasicWall *w = meshes->getWalls()->getWall(i);
    Vec3f h = Vec3f(0,w->getMaxHeight(),0);
    //Vec3f b = Vec3f(0,w->getBottomEdge(),0);
    Vec3f b = Vec3f(0,0,0);
    unsigned int num_quads = w->numConvexQuads();
    for (unsigned int j = 0; j < num_quads; j++) {
      const ConvexQuad& q = w->getConvexQuad(j);
      //(*ARGS->output) << std::endl << std::endl;
#if 1
      occlusions.push_back(QuadThing(q[1]+h,q[0]+h,q[3]+h,q[2]+h,Vec3f(0,1,0))); // top
      occlusions.push_back(QuadThing(q[1]+b,q[0]+b,q[0]+h,q[1]+h,q.getNormal(0))); // front
      occlusions.push_back(QuadThing(q[3]+b,q[2]+b,q[2]+h,q[3]+h,q.getNormal(2))); //back 
#else
      /*
      (*ARGS->output) << "TEST" << std::endl;
      (q[3]+b).Print("q[3]+b");
      (q[2]+b).Print("q[2]+b");
      (q[2]+h).Print("q[2]+h");
      (q[3]+h).Print("q[3]+h");
      */
      Vec3f A = (q[2]+q[1])*0.5;
      Vec3f B = (q[3]+q[0])*0.5;

      //Vec3f A = (q[0]+q[3]) * 0.5;
      //Vec3f B = (q[1]+q[2]) * 0.5;
      
      Vec3f norm; computeNormal(B+b,A+b,A+h,norm);
      double dot = norm.Dot3(q.getNormal(2));
      assert (dot > 0.99);

      //occlusions.push_back(QuadThing(B+b,A+b,A+h,B+h,norm)); //q.getNormal(0))); // average
      occlusions.push_back(QuadThing(B+b,A+b,A+h,B+h,q.getNormal(0))); // average
#endif
      /*
      (B+b).Print("B+b");
      (A+b).Print("A+b");
      (A+h).Print("A+h");
      (B+h).Print("B+h");
      */
      if (j == 0) { 
	occlusions.push_back(QuadThing(q[0]+b,q[3]+b,q[3]+h,q[0]+h,q.getNormal(3))); // one side
      }
      if (j == num_quads-1) { 
	occlusions.push_back(QuadThing(q[2]+b,q[1]+b,q[1]+h,q[2]+h,q.getNormal(1))); // other side
      } 
    }
  }
}

void PlaceOcclusions(AccelerationGrid<int,int> &quad_grid, std::vector<QuadThing> &occlusions) {
  int num_occlusions = occlusions.size();
  for (int OCC = 0; OCC < num_occlusions; OCC++) {

    //(*ARGS->output) << "placing " << OCC << std::endl;

    QuadThing& q = occlusions[OCC];

    int Ai,Aj,Ak,Bi,Bj,Bk,Ci,Cj,Ck,Di,Dj,Dk;
    quad_grid.whichCell(q.pts[0],Ai,Aj,Ak);
    quad_grid.whichCell(q.pts[1],Bi,Bj,Bk);
    quad_grid.whichCell(q.pts[2],Ci,Cj,Ck);
    quad_grid.whichCell(q.pts[3],Di,Dj,Dk);

    int min_i = min4(Ai,Bi,Ci,Di);
    int min_j = min4(Aj,Bj,Cj,Dj);
    int min_k = min4(Ak,Bk,Ck,Dk);
    int max_i = max4(Ai,Bi,Ci,Di);  
    int max_j = max4(Aj,Bj,Cj,Dj);  
    int max_k = max4(Ak,Bk,Ck,Dk);
    //(*ARGS->output) << "min " << min_i << " " << min_j << " " << min_k << std::endl;
    //(*ARGS->output) << "max " << max_i << " " << max_j << " " << max_k << std::endl;

    // adjustments are probably a hack!!
    //(*ARGS->output) << "HACKED" << std::endl;
    min_i = max2(0,min_i-1);  
    min_j = max2(0,min_j-1);
    min_k = max2(0,min_k-1);
    max_i = min2(quad_grid.dx-1,max_i+1);
    max_j = min2(quad_grid.dy-1,max_j+1);
    max_k = min2(quad_grid.dz-1,max_k+1);
    //(*ARGS->output) << "  min " << min_i << " " << min_j << " " << min_k << std::endl;
    //(*ARGS->output) << "  max " << max_i << " " << max_j << " " << max_k << std::endl;

  
    /*    q.pts[0].Print("pts[0]");
    q.pts[1].Print("pts[1]");
    q.pts[2].Print("pts[2]");
    q.pts[3].Print("pts[3]");
    */

    for (int i = min_i; i <= max_i; i++) {
      for (int j = min_j; j <= max_j; j++) {
	for (int k = min_k; k <= max_k; k++) {
	  std::set<int>& cell = quad_grid.cells[quad_grid.getIndex(i,j,k)];
	  //quad_grid.printCell(i,j,k);
	  std::pair< std::set<int>::iterator,bool> answer = cell.insert(OCC);
	  assert (answer.second == true);
	}
      } 
    }
  }

  //quad_grid.PrintStats();
}


std::vector<QuadThing> GLOBAL_occlusions;

void ReMesh::AssignFakeMaterials2(MeshManager *meshes, std::vector<Vec3f> &projector_centers) {

  Mesh *mesh = meshes->getMesh();

  if (meshes->args->no_blending_hack) {
    mesh->RecomputeStats();
    return;
  }

  double scene_radius = meshes->getWalls()->getSceneRadius();
  int num_verts = mesh->numVertices();

  //  std::vector<QuadThing> occlusions;
  GLOBAL_occlusions.clear();
  CollectOcclusions(GLOBAL_occlusions,meshes);

  //(*ARGS->output) << "num occlusions " << occlusions.size() << std::endl;

#if 0
  int dx = 60;
  int dy = 2;
  int dz = 60;
#else
  int dx = 1;
  int dy = 1;
  int dz = 1;
#endif

  if (scene_radius < 15*12*INCH_IN_METERS) {
    // NOT IN EMPAC
    
  }

  AccelerationGrid<int,int> quad_grid(dx,dy,dz,
				      Vec3f(-scene_radius*3,-0.1,-scene_radius*3),
				      Vec3f(scene_radius*3,2*scene_radius,scene_radius*3));

  PlaceOcclusions(quad_grid,GLOBAL_occlusions);
  
  for (int v = 0; v < num_verts; v++) {
    Vertex *vert = mesh->getVertex(v);
    Vec3f pt = vert->get();
    for (unsigned int i = 0; i < projector_centers.size(); i++) {
      bool test = RayFromPointReachesProjector(meshes->args->projectors[i],pt,meshes,quad_grid,GLOBAL_occlusions);
      vert->setProjectorVisibility(i,test);
      //(*ARGS->output) << "total occ " << occlusions.size(); 
    }
  }

  // =======================================================================

  // adjust material settings for each...
  std::vector<ElementID> ids;
  //int num_walls = meshes->getWalls()->numWalls();
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    if (e->isALine()) continue;
    Vec3f centroid; 
    e->computeCentroid(centroid);      
    Vec3f normal;
    e->computeNormal(normal);

    /*
    if (normal.Dot3(Vec3f(0,1,0)) > 0.99 && fabs(centroid.y()< 0.001)) {
      Vec3f straight_up = centroid + Vec3f(0,scene_radius,0);
      for (int j = 0; j < num_walls; j++) {
	if (meshes->getWalls()->SegmentIntersectsWallTop(j,centroid,straight_up)) {
	  e->setMaterial("EXTRA_under_wall");
	}
      }
    }
    */
    if (!e->getMaterialPtr()->isProjection()) continue;

    Vertex *a = mesh->getVertex((*e)[0]);
    Vertex *b = mesh->getVertex((*e)[1]);
    Vertex *c = mesh->getVertex((*e)[2]);

    for (unsigned int i = 0; i < projector_centers.size(); i++) {
      Vec3f projector = projector_centers[i];
      const Vec3f& proj_direction = meshes->args->projectors[i].getProjDirectionReplacement();

      bool vis_a = a->getProjectorVisibility(i);
      bool vis_b = b->getProjectorVisibility(i);
      bool vis_c = c->getProjectorVisibility(i);

      //      bool vis_c = c->getProjectorVisibility(i);

      bool tri_vis = (proj_direction.Dot3(normal) < 0);
      
      bool visible = vis_a && vis_b && vis_c && tri_vis;
      //bool visible = vis_a && vis_b && vis_c;
      //bool visible = tri_vis;

      if (visible)
	e->setSidedness(i,'a');
      else
	e->setSidedness(i,'b');
    }
  }


  if (meshes->args->test_color_calibration) {
    (*ARGS->output) << "test_color_calibration" << std::endl;
    for (elementshashtype::const_iterator foo = mesh->getElements().begin();
	 foo != mesh->getElements().end();
	 foo++) {
      Element *e = foo->second;

      int start = rand() % projector_centers.size();
      bool found = false;
      for (unsigned int i = 0; i < projector_centers.size(); i++) {
	int v = (start + i) % projector_centers.size();

	char visible = e->getSidedness(v);
	if (visible == 'a') {
	  if (found == false) {
	    found = true;
	  } else {
	    e->setSidedness(v,'b');
	  }
	}

	//if (visible)
	//  e->setSidedness(i,'a');
	//else
	//e->setSidedness(i,'b');
	
      }
    }
  }

  mesh->RecomputeStats();
}



bool conservativeSidedness(std::string &common, const std::string &insert) {
  assert (common.size() == insert.size());
  bool answer = false;
  for (unsigned int i = 0; i < common.size(); i++) {
    if (insert[i] == 'b')
      common[i] = 'b';
    if (common[i] == 'a') answer = true;
  }
  return answer;
}

bool TestSidednessSet(Element *e, int changing_vert, std::string &common) {
  assert (e->isATriangle());
  //Mesh *mesh = e->getMesh();
  for (int i = 0; i < 3; i++) {
    if ((*e)[i] == changing_vert) continue;
    std::vector<Element*> elements;
    Collect::CollectElementsWithVertex(e,(*e)[i],elements);
    bool success = true; 
    for (unsigned int j = 0; j < elements.size(); j++) {
      Element *e2 = elements[j];
      if (!e2->getMaterialPtr()->isProjection()) continue;
      success &= conservativeSidedness(common,e2->getSidedness());     
    }
    if (!success) { /*(*ARGS->output) << " means black! " << std::endl;*/ return false; }
  }
  return true;
}

bool ReMesh::TryToCheat(MeshManager *meshes, Element *elem, int vert) {
  //Mesh *mesh = meshes->getMesh();
  bool same_sidedness = true;
  //bool same_real_material = true;
  std::vector<Element*> elements;

  // collect all the elements around that vertex
  Collect::CollectElementsWithVertex(elem,vert,elements);
  unsigned int j; 
  for (j = 0; j < elements.size(); j++) {
    Element *e2 = elements[j];
    if (!e2->getMaterialPtr()->isProjection()) continue;
    //if (!mesh->getMaterial(e2->getRealMaterial()).isProjection()) continue;
    //if (e2->getRealMaterial() != elem->getRealMaterial()) { same_real_material = false; }
    if (e2->getSidedness() != elem->getSidedness()) { same_sidedness = false; }
  }
  //if (!same_real_material) return false;
  // if they all have the same projector visibility, there is nothing to do...
  if (same_sidedness) return false;

  // determine a common sidedness that could be assigned around this vertex
  std::string common = elem->getSidedness();
  bool success = true;
  for (j = 0; j < elements.size(); j++) {
    Element *e2 = elements[j];
    if (!e2->getMaterialPtr()->isProjection()) continue;
    //if (!mesh->getMaterial(e2->getRealMaterial()).isProjection()) continue;
    success &= conservativeSidedness(common,e2->getSidedness());
  }
  if (!success) { return false; }

  // test to see if setting each element to that common sidedness is
  // ok (will not create black blending weights for some neighboring
  // vertex)
  for (j = 0; j < elements.size(); j++) {
    Element *e2 = elements[j];
    if (!e2->getMaterialPtr()->isProjection()) continue;
    //if (!mesh->getMaterial(e2->getRealMaterial()).isProjection()) continue;
    if (!TestSidednessSet(e2,vert,common)) return false;
  }

  for (j = 0; j < elements.size(); j++) {
    Element *e2 = elements[j];
    if (!e2->getMaterialPtr()->isProjection()) continue;
    //if (!mesh->getMaterial(e2->getRealMaterial()).isProjection()) continue;
    e2->setSidedness(common);
  }
  //(*ARGS->output) << "cheated!" << std::endl;
  return true;
}

void ReMesh::CheatFakeMaterials(MeshManager *meshes) {
  Mesh *mesh = meshes->getMesh();
  double area_tolerance = 0.03 * GLOBAL_DESIRED_LENGTH * GLOBAL_DESIRED_LENGTH;
  double length_tolerance = 0.1 * GLOBAL_DESIRED_LENGTH;
  double angle_tolerance = 10 * M_PI/180;
  elementshashtype::const_iterator foo;
  for (foo=mesh->getElements().begin(); foo!=mesh->getElements().end();foo++) {
    Element *elem = foo->second;
    assert (elem != NULL);
    if (!elem->getMaterialPtr()->isProjection()) continue;
    //if (!mesh->getMaterial(elem->getRealMaterial()).isProjection()) continue;
    if (!elem->isATriangle()) {
      assert (elem->isAQuad());
      (*ARGS->output) << "quad" << std::endl;
      continue;
    }
    assert (elem->isATriangle());
    bool success = false;
    
    for (int i = 0; i < 3; i++) {
      Vec3f verta = mesh->getVertex((*elem)[i])->get();
      Vec3f vertb = mesh->getVertex((*elem)[(i+1)%3])->get();
      double length = DistanceBetweenTwoPoints(verta,vertb);
      if (length < length_tolerance) {
	if (!success)
	  success = TryToCheat(meshes,elem,(*elem)[i]);
	if (!success)
	  success = TryToCheat(meshes,elem,(*elem)[(i+1)%3]);
      }
    }

    double smallest_angle = min3(elem->Angle(0),elem->Angle(1),elem->Angle(2));
    double area = elem->Area();
    if (smallest_angle < angle_tolerance || area < area_tolerance) {
      if (!success)
	success = TryToCheat(meshes,elem,(*elem)[0]);
      if (!success)
	success = TryToCheat(meshes,elem,(*elem)[1]);
      if (!success)
	success = TryToCheat(meshes,elem,(*elem)[2]);
      if (!success) {
	//(*ARGS->output) << "couldn't cheat" << std::endl;
      }
    }
  }
}
