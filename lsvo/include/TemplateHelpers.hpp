//#include<optix_world.h>

#include<Photon.hpp>


template <unsigned int axis>
float getAxisComponent(const optix::float3& f) {
  return (&(f.x))[axis];
}

template <unsigned int axis>
struct ComparePhotons {
  typedef bool result_type;

  inline bool operator()(const PhotonRecord& a, const PhotonRecord& b) const {
    return getAxisComponent<axis>(a.position) < getAxisComponent<axis>(b.position);
  }

  inline bool operator()(const PhotonRecord* a, const PhotonRecord* b) const {
    return getAxisComponent<axis>(a->position) < getAxisComponent<axis>(b->position);
  }
};

template <typename T, unsigned int axis>
void
select(T* arr, int left, int right, int k, ComparePhotons<axis> comp) {
  while(1) {
    int pivotIndex    = (left + right) / 2;
    T* newPivot       = std::partition(&arr[left], &arr[right], boost::bind(comp, _1, arr[pivotIndex]));
    int newPivotIndex = newPivot - &arr[0];
    if(k == newPivotIndex) {
      return;
    } else if(k < newPivotIndex) {
      right = newPivotIndex - 1;
    } else {
      left  = newPivotIndex + 1;
    }
  }
}
