#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <vector>
#include <iostream>
#include <list>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cassert>

#include <cmath>

#include "vectors.h"
#include "utils.h"
#include "wall.h"
#include "wall_chain.h"
#include "mesh.h"

#include "triangle.h"
#include "quad.h"
#include "polygon.h"
#include "matrix.h"

#include "wall_fingerprint.h"

extern Vec3f PD_skylight_frame;
extern Vec3f PD_skylight_center;
extern Vec3f PD_wall;

// ===============================================================================
// ===============================================================================

BasicWall::BasicWall(double _bottom_edge, double _height, int _material_index) {
  bottom_edge = _bottom_edge;
  height_a=_height;
  height_b=-1;
  material_index=_material_index;
  assert (bottom_edge < height_a);
  if (fabs(height_a-BLUE_HEIGHT) < 0.01) {
    color = Vec3f(0,0,1);
  } else if (fabs(height_a-GREEN_HEIGHT) < 0.01) {
    color = Vec3f(0,1,0);
  } else if (fabs(height_a-RED_HEIGHT) < 0.01) {
    color = Vec3f(1,0,0);
  } else if (1) {
    std::cout << "height " << _height << std::endl;
    color = Vec3f(1,1,1);
  } else {
    assert(0);
    color = Vec3f(1,1,1);
  } 
  is_curved_wall = false;
  is_column = false;
  is_ramp = false;  
  is_platform = false;

  std::stringstream wall_name_ss;
  wall_name_ss << "WALL" << wall_counter;
  //std::cout << "set texture filename " << texture_filename.str() << std::endl;
  //m.setName(material_name.str());   m.setDiffuseReflectance(color);  m.setTextureFilename(texture_filename.str()); mesh->addMaterial(m);

  wall_name = wall_name_ss.str();
  wall_counter++;
}

BasicWall* BasicWall::QuadWall(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
			       double _bottom_edge, double _height, int _material_index) {
  BasicWall *answer = new BasicWall(_bottom_edge,_height,_material_index);
  ConvexQuad q(_g,_h,_a,_b,_c,_d,_e,_f);

  // Rotate so longest edges are 0->1 && 2->3
  double long_a = (q[0]-q[1]).Length();
  double long_b = (q[2]-q[3]).Length();
  double short_a = (q[1]-q[2]).Length();
  double short_b = (q[0]-q[3]).Length();
  if (short_a > 2.0*long_a && short_a > 2.0*long_b && short_b > 2.0*long_a && short_b > 2.0*long_b) {
    std::cout << "ROTATE" << std::endl;
    q.rotate90();
    long_a = (q[0]-q[1]).Length();
    long_b = (q[2]-q[3]).Length();
    short_a = (q[1]-q[2]).Length();
    short_b = (q[0]-q[3]).Length();
  }
  assert(long_a > 2.0*short_a && long_a > 2.0*short_b && long_b > 2.0*short_a && long_b > 2.0*short_b);

  // square up the edges so they are exactly parallel or perpendicular
  Vec3f centroid = (q[0]+q[1]+q[2]+q[3])*0.25;

  Vec3f long_dir = ((q[1]-q[0]) + (q[2]-q[3])) * 0.25;
  Vec3f short_dir = ((q[3]-q[0]) + (q[2]-q[1])) * 0.25;

  Vec3f v0 = centroid-long_dir-short_dir;
  Vec3f v1 = centroid+long_dir-short_dir;
  Vec3f v2 = centroid+long_dir+short_dir;
  Vec3f v3 = centroid-long_dir+short_dir;

  q = ConvexQuad(v0.x(),v0.z(),v1.x(),v1.z(),v2.x(),v2.z(),v3.x(),v3.z());

  answer->convex_quads.push_back(q);
  /*
  Vec3f a = 0.5*(answer.convex_quads[0][0]+answer.convex_quads[0][3]); 
  Vec3f b = 0.5*(answer.convex_quads[0][1]+answer.convex_quads[0][2]); 
  answer.start_trajectory = a-b;
  answer.start_trajectory.Normalize();
  answer.start_trajectory.Normalize();
  answer.end_trajectory = -answer.start_trajectory;
  */
  return answer;
}




