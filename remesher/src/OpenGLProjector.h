#ifndef OPENGLPROJECTOR_H_INCLUDED_
#define OPENGLPROJECTOR_H_INCLUDED_

#include <cstdlib>
#include <cstdio>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "vectors.h"

// represent projector as OpenGL camera
class OpenGLProjector {
public:

  // create a camera object from a *.cam file
  OpenGLProjector(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "Unable to open file %s\n", filename);
      exit(-1);
    }

    char comment[1024];
    fgets(comment, 1024, fp);
    fscanf(fp, "%d %d", &width, &height);
    // printf("%d %d\n", width, height);

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    for (int i=0; i<16; i++){
      fscanf(fp, "%lf", &projection_matrix[i]);
      // printf("%f\n", projection_matrix[i]);
    }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    for (int i=0; i<16; i++){
      fscanf(fp, "%lf", &modelview_matrix[i]);
      //      printf("%f\n", modelview_matrix[i]);
    }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    fscanf(fp, "%lf", &cx);
    fscanf(fp, "%lf", &cy);
    fscanf(fp, "%lf", &cz);

    cx2 = cx;
    cy2 = cy;
    cz2 = cz;

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    for(int i=0; i<4; ++i) {
      for(int j=0; j<3; ++j) {
	fscanf(fp, "%lf", &near_plane_verts[i][j]);
	//printf("%f ", near_plane_verts[i][j]);
      }
      //printf("\n");
    }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    fscanf(fp, "%lf", &near_plane_dist);
    //printf("%f ", near_plane_dist);
    fscanf(fp, "%lf", &far_plane_dist);
    //printf("%f ", far_plane_dist);

    fclose(fp);

    const GLdouble* near_a = getNearPlaneVertexOriginal(0);
    const GLdouble* near_b = getNearPlaneVertexOriginal(1);
    const GLdouble* near_c = getNearPlaneVertexOriginal(2);
    Vec3f npa = Vec3f(near_a[0],near_a[1],near_a[2]);
    Vec3f npb = Vec3f(near_b[0],near_b[1],near_b[2]);
    Vec3f npc = Vec3f(near_c[0],near_c[1],near_c[2]);
    
    // HACKnot really horizontal & vertical!!!
    Vec3f horizontal = npa-npb;
    horizontal.Normalize();
    Vec3f vertical = npa-npc;
    vertical.Normalize();
    Vec3f::Cross3(direction,vertical,horizontal);
    direction.Normalize();

    //direction.Print("DIRECTION");

    if (direction.y() > 0) {
      //cout << "FLIPPING PROJECTOR DIRECTION" << endl;
      direction = -direction;
      double x = near_plane_verts[1][0];
      double y = near_plane_verts[1][1];
      double z = near_plane_verts[1][2];
      near_plane_verts[1][0] = near_plane_verts[3][0];
      near_plane_verts[1][1] = near_plane_verts[3][1];
      near_plane_verts[1][2] = near_plane_verts[3][2];
      near_plane_verts[3][0] = x;
      near_plane_verts[3][1] = y;
      near_plane_verts[3][2] = z;
    }

    near_plane_verts_replacement[0][0] = near_plane_verts[0][0];
    near_plane_verts_replacement[0][1] = near_plane_verts[0][1];
    near_plane_verts_replacement[0][2] = near_plane_verts[0][2];
    near_plane_verts_replacement[1][0] = near_plane_verts[1][0];
    near_plane_verts_replacement[1][1] = near_plane_verts[1][1];
    near_plane_verts_replacement[1][2] = near_plane_verts[1][2];
    near_plane_verts_replacement[2][0] = near_plane_verts[2][0];
    near_plane_verts_replacement[2][1] = near_plane_verts[2][1];
    near_plane_verts_replacement[2][2] = near_plane_verts[2][2];
    near_plane_verts_replacement[3][0] = near_plane_verts[3][0];
    near_plane_verts_replacement[3][1] = near_plane_verts[3][1];
    near_plane_verts_replacement[3][2] = near_plane_verts[3][2];

    direction_replacement = direction;
  }

  const Vec3f& getProjDirectionOriginal() const { return direction; }
  const Vec3f& getProjDirectionReplacement() const { return direction_replacement; }

  Vec3f getVec3fCenterOriginal2() const { return Vec3f(cx,cy,cz); }
  Vec3f getVec3fCenterReplacement() const { return Vec3f(cx2,cy2,cz2); }

  static void FindGroundPlane(const Vec3f &c_orig, double nearPlane[3], double ground[3]) {
    ground[0] = c_orig.x() - c_orig.y()*(c_orig.x()-nearPlane[0])/(c_orig.y()-nearPlane[1]);
    ground[1] = 0;
    ground[2] = c_orig.z() - c_orig.y()*(c_orig.z()-nearPlane[2])/(c_orig.y()-nearPlane[1]);
    
    std::cout << "GROUND : " << ground[0] << " " << ground[1] << " " << ground [2] << std::endl;

  }

  static void SetNearPlaneReplacement(const Vec3f &c_repl, double ground[3], double nearPlane[3]) {
    assert (c_repl.y() > 0 && fabs(ground[1]) < 0.00001);
    nearPlane[0] = c_repl.x() + (ground[0]-c_repl.x())/(ground[1]-c_repl.y());
    nearPlane[1] = c_repl.y()-1;
    nearPlane[2] = c_repl.z() + (ground[2]-c_repl.z())/(ground[1]-c_repl.y());

    std::cout << "NEAR : " << nearPlane[0] << " " << nearPlane[1] << " " << nearPlane [2] << std::endl;
  }


  void setVec3fCenterReplacement(const Vec3f &v) { 
    cx2 = v.x(); cy2 = v.y(); cz2 = v.z(); 
    
    //SetNearPlaneReplacement();

    double ground_plane_verts[4][3];
    
    FindGroundPlane(getVec3fCenterOriginal2(),near_plane_verts[0],ground_plane_verts[0]);
    FindGroundPlane(getVec3fCenterOriginal2(),near_plane_verts[1],ground_plane_verts[1]);
    FindGroundPlane(getVec3fCenterOriginal2(),near_plane_verts[2],ground_plane_verts[2]);
    FindGroundPlane(getVec3fCenterOriginal2(),near_plane_verts[3],ground_plane_verts[3]);

    SetNearPlaneReplacement(getVec3fCenterReplacement(),ground_plane_verts[0],near_plane_verts_replacement[0]);
    SetNearPlaneReplacement(getVec3fCenterReplacement(),ground_plane_verts[1],near_plane_verts_replacement[1]);
    SetNearPlaneReplacement(getVec3fCenterReplacement(),ground_plane_verts[2],near_plane_verts_replacement[2]);
    SetNearPlaneReplacement(getVec3fCenterReplacement(),ground_plane_verts[3],near_plane_verts_replacement[3]);

    Vec3f d = (getNearPlaneVec3fReplacement(0) + getNearPlaneVec3fReplacement(1) + 
	       getNearPlaneVec3fReplacement(2) + getNearPlaneVec3fReplacement(3)) * 0.25;
    d -= getVec3fCenterReplacement();
    d.Normalize();
    
    direction_replacement = d;
  }

  void getCenterOriginal2(GLdouble &x, GLdouble &y, GLdouble &z) const{
    x = cx;
    y = cy;
    z = cz;
  }

  void getCenterReplacement(GLdouble &x, GLdouble &y, GLdouble &z) const{
    x = cx2;
    y = cy2;
    z = cz2;
  }


  GLint getHeight() const{
    //    return 512;
    return height;
  }

  GLint getWidth() const{
    //    return 512;
    return width;
  }

  Vec3f getNearPlaneVec3fOriginal(int i) const {
    assert (i >= 0 && i < 4);
    return Vec3f (near_plane_verts[i][0],
		  near_plane_verts[i][1],
		  near_plane_verts[i][2]);
  }

  Vec3f getNearPlaneVec3fReplacement(int i) const {
    assert (i >= 0 && i < 4);
    return Vec3f (near_plane_verts_replacement[i][0],
		  near_plane_verts_replacement[i][1],
		  near_plane_verts_replacement[i][2]);
  }

  inline GLdouble getNearPlaneDist() const
  {
    return near_plane_dist;
  }
  
  inline GLdouble getFarPlaneDist() const
  {
    return far_plane_dist;
  }

  inline const GLdouble* getNearPlaneVertexOriginal(int i) const
  {
    if(i<4)
      return near_plane_verts[i];
    else 
      return 0;
  }

  inline const GLdouble* getNearPlaneVertexReplacement(int i) const
  {
    if(i<4)
      return near_plane_verts_replacement[i];
    else 
      return 0;
  }

  GLdouble *getProjectionMatrix() const{
    return (GLdouble*)&projection_matrix;
  }

  GLdouble *getModelviewMatrix() const{
    return (GLdouble*)&modelview_matrix;
  }

  // set the opengl "camera" to the view for this projector
  // NOTE: this overwrites the current projection and modelview matrices
  void setOpenGLCamera() const{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection_matrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(modelview_matrix);
  }

private:
  GLint height;
  GLint width;
  GLdouble cx, cy, cz;  // camera center

  // REPLACEMENT CENTER!!!
  GLdouble cx2, cy2, cz2;  // camera center

  GLdouble projection_matrix[16];
  GLdouble modelview_matrix[16];
  // 4 vertices of near plane
  GLdouble near_plane_verts[4][3];

  GLdouble near_plane_verts_replacement[4][3];

  GLdouble near_plane_dist; // near plane distance from camera center
  GLdouble far_plane_dist;  // far plane distance from camera center

  Vec3f direction;
  Vec3f direction_replacement;
};

#endif // #ifndef OPENGLPROJECTOR_H_INCLUDED_
