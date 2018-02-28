#ifndef ACCELERATION_GRID_H
#define ACCELERATION_GRID_H

#include "vectors.h"
#include <set>
#include <vector>
#include "triangle.h"
#include "mesh.h"
#include "vertex.h"

class Mesh;

template <class T1, class T2>
class AccelerationGrid {

 public:

  // constructor
  AccelerationGrid(int _dx, int _dy, int _dz, Vec3f _min, Vec3f _max) {
    dx = _dx;
    dy = _dy;
    dz = _dz;
    min = _min;
    max = _max;
    cells = std::vector<std::set<T2> > (dx*dy*dz);
  }
  
  void AddElement(T1 e);
  void RemoveElement(T1 e);

  // accessor
  const std::set<T2>& getElementsInCell(int i, int j, int k) const {
    assert (i >= 0 && i < dx);
    assert (j >= 0 && j < dy);
    assert (k >= 0 && k < dz);
    return cells[getIndex(i,j,k)];
  }

  void printCell(int &i, int &j, int &k) {
    double __dx = (max.x()-min.x())/double(dx);
    double __dy = (max.y()-min.y())/double(dy);
    double __dz = (max.z()-min.z())/double(dz);
    std::cout << "CELL " << i << " " << j << " " << k << std::endl;
    std::cout << "  min " << 
      min.x()+__dx*i << " " << 
      min.y()+__dy*j << " " << 
      min.z()+__dz*k << " " << std::endl;
    std::cout << "  max " << 
      min.x()+__dx*(i+1) << " " << 
      min.y()+__dy*(j+1) << " " << 
      min.z()+__dz*(k+1) << " " << std::endl;
  }
  

  void whichCell(Vec3f v, int &i, int &j, int &k) {
    assert (v.x() >= min.x() && v.x() <= max.x());
    i = (int)floor( (v.x()-min.x())*dx / (max.x()-min.x() ) );
    assert (i >= 0 && i < dx);

    assert (v.y() >= min.y() && v.y() <= max.y());
    j = (int)floor( (v.y()-min.y())*dy / (max.y()-min.y() ) );
    assert (j >= 0 && j < dy);

    assert (v.z() >= min.z() && v.z() <= max.z());
    k = (int)floor( (v.z()-min.z())*dz / (max.z()-min.z() ) );
    assert (k >= 0 && k < dz);

    //std::cout << "which cell " << std::endl;
    //v.Print("  v");
    //std::cout << "  " << i << " " << j << " " << k << std::endl;

  }

  // modifier

  // modifier
  bool removeFromCell(int i, int j, int k, T2 element) {
    std::set<T2>& cell = cells[getIndex(i,j,k)];
    bool answer = cell.erase(element);
    return answer;
  }


  void PrintStats() {
    for (int i = 0; i < dx; i++) {
      for (int j = 0; j < dy; j++) {
	for (int k = 0; k < dz; k++) {      
	  int count = cells[getIndex(i,j,k)].size();
	  if (count > 0)
	    std::cout << i << ":" << j << ":" << k << "  " << count << std::endl;
	}
      }
    }
  }

  // private:

  int getIndex(int i, int j, int k) const {
    assert (i >= 0 && i < dx);
    assert (j >= 0 && j < dy);
    assert (k >= 0 && k < dz);
    //std::cout << "dimensions " << dx << " " << dy << " " << dz << std::endl;
    int answer = i*dy*dz + j*dz + k;
    //std::cout << i << " " << j << " " << k << " " << answer << " " << dx*dy*dz << std::endl;
    assert (answer >=0 && answer < dx*dy*dz);
    return answer;
  }

  // representation
  int dx;
  int dy;
  int dz;

  Vec3f min;
  Vec3f max;

  // public:
  std::vector<std::set<T2> > cells;

  Mesh *mesh;
};


template <> 
void AccelerationGrid<Element*,ElementID>::AddElement(Element* e);

template <>
void AccelerationGrid<Element*,ElementID>::RemoveElement(Element* e);



#endif