Furniture* Furniture::Desk(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
                           double _height, int _material_index) {
  Furniture* answer = new Furniture(_a,_b,_c,_d,_e,_f,_g,_h);
  //std::cout << "DESK" << std::endl;
  answer->height = _height;
  answer->material_index = _material_index;
  answer->furn_type = FURNITURE_DESK;
  return answer;
}
Furniture* Furniture::Bed(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
                          double _height, int _material_index) {
  Furniture* answer = new Furniture(_a,_b,_c,_d,_e,_f,_g,_h);
  //std::cout << "BED" << std::endl;
  answer->height = _height;
  answer->material_index = _material_index;
  answer->furn_type = FURNITURE_BED;
  return answer;
}
Furniture* Furniture::Wardrobe(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
                               double _height, int _material_index) {
  Furniture* answer = new Furniture(_a,_b,_c,_d,_e,_f,_g,_h);
  //std::cout << "WARDROBE" << std::endl;
  answer->height = _height;
  answer->material_index = _material_index;
  answer->furn_type = FURNITURE_WARDROBE;
  return answer;
}



Vec3f BasicWall::get_start_trajectory() const {
  assert (!IsColumn());
  if (IsCurvedWall()) return curved_start_trajectory;
  Vec3f a = 0.5*(convex_quads[0][0]+convex_quads[0][3]); 
  Vec3f b = 0.5*(convex_quads[0][1]+convex_quads[0][2]); 
  Vec3f start_trajectory = a-b;
  start_trajectory.Normalize();
  return start_trajectory;
}

Vec3f BasicWall::get_end_trajectory() const {
  assert (!IsColumn());
  if (IsCurvedWall()) return curved_end_trajectory;
  int last_quad = convex_quads.size()-1;
  assert (last_quad == 0 || last_quad == 1);
  Vec3f a = 0.5*(convex_quads[last_quad][0]+convex_quads[last_quad][3]); 
  Vec3f b = 0.5*(convex_quads[last_quad][1]+convex_quads[last_quad][2]); 
  Vec3f end_trajectory = b-a;
  end_trajectory.Normalize();
  return end_trajectory;
}

BasicWall* BasicWall::LShapedWall(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
				 double _i,double _j,double _k,double _l,
				 double _bottom_edge, double _height, int _material_index) {
  BasicWall *answer = new BasicWall(_bottom_edge,_height,_material_index);
  ConvexQuad q1(_a,_b,_c,_d,_i,_j,_k,_l);
  ConvexQuad q2(_c,_d,_e,_f,_g,_h,_i,_j);

  answer->convex_quads.push_back(q1);
  answer->convex_quads.push_back(q2);
  /*
  Vec3f a = 0.5*(answer.convex_quads[0][0]+answer.convex_quads[0][3]); 
  Vec3f b = 0.5*(answer.convex_quads[0][1]+answer.convex_quads[0][2]); 
  Vec3f c = 0.5*(answer.convex_quads[1][1]+answer.convex_quads[1][2]); 
  answer.start_trajectory = a-b;
  answer.start_trajectory.Normalize();
  answer.end_trajectory = c-b;//-answer.start_trajectory;
  answer.end_trajectory.Normalize();
  */
  return answer;
}




