#include <fstream>
#include "remesh.h"
#include "mesh.h"
#include "meshmanager.h"
#include "vertex.h"
#include "element.h"
#include "triangle.h"
#include "vectors.h"

void ReMesh::SubdivisionHelperEdgeVertex(Mesh *mesh, int a, int b, int m, int n, Vec3f &v) {

  Vec3f va = mesh->getVertex(a)->get();
  Vec3f vb = mesh->getVertex(b)->get();
  Vec3f vm = mesh->getVertex(m)->get();
  Vec3f vn = mesh->getVertex(n)->get();
  double x = (3*va.x() + 3*vb.x() + vm.x() + vn.x()) / 8.0;
  double y = (3*va.y() + 3*vb.y() + vm.y() + vn.y()) / 8.0;
  double z = (3*va.z() + 3*vb.z() + vm.z() + vn.z()) / 8.0;
  v = Vec3f(x,y,z);
}

void ReMesh::SubdivisionHelperAddEdgeVertices(Mesh *m) {

  // ----------------------------------------------------------------
  // NOTE: for the correspondence file, 
  // it is important that we use the same order as the save function
  // ----------------------------------------------------------------

  // collect the used_materials
  std::set<std::string> used_materials;
  for (elementshashtype::const_iterator foo = m->getElements().begin();
       foo != m->getElements().end();
       foo++) { 
    Element *e = foo->second;
    std::string matname = e->getRealMaterialName();
    used_materials.insert(matname);
  }

  // make a list of the triangles in the order they would be saved to file
  std::vector<int> old_triangles;
  for (std::set<std::string>::iterator mat_iter = used_materials.begin();
       mat_iter != used_materials.end(); mat_iter++) {
    const std::string &THIS_MAT = *mat_iter;
    for (elementshashtype::const_iterator foo = m->getElements().begin();
	 foo != m->getElements().end();
	 foo++) { 
      Element *e = foo->second;
      const std::string &matname = e->getRealMaterialName();
      if (matname != THIS_MAT) continue;
      assert (e->isATriangle());
      old_triangles.push_back(e->getID());
    }
  }

  // split each triangle into 4, remembering those elements
  std::vector<Element*> new_triangles(4*old_triangles.size(),NULL);
  for (unsigned int i = 0; i < old_triangles.size(); i++) {
    Element *e = Element::GetElement(old_triangles[i]);
    if (e == NULL) continue;
    assert (e->isATriangle());
    Triangle *t = (Triangle*)e;
    
    int a = (*t)[0];
    int b = (*t)[1];
    int c = (*t)[2];
    
    int ab = m->getChildVertex(a,b);
    int ac = m->getChildVertex(a,c);
    int bc = m->getChildVertex(b,c);
    
    if (ab == -1) { 
      Vec3f posab = (m->getVertex(a)->get()+m->getVertex(b)->get())*0.5; 
      ab = m->addVertex(posab,a,b,0,0);
    }
    if (ac == -1) { 
      Vec3f posac = (m->getVertex(a)->get()+m->getVertex(c)->get())*0.5; 
      ac = m->addVertex(posac,a,c,0,0);
    }
    if (bc == -1) { 
      Vec3f posbc = (m->getVertex(b)->get()+m->getVertex(c)->get())*0.5; 
      bc = m->addVertex(posbc,b,c,0,0);
    }
    
    assert (ab != -1 && ac != -1 && bc != -1);
    
    std::string mat = t->getFakeMaterial();
    
    m->removeElement(t);

    // the four new triangles
    new_triangles[i*4+0] = new Triangle(a,ab,ac,m,mat);
    new_triangles[i*4+1] = new Triangle(b,bc,ab,m,mat);
    new_triangles[i*4+2] = new Triangle(c,ac,bc,m,mat);
    new_triangles[i*4+3] = new Triangle(ab,bc,ac,m,mat);
    m->addElement(new_triangles[i*4+0]);
    m->addElement(new_triangles[i*4+1]);
    m->addElement(new_triangles[i*4+2]);
    m->addElement(new_triangles[i*4+3]);
  }


  // walk through the new mesh, in the order they will be saved to a file
  std::map<Element*,int> correspondences;
  int count = 0;
  for (std::set<std::string>::iterator mat_iter = used_materials.begin();
       mat_iter != used_materials.end(); mat_iter++) {
    const std::string &THIS_MAT = *mat_iter;
    for (elementshashtype::const_iterator foo = m->getElements().begin();
	 foo != m->getElements().end();
	 foo++) { 
      Element *e = foo->second;
      const std::string &matname = e->getRealMaterialName();
      if (matname != THIS_MAT) continue;
      assert (e->isATriangle());
      correspondences[e] = count;
      count++;
    }
  }

  assert (correspondences.size() == old_triangles.size()*4);
  std::ofstream ostr("correpondences.txt");
  for (unsigned int i = 0; i < old_triangles.size(); i++) {
    int a = correspondences[new_triangles[i*4+0]];
    int b = correspondences[new_triangles[i*4+1]];
    int c = correspondences[new_triangles[i*4+2]];
    int d = correspondences[new_triangles[i*4+3]];
    ostr << i << " : " << a << " " << b << " " << c << " " << d << std::endl;
  }


}

