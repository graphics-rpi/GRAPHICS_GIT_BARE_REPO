#include "accelerationgrid.h"
#include "element.h"
#include "triangle.h"
#include "mesh.h"
#include "vertex.h"
#include "boundingbox.h"

void Element::MakeAccelerationGrid(int dx, int dy, int dz) {

  assert (acceleration_grid == NULL);

  BoundingBox bbox(Vec3f(0,0,0),Vec3f(0,0,0));

  Mesh *mesh = NULL;

  for (elementshashtype::const_iterator foo = all_elements.begin(); foo != all_elements.end(); foo++) {
    Element *e = foo->second;

    if (e->isALine()) continue;

    assert (e->isATriangle());
    mesh = e->getMesh();
    Vec3f A = mesh->getVertex((*e)[0])->get();
    Vec3f B = mesh->getVertex((*e)[1])->get();
    Vec3f C = mesh->getVertex((*e)[2])->get();
    bbox.Extend(A);
    bbox.Extend(B);
    bbox.Extend(C);
  }

  Vec3f min,max;
  bbox.Get(min,max);

  Vec3f diff = max-min;
  min -= 0.001*diff;
  max += 0.001*diff;

  acceleration_grid = new AccelerationGrid<Element*,ElementID>(dx,dy,dz,min,max);  
  acceleration_grid->mesh = mesh;
    
  for (elementshashtype::const_iterator foo = all_elements.begin(); foo != all_elements.end(); foo++) {
    Element *e = foo->second;
    if (e->isALine()) continue;
    assert (e->isATriangle());
    acceleration_grid->AddElement(e);
  }

}

void Element::DestroyAccelerationGrid() {

  assert (acceleration_grid != NULL);
  delete acceleration_grid;
  acceleration_grid = NULL;

}



void Element::GetElementsInBB(Vec3f &minx, Vec3f &maxx, std::set<ElementID> &ids) {
  if (Element::acceleration_grid == NULL) {
    for (elementshashtype::const_iterator foo = all_elements.begin();
	 foo != all_elements.end(); foo++) {
      Element *e = foo->second;
      assert (e->isATriangle());
      Vec3f normal; e->computeNormal(normal);
      Mesh *mesh = e->getMesh();
      Vec3f tmp = mesh->getVertex((*e)[0])->get();
      if (fabs(tmp.y()) > 0.00001) continue;
      if (normal.Dot3(Vec3f(0,1,0)) < 0.9) continue;
      ids.insert(e->getID());
    }
  }

  else {
    assert (Element::acceleration_grid != NULL);

    int min_i,min_j,min_k,max_i,max_j,max_k;
    Element::acceleration_grid->whichCell(minx,min_i,min_j,min_k);
    Element::acceleration_grid->whichCell(maxx,max_i,max_j,max_k);
    assert (min_i >=0 && max_i >=0 && min_j >=0 && max_j >= 0 && min_k >=0 && max_k >= 0);

    for (int i = min_i; i <= max_i; i++) {
      for (int j = min_j; j <= max_j; j++) {
	for (int k = min_k; k <= max_k; k++) {
	  const std::set<ElementID>& cell = Element::acceleration_grid->cells[Element::acceleration_grid->getIndex(i,j,k)];
	  for (std::set<ElementID>::const_iterator iter = cell.begin(); iter != cell.end(); iter++) {

	    //std::pair<std::set<ElementID>::iterator,bool> answer = ids.insert(*iter);
	    ids.insert(*iter);

	  }
	}
      } 
    }
  }

}



template <> 
void AccelerationGrid<Element*,ElementID>::AddElement(Element* e) {

  assert (e == Element::GetElement(e->getID()));

  assert (e->isATriangle());
  Triangle *t = (Triangle*)e;

  Vec3f A = mesh->getVertex((*t)[0])->get();
  Vec3f B = mesh->getVertex((*t)[1])->get();
  Vec3f C = mesh->getVertex((*t)[2])->get();
  
  int Ai,Aj,Ak,Bi,Bj,Bk,Ci,Cj,Ck;
  whichCell(A,Ai,Aj,Ak);
  whichCell(B,Bi,Bj,Bk);
  whichCell(C,Ci,Cj,Ck);
  int min_i = min3(Ai,Bi,Ci);
  int min_j = min3(Aj,Bj,Cj);
  int min_k = min3(Ak,Bk,Ck);
  int max_i = max3(Ai,Bi,Ci);
  int max_j = max3(Aj,Bj,Cj);
  int max_k = max3(Ak,Bk,Ck);
  ElementID elementID = t->getID();
  assert (min_i >=0 && max_i >=0 && min_j >=0 && max_j >= 0 && min_k >=0 && max_k >= 0);

  e->acceleration_bb[0] = min_i;
  e->acceleration_bb[1] = min_j;
  e->acceleration_bb[2] = min_k;
  e->acceleration_bb[3] = max_i;
  e->acceleration_bb[4] = max_j;
  e->acceleration_bb[5] = max_k;
  
  for (int i = min_i; i <= max_i; i++) {
    for (int j = min_j; j <= max_j; j++) {
      for (int k = min_k; k <= max_k; k++) {
	std::set<ElementID>& cell = cells[getIndex(i,j,k)];
	std::pair< std::set<ElementID>::iterator,bool> answer = cell.insert(elementID);
	assert (answer.second == true);
      }
    } 
  }
}



template <>
void AccelerationGrid<Element*,ElementID>::RemoveElement(Element* e) {

  assert (e != NULL);
  assert (e == Element::GetElement(e->getID()));

  int min_i = e->acceleration_bb[0];
  int min_j = e->acceleration_bb[1];
  int min_k = e->acceleration_bb[2];
  int max_i = e->acceleration_bb[3];
  int max_j = e->acceleration_bb[4];
  int max_k = e->acceleration_bb[5];
  ElementID elementID = e->getID();

  assert (min_i >=0 && max_i >=0 && min_j >=0 && max_j >= 0 && min_k >=0 && max_k >= 0);

  //cout << "HI6" << endl;
  for (int i = min_i; i <= max_i; i++) {
    for (int j = min_j; j <= max_j; j++) {
      for (int k = min_k; k <= max_k; k++) {
	//cout << "REMOVE FROM CELL " << i << " " << j << " " << k << endl;
	std::set<ElementID>& cell = cells[getIndex(i,j,k)];
	bool answer = cell.erase(elementID);
	assert (answer == true);
      }
    } 
  }
}