void BasicWall::addWindow(const Window &w_orig, int which_quad) {
  Window w = w_orig;
  int num_quads = numConvexQuads();
  assert (which_quad >= 0 && which_quad < num_quads);
  ConvexQuad q = getConvexQuad(which_quad);
  //std::cout << std::endl << " ++++++++++++++++++++++++++" << std::endl;
  int best_j = -1;
  for (int j = 0; j < 4; j++) {
    bool all_dots = true;
    //std::cout << "   W" << j << " ";
    for (int i = 0; i < 4; i++) {
      Vec3f walldir = q[(i+1)%4] - q[i]; walldir.Normalize();
      Vec3f windir  = w[(i+j+1)%4] - w[(i+j)%4]; windir.Normalize();
      double dot = walldir.Dot3(windir);
      //std::cout << "  " << dot;
      if (dot < 0.4 ) all_dots = false;
      //assert (dot > 0.9);
    }    
    if (all_dots) best_j = j;
    //std::cout << std::endl;
  }
  //std::cout << "best_j  " << best_j << std::endl;
  if (best_j < 0) {
    std::cout << "WARNING: ignoring BAD WINDOW" << std::endl;
    return;
  }
  assert (best_j >= 0);
  //std::cout << "AFTER" << std::endl;

  /*
  w[0].Print("b0");
  w[1].Print("b1");
  w[2].Print("b2");
  w[3].Print("b3");
  */

  Vec3f tmp[4];
  tmp[0] = ProjectToSegment(w[(best_j+0)%4],q[0],q[1]);
  tmp[1] = ProjectToSegment(w[(best_j+1)%4],q[0],q[1]);
  tmp[2] = ProjectToSegment(w[(best_j+2)%4],q[2],q[3]);
  tmp[3] = ProjectToSegment(w[(best_j+3)%4],q[2],q[3]);

  w.setVert(0,tmp[0]); //ProjectToSegment(w[(best_j+0)%4],q[0],q[1]));
  w.setVert(1,tmp[1]); //ProjectToSegment(w[(best_j+1)%4],q[0],q[1]));
  w.setVert(2,tmp[2]); //ProjectToSegment(w[(best_j+2)%4],q[2],q[3]));
  w.setVert(3,tmp[3]); //ProjectToSegment(w[(best_j+3)%4],q[2],q[3]));
  /*
  w[0].Print("a0");
  w[1].Print("a1");
  w[2].Print("a2");
  w[3].Print("a3");
  */
  //std::cout << "dots ";
  bool all_dots = true;
  for (int i = 0; i < 4; i++) {
    Vec3f walldir = q[(i+1)%4] - q[i]; walldir.Normalize();
    Vec3f windir  = w[(i+1)%4] - w[i]; windir.Normalize();
    double dot = walldir.Dot3(windir);
    //std::cout << "  " << dot;
    if (dot < 0.4 ) all_dots = false;
  }
  //std::cout << std::endl;
  assert (all_dots);

  assert (OnSegment(w[0],q[0],q[1]));
  assert (OnSegment(w[1],q[0],q[1]));
  assert (OnSegment(w[2],q[2],q[3]));
  assert (OnSegment(w[3],q[2],q[3]));

  windows.push_back(w); 
}

BasicWall* BasicWall::CurvedWall(double _a,double _b,double _c,double _d,double _e,double _f, 
				 double bottom_edge, double _height,int material_index) {

  BasicWall *answer = new BasicWall(bottom_edge,_height, material_index);
  answer->is_curved_wall = true;
  Vec3f center = Vec3f(_a,0,_b);
  answer->circle_center = center;
  double radius_small = _c;
  double radius_large = _d;
  assert (radius_small < radius_large);
  double theta1 = _e;
  double theta2 = _f;

  int CURVE_SEGMENTS = my_ugly_args_hack->num_curve_segments;

  for (int i = CURVE_SEGMENTS-1; i >= 0; i--) {
    double angle2 = theta1 + i     * (theta2-theta1)/double(CURVE_SEGMENTS);
    double angle1 = theta1 + (i+1) * (theta2-theta1)/double(CURVE_SEGMENTS);
    Vec3f a = center+radius_small*Vec3f(cos(angle1),0,sin(angle1));
    Vec3f b = center+radius_small*Vec3f(cos(angle2),0,sin(angle2));
    Vec3f c = center+radius_large*Vec3f(cos(angle2),0,sin(angle2));
    Vec3f d = center+radius_large*Vec3f(cos(angle1),0,sin(angle1));
    answer->convex_quads.push_back(ConvexQuad(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z()));
  }

  Vec3f start = 0.5*(answer->convex_quads[0][0]+answer->convex_quads[0][3]); 
  Vec3f end = 0.5*(answer->convex_quads[CURVE_SEGMENTS-1][1]+answer->convex_quads[CURVE_SEGMENTS-1][2]); 

  Vec3f v1 = center-start; v1.Normalize();
  Vec3f v2 = center-end; v2.Normalize();

  Vec3f::Cross3(answer->curved_start_trajectory,v1,Vec3f(0,1,0));
  Vec3f::Cross3(answer->curved_end_trajectory,v2,Vec3f(0,1,0));

  answer->curved_start_trajectory.Normalize();
  answer->curved_start_trajectory = -answer->curved_start_trajectory;
  answer->curved_end_trajectory.Normalize();

  if (answer->curved_start_trajectory.Dot3(answer->curved_end_trajectory) > 0.6) {
    std::cout << "WARNING: TOO TIGHT A CURVED WALL ARC: BAD TRAJECTORY" << std::endl;
    delete answer;
    return NULL;
  }

  return answer;
}