void ReMesh::LoopSubdivision(MeshManager *meshes) {

  std::cout << "subdivide only works for triangles now!" << std::endl;
  //return;
  //exit(0);

  Mesh *mesh = meshes->getMesh();

  std::cout << "not smoothing!" << std::endl;

  /*
    // NOT SMOOTHING

  // figure out the new vertex positions (but don't move them yet)
  int vcount = mesh->numVertices();
  int *ncount  = new int[vcount];
  int *ncount2 = new int[vcount];
  Vec3f *vpos  = new Vec3f[vcount];
  Vec3f *vpos2 = new Vec3f[vcount];

  for (int i = 0; i < vcount; i++) {
    vpos[i] = Vec3f(0,0,0);
    ncount[i] = 0;
    vpos2[i] = Vec3f(0,0,0);
    ncount2[i] = 0;
  }
  */

  /*
  for (elementshashtype::const_iterator foo = mesh->getElements().begin();
       foo != mesh->getElements().end();
       foo++) {
    Element *e = foo->second;
    //I/terator<Element*> *iter = mesh->getElements()->StartIteration();
    //whi/le (Element *t = (Element*)iter->GetNext()) {
    assert (e->isATriangle());
    Triangle *t = (Triangle*)e;
    for (int i = 0; i < 3; i++) {
      int v  = (*t)[i];
      int v1 = (*t)[(i+1)%3];
      assert (v <= vcount);
      assert (v1 <= vcount);
      vpos[v] += mesh->getVertex(v1)->get();
      ncount[v]++;

      // HACK

      if (0) {
	//if (t->getNeighbor(i) == NULL) {
        vpos2[v] += mesh->getVertex(v1)->get();
        vpos2[v1] += mesh->getVertex(v)->get();
        ncount2[v]++;
        ncount2[v1]++;
      }
    }
  }

  */

  //mesh->getElements()->EndIteration(iter);
  SubdivisionHelperAddEdgeVertices(mesh);

  /*
  // move the original vertices to new positions
  for (int i = 0; i < vcount; i++) {
    if (ncount2[i] == 0) {
      int n = ncount[i];
      if (n == 0) continue; // this is a vertex with no neighboring triangles, do nothing!
      double alpha = 0.375 + (0.25 * cos(2 * M_PI / double(n)));
      alpha = 0.625 - (alpha*alpha);
      Vec3f tmp = mesh->getVertex(i)->get();
      tmp *= (1 - alpha);
      vpos[i] *= alpha / double(n);
      vpos[i] += tmp;
      std::cout << " NOT DOING TEXTURE CORRECTLY FOR SUBDIVISION" << std::endl;
      mesh->getVertex(i)->set2(vpos[i],0,0); //.x(),vpos[i].y(),vpos[i].z());
    } else {
      assert (ncount2[i] == 2);
      Vec3f tmp = mesh->getVertex(i)->get();
      tmp *= 0.5f;
      vpos2[i] *= 0.25f;
      vpos2[i] += tmp;
      std::cout << " NOT DOING TEXTURE CORRECTLY FOR SUBDIVISION" << std::endl;
      mesh->getVertex(i)->set2(vpos2[i],0,0); //.x(),vpos2[i].y(),vpos2[i].z());
    }
  }

  delete [] vpos;
  delete [] vpos2;
  delete [] ncount;
  delete [] ncount2;
  */

  mesh->RecomputeStats();
}






