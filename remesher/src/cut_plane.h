#ifndef _CUT_PLANE_H_
#define _CUT_PLANE_H_

#include "vectors.h"

class CutPlane {
 public:
  Vec3f quad[4];
  Vec3f normal;
  int which_projector;
  Vec3f projector_center;
  double d;
};

#endif