BasicWall* BasicWall::Platform(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
			       double _bottom_edge, double _height, int _material_index) {
  BasicWall *answer = new BasicWall(_bottom_edge,_height,_material_index);
  ConvexQuad q(_g,_h,_a,_b,_c,_d,_e,_f);
  answer->is_platform = true;

  const double platform_width = INCH_IN_METERS*4.0;

  double long_a = (q[0]-q[1]).Length();
  double long_b = (q[2]-q[3]).Length();
  double short_a = (q[1]-q[2]).Length();
  double short_b = (q[0]-q[3]).Length();

  //assert (fabs(long_a-platform_width) < 0.5*INCH_IN_METERS);
  //assert (fabs(long_b-platform_width) < 0.5*INCH_IN_METERS);
  //assert (fabs(short_a-platform_width) < 0.5*INCH_IN_METERS);
  //assert (fabs(short_b-platform_width) < 0.5*INCH_IN_METERS);

  //  std::cout << "PLATFORM before " << long_a << " " << long_b << " " << short_a << " " << short_b << std::endl;

  /*
  if (short_a > 2.0*long_a && short_a > 2.0*long_b && short_b > 2.0*long_a && short_b > 2.0*long_b) {
    std::cout << "ROTATE" << std::endl;
    q.rotate90();
    long_a = (q[0]-q[1]).Length();
    long_b = (q[2]-q[3]).Length();
    short_a = (q[1]-q[2]).Length();
    short_b = (q[0]-q[3]).Length();
  }
  //assert(long_a > 2.0*short_a && long_a > 2.0*short_b && long_b > 2.0*short_a && long_b > 2.0*short_b);
  */

  // square up the edges so they are exactly parallel or perpendicular
  Vec3f centroid = (q[0]+q[1]+q[2]+q[3])*0.25;

  Vec3f axis_1 = ((q[1]-q[0]) + (q[2]-q[3]));// * 0.25;
  Vec3f axis_2 = ((q[3]-q[0]) + (q[2]-q[1]));// * 0.25;

  axis_1.Normalize();
  axis_2.Normalize();

  double dot = axis_1.Dot3(axis_2);
  //  std::cout << "dot " << dot << std::endl;
  
  double angle = SignedAngleBetweenNormalized(axis_1,axis_2,Vec3f(0,1,0));
  //  std::cout << "angle " << angle << std::endl;

  double adjust_angle;

  if (angle > 0) {
    adjust_angle = (angle - M_PI/2.0) /2.0;

  } else {
    adjust_angle = (angle + M_PI/2.0) /2.0;

  }

  //  std::cout << "adjust_angle " << adjust_angle << std::endl;
  
  Mat mat = Mat::MakeYRotation(adjust_angle);
  mat.TransformDirection(axis_1);
  mat = Mat::MakeYRotation(-adjust_angle);
  mat.TransformDirection(axis_2);

  dot = axis_1.Dot3(axis_2);
  //  std::cout << "dot after" << dot << std::endl;

  assert (fabs((double)(dot < 0.000001)));
  
  angle = SignedAngleBetweenNormalized(axis_1,axis_2,Vec3f(0,1,0));
  //  std::cout << "angle after" << angle << std::endl;

  
  Vec3f v0 = centroid-axis_1*platform_width*0.5-axis_2*platform_width*0.5;
  Vec3f v1 = centroid+axis_1*platform_width*0.5-axis_2*platform_width*0.5;
  Vec3f v2 = centroid+axis_1*platform_width*0.5+axis_2*platform_width*0.5;
  Vec3f v3 = centroid-axis_1*platform_width*0.5+axis_2*platform_width*0.5;

  q = ConvexQuad(v0.x(),v0.z(),v1.x(),v1.z(),v2.x(),v2.z(),v3.x(),v3.z());

  long_a = (q[0]-q[1]).Length();
  long_b = (q[2]-q[3]).Length();
  short_a = (q[1]-q[2]).Length();
  short_b = (q[0]-q[3]).Length();

  //  std::cout << "PLATFORM after " << long_a << " " << long_b << " " << short_a << " " << short_b << std::endl;

  answer->convex_quads.push_back(q);
  return answer;
}


BasicWall* BasicWall::Ramp(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
			   double _bottom_edge,double _height,int _material_index) {
  BasicWall *answer = new BasicWall(_bottom_edge,_height,_material_index);
  ConvexQuad q(_g,_h,_a,_b,_c,_d,_e,_f);
  answer->is_ramp = true;  

  assert (fabs(answer->height_a-2.0*INCH_IN_METERS) < 0.001);
  answer->height_b = 0.25*INCH_IN_METERS;

  const double ramp_width = INCH_IN_METERS*3.0;
  const double ramp_length = INCH_IN_METERS*5.0;

  //double short_a = (q[0]-q[1]).Length();
  //double short_b = (q[2]-q[3]).Length();
  //double long_a = (q[1]-q[2]).Length();
  //double long_b = (q[0]-q[3]).Length();

//  assert (fabs(long_a-ramp_length) < 0.5*INCH_IN_METERS);
//  assert (fabs(long_b-ramp_length) < 0.5*INCH_IN_METERS);
//  assert (fabs(short_a-ramp_width) < 0.5*INCH_IN_METERS);
//  assert (fabs(short_b-ramp_width) < 0.5*INCH_IN_METERS);

  //  std::cout << "RAMP before " << long_a << " " << long_b << " " << short_a << " " << short_b << std::endl;

  /*
  if (short_a > 2.0*long_a && short_a > 2.0*long_b && short_b > 2.0*long_a && short_b > 2.0*long_b) {
    std::cout << "ROTATE" << std::endl;
    q.rotate90();
    long_a = (q[0]-q[1]).Length();
    long_b = (q[2]-q[3]).Length();
    short_a = (q[1]-q[2]).Length();
    short_b = (q[0]-q[3]).Length();
  }
  //assert(long_a > 2.0*short_a && long_a > 2.0*short_b && long_b > 2.0*short_a && long_b > 2.0*short_b);
  */

  // square up the edges so they are exactly parallel or perpendicular
  Vec3f centroid = (q[0]+q[1]+q[2]+q[3])*0.25;

  Vec3f axis_1 = ((q[1]-q[0]) + (q[2]-q[3]));// * 0.25;
  Vec3f axis_2 = ((q[3]-q[0]) + (q[2]-q[1]));// * 0.25;

  axis_1.Normalize();
  axis_2.Normalize();

  double dot = axis_1.Dot3(axis_2);
  //  std::cout << "dot " << dot << std::endl;
  
  double angle = SignedAngleBetweenNormalized(axis_1,axis_2,Vec3f(0,1,0));
  //  std::cout << "angle " << angle << std::endl;

  double adjust_angle;

  if (angle > 0) {
    adjust_angle = (angle - M_PI/2.0) /2.0;

  } else {
    adjust_angle = (angle + M_PI/2.0) /2.0;

  }

  //  std::cout << "adjust_angle " << adjust_angle << std::endl;
  
  Mat mat = Mat::MakeYRotation(adjust_angle);
  mat.TransformDirection(axis_1);
  mat = Mat::MakeYRotation(-adjust_angle);
  mat.TransformDirection(axis_2);

  dot = axis_1.Dot3(axis_2);
  //  std::cout << "dot after" << dot << std::endl;

  assert (fabs((double)(dot < 0.000001)));
  
  angle = SignedAngleBetweenNormalized(axis_1,axis_2,Vec3f(0,1,0));
  //  std::cout << "angle after" << angle << std::endl;

  Vec3f v0 = centroid-axis_1*ramp_width*0.5-axis_2*ramp_length*0.5;
  Vec3f v1 = centroid+axis_1*ramp_width*0.5-axis_2*ramp_length*0.5;
  Vec3f v2 = centroid+axis_1*ramp_width*0.5+axis_2*ramp_length*0.5;
  Vec3f v3 = centroid-axis_1*ramp_width*0.5+axis_2*ramp_length*0.5;

  q = ConvexQuad(v0.x(),v0.z(),v1.x(),v1.z(),v2.x(),v2.z(),v3.x(),v3.z());

  //  std::cout << "RAMP after " << long_a << " " << long_b << " " << short_a << " " << short_b << std::endl;

  answer->convex_quads.push_back(q);
  return answer;
}

BasicWall* BasicWall::Column(double x,double z,double radius,double bottom_edge,double height,int material_index) {
  BasicWall *answer = new BasicWall(bottom_edge,height,material_index);
  answer->is_column = true;  

  //  answer->circle_center = Vec3f(x,0,z);
  Vec3f &center = answer->circle_center = Vec3f(x,0,z);
  assert (radius > 0);

  int column_segments = my_ugly_args_hack->num_column_faces/2 -1;
  assert (column_segments >= 1);
  double angle = 2*M_PI / double(my_ugly_args_hack->num_column_faces); //num_vertices);
  for (int i = 0; i < column_segments; i++) {
    double angle1 = (i+0.5)*angle;
    double angle2 = (i+1.5)*angle;    
    Vec3f a = center+radius*Vec3f(cos(angle1),0,sin(angle1));
    Vec3f b = center+radius*Vec3f(cos(angle2),0,sin(angle2));
    Vec3f c = center+radius*Vec3f(cos(-angle2),0,sin(-angle2));
    Vec3f d = center+radius*Vec3f(cos(-angle1),0,sin(-angle1));
    answer->convex_quads.push_back(ConvexQuad(a.x(),a.z(),b.x(),b.z(),c.x(),c.z(),d.x(),d.z()));
  }
  // const ConvexQuad &q = answer->getConvexQuad(0);
  return answer;
}



// ========================================================================================
// ========================================================================================
// ========================================================================================

Vec3f BasicWall::getCentroid() const {
  Vec3f answer(0,0,0);
  for (unsigned int i = 0; i < convex_quads.size(); i++) {
    answer += convex_quads[i].getCentroid();
  }
  answer /= double(convex_quads.size());
  return answer;
}

#define MY_BARYCENTRIC_EPSILON 0.00001

double BasicWall::getRampHeight(const Vec3f &pos) const {
  assert (IsRamp());

  assert (numGoodVerts() == 4);

  double alpha, beta, gamma;
  BarycentricCoordinates(getGoodVert(0),
			 getGoodVert(1),
			 getGoodVert(2),
			 pos, alpha,beta,gamma);

  double h;

  if (alpha < -MY_BARYCENTRIC_EPSILON || beta < -MY_BARYCENTRIC_EPSILON || gamma < -MY_BARYCENTRIC_EPSILON) {
    BarycentricCoordinates(getGoodVert(0),
			   getGoodVert(2),
			   getGoodVert(3),
			   pos, alpha,beta,gamma);
    assert (alpha >= -MY_BARYCENTRIC_EPSILON && beta >= -MY_BARYCENTRIC_EPSILON && gamma >= -MY_BARYCENTRIC_EPSILON);
    //std::cout << "abg " << alpha << " " << beta << " " << gamma << std::endl;
    h = getHeight2(0)*alpha + getHeight2(2)*beta + getHeight2(3)*gamma;
  } else {
    assert (alpha >= -MY_BARYCENTRIC_EPSILON && beta >= -MY_BARYCENTRIC_EPSILON && gamma >= -MY_BARYCENTRIC_EPSILON);
    //std::cout << "abg " << alpha << " " << beta << " " << gamma << std::endl;
    h = getHeight2(0)*alpha + getHeight2(1)*beta + getHeight2(2)*gamma;
  }
  return h;
}


void BasicWall::invert() {
  assert (convex_quads.size() >= 1);
  bool flag = convex_quads[0].invert();
  for (unsigned int i = 1; i < convex_quads.size(); i++) {
    bool flag2 = convex_quads[i].invert();
    assert (flag == flag2);
  }
}
void BasicWall::flip() {
  for (unsigned int i = 0; i < convex_quads.size(); i++) {
    convex_quads[i].flip();
  }
}
void BasicWall::rotate90() {
  for (unsigned int i = 0; i < convex_quads.size(); i++) {
    convex_quads[i].rotate90();
  }
}


// ===============================================================================
// ===============================================================================


// ===========================================================================================
// ===========================================================================================
// PAINT
// ===========================================================================================
// ===========================================================================================

void Window::paint(double height) const {
  glBegin(GL_QUADS);
  Vec3f v_height = Vec3f(0,height*1.02,0);
  //top
  glColor3f(color.r(),color.g(),color.b());
  InsertNormal((*this)[0],(*this)[3],(*this)[2]);
  VecVertex((*this)[0]+v_height);
  VecVertex((*this)[3]+v_height);
  VecVertex((*this)[2]+v_height);
  VecVertex((*this)[1]+v_height);
  glEnd();
}

void Skylight::paint() const {
  glBegin(GL_QUADS);
  Vec3f v_height = Vec3f(0,0.01,0);
  //top
  glColor3f(PD_skylight_frame.x(), PD_skylight_frame.y(), PD_skylight_frame.z());
  InsertNormal((*this)[0],(*this)[3],(*this)[2]);
  VecVertex((*this)[0]+v_height);
  VecVertex((*this)[3]+v_height);
  VecVertex((*this)[2]+v_height);
  VecVertex((*this)[1]+v_height);
  glEnd();
}

void Furniture::paint() const {
  glBegin(GL_QUADS);
  Vec3f v_height = Vec3f(0,height,0);
  //top

  glColor3f(1.0,0.6,0.2);

  InsertNormal((*this)[0],(*this)[3],(*this)[2]);
  VecVertex((*this)[0]+v_height);
  VecVertex((*this)[3]+v_height);
  VecVertex((*this)[2]+v_height);
  VecVertex((*this)[1]+v_height);

  InsertNormal((*this)[0],(*this)[1],(*this)[0]+v_height);
  VecVertex((*this)[0]);
  VecVertex((*this)[0]+v_height);
  VecVertex((*this)[1]+v_height);
  VecVertex((*this)[1]);

  InsertNormal((*this)[1],(*this)[2],(*this)[1]+v_height);
  VecVertex((*this)[1]);
  VecVertex((*this)[1]+v_height);
  VecVertex((*this)[2]+v_height);
  VecVertex((*this)[2]);

  InsertNormal((*this)[2],(*this)[3],(*this)[2]+v_height);
  VecVertex((*this)[2]);
  VecVertex((*this)[2]+v_height);
  VecVertex((*this)[3]+v_height);
  VecVertex((*this)[3]);

  InsertNormal((*this)[3],(*this)[0],(*this)[3]+v_height);
  VecVertex((*this)[3]);
  VecVertex((*this)[3]+v_height);
  VecVertex((*this)[0]+v_height);
  VecVertex((*this)[0]);


  glEnd();

}

void BasicWall::paint() const {
  glBegin(GL_QUADS);
  //  Vec3f h = Vec3f(0,height2,0);
  Vec3f b = Vec3f(0,bottom_edge,0);
  unsigned int num_quads = convex_quads.size();
  for (unsigned int i = 0; i < num_quads; i++) {
    const ConvexQuad& q = convex_quads[i];
    Vec3f h[4] = { Vec3f(0,getHeight2(0),0),
		   Vec3f(0,getHeight2(1),0),
		   Vec3f(0,getHeight2(2),0),
		   Vec3f(0,getHeight2(3),0) };
    //top
    glColor3f(color.r(),color.g(),color.b());
    InsertNormal(q[3]+h[3],q[2]+h[2],q[1]+h[1]);
    VecVertex(q[3]+h[3]);
    VecVertex(q[2]+h[2]);
    VecVertex(q[1]+h[1]);
    VecVertex(q[0]+h[0]);
    glColor3f(PD_wall.x(),PD_wall.y(),PD_wall.z());
    // bottom
    InsertNormal(q[0]+b,q[1]+b,q[2]+b);
    VecVertex(q[0]+b);
    VecVertex(q[1]+b);
    VecVertex(q[2]+b);
    VecVertex(q[3]+b);
    // front
    InsertNormal(q[1]+b,q[0]+b,q[0]+h[0]);
    VecVertex(q[1]+b);
    VecVertex(q[0]+b);
    VecVertex(q[0]+h[0]);
    VecVertex(q[1]+h[1]);
    // back
    InsertNormal(q[3]+b,q[2]+b,q[2]+h[2]);
    VecVertex(q[3]+b);
    VecVertex(q[2]+b);
    VecVertex(q[2]+h[2]);
    VecVertex(q[3]+h[3]);
    // sides
    if (i == 0) {
      InsertNormal(q[0]+b,q[3]+b,q[3]+h[3]);
      VecVertex(q[0]+b);
      VecVertex(q[3]+b);
      VecVertex(q[3]+h[3]);
      VecVertex(q[0]+h[0]);
    }
    if (i == num_quads-1) {
      InsertNormal(q[2]+b,q[1]+b,q[1]+h[1]);
      VecVertex(q[2]+b);
      VecVertex(q[1]+b);
      VecVertex(q[1]+h[1]);
      VecVertex(q[2]+h[2]);
    }
  }
  glEnd();
  for (unsigned int i = 0; i < windows.size(); i++) {
    windows[i].paint(getMaxHeight()+0.0005);
  }

  if (wall_name == "canvas_wall" ||
      wall_name == "luan_wall") {

    const ConvexQuad& q = convex_quads[0];

    Vec3f va = 0.9*q[3] + 0.1*q[2] + (q[3]-q[0])*0.1;
    Vec3f vb = 0.1*q[3] + 0.9*q[2] + (q[3]-q[0])*0.1;
    Vec3f vc = va + (q[3]-q[0])*15;
    Vec3f vd = vb + (q[3]-q[0])*15;

    
    Vec3f h = Vec3f(0,getMaxHeight(),0);

    glColor3f(1,0,0);
    glDisable(GL_LIGHTING);
    glLineWidth(10);
    glBegin(GL_LINES);
    VecVertex(va+b);
    VecVertex(vc+b);
    VecVertex(va+0.8*h);
    VecVertex(vc+b);
    VecVertex(va+0.8*h);
    VecVertex(va+b);
    VecVertex(vb+b);
    VecVertex(vd+b);
    VecVertex(vb+0.8*h);
    VecVertex(vd+b);
    VecVertex(vb+0.8*h);
    VecVertex(vb+b);
    glEnd();
    glEnable(GL_LIGHTING);
  }
}

// ===========================================================================================
