#ifndef CAMERA_H_INCLUDED_
#define CAMERA_H_INCLUDED_

#include <algorithm>
#include <Vector3.h>
#include <Matrix3.h>
#include <util.h>
#include "math.h"

class Camera {
public:
  Camera (){
  }

  void loadCalibration(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("cannot open %s\n", filename);
    }
    
    char comment[1024];
    fgets(comment, 1024, fp);
    fscanf(fp, "%d %d", &cols, &rows);

    double temp[9];
    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    for (int i=0; i<9; i++){
      fscanf(fp, "%lf", &temp[i]);
    }
    K = m3d(temp);

    double temp2[3];
    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    for (int i=0; i<3; i++){
      for (int j=0; j<3; j++){
        fscanf(fp, "%lf", &temp[i*3+j]);
      }
      fscanf(fp, "%lf", &temp2[i]);      
    }
    R = m3d(temp);
    t = v3d(temp2);

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    fscanf(fp, "%lf", &sx);
    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    fscanf(fp, "%lf", &k1);
    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    fscanf(fp, "%lf %lf %lf", 
           &(world_offset.x()),
           &(world_offset.y()),
           &(world_offset.z()));
    
    fclose(fp);

    // remove world frame calibration offset
    t -= world_offset;

    // calculate the center of projection
    center = - (R.transpose() * t);

    // cache KR, Kt for later
    KR = K * R;
    Kt = K * t;

    // cache inverse (KR)^-1 for later
    KRinv = R.transpose() * K.inverse();
  }

  //!!! note: we compensate for the 0.5" gatorboard target thickness here
  v3d PointFromPixel(double row, double col, double height){
    v3d origin = center;
    v3d direction = view_vector(row, 
                                getCx() + (col - getCx())/sx);

    const double epsilon = 1e-6;
    if (fabs(direction.y()) < epsilon){
      FATAL_ERROR("View orthogonal to y-axis");
    }
    double l = (height - origin.y()) / direction.y();
    
    return origin + l * direction;
  }

  v3d PixelFromPoint(const v3d &point){
    v3d pixel = KR * point + Kt;
    pixel = pixel / pixel.z();
    pixel.x() = getCx() + sx * (pixel.x() - getCx());
    return pixel;
  }

  void writeOpenGLCamera(const char *filename, double near, double far){
    double GLProjection[16];
    double GLModelview[16];

    CalculateOpenGLMatrix(near, far, GLProjection, GLModelview);
    
    FILE *fp = fopen(filename, "wt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s for output\n", filename);
      exit(-1);
    }
    fprintf(fp, "# width height\n");
    fprintf(fp, "%6d %6d\n", cols, rows);
    fprintf(fp, "# projection matrix\n");
    for (int i=0; i<4; i++){
      for (int j=0; j<4; j++){
        fprintf(fp, "%10.5lf ", GLProjection[4*i+j]);
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "# modelview matrix\n");
    for (int i=0; i<4; i++){
      for (int j=0; j<4; j++){
        fprintf(fp, "%10.5lf ", GLModelview[4*i+j]);
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "# camera center\n");
    fprintf(fp, "%10.5lf %10.5lf %10.5lf\n",
            center.x(), center.y(), center.z());    
    fclose(fp);
  }

  v3d getCenter(){
    return center;
  }

  v3d getOffset(){
    return world_offset;
  }

  double getSx(){
    return sx;
  }

  double getK1(){
    return k1;
  }

  double getCx(){
    return K(0,3);
  }

  double getCy(){
    return K(1,3);
  }

private:
  int rows, cols;
  m3d K; // camera intrinsics
  m3d R; // extrinsics rotation
  m3d KR; // KR cached for fast calculation
  m3d KRinv; // (KR)^-1 cached for fast calculation
  v3d t; // extrinsics translation
  v3d Kt; // cached for fast calculation
  v3d world_offset;
  double sx;
  double k1;
  v3d center; // center of projection cached for fast access

  // return a line-of-sight vector through a given pixel
  v3d view_vector(double row, double col){
    v3d x(col, row, 1.);
    v3d v = KRinv * x;
    v.normalize();
    return v;
  }


  void CalculateOpenGLMatrix(double near, double far, // opengl near&far planes
                             double GLProjection[16], // proj matrix
                             double GLModelview[16])  // output modelview matrix
  {
    double K1[16];
    double Vinv[16];
    double temp[16];
    int width = cols;
    int height = rows;

    // create opengl projection matrix from camera intrinsics
    K1[0] = K(0,0)*sx;  // note sx can be set to 1. if you don't know it
    K1[1] = K(0,1);
    K1[2] = K(0,2);
    K1[3] = 0.;
    
    K1[4] = 0.;
    K1[5] = K(1,1);
    K1[6] = K(1,2);
    K1[7] = 0.;
    
    K1[8] = 0.;
    K1[9] = 0.;
    K1[10] = (far + near)/(far - near);
    K1[11] = -2.*near*far/(far - near);
    
    K1[12] = 0.;
    K1[13] = 0.;
    K1[14] = 1.;
    K1[15] = 0.;
    
    // create the inverse of the opengl viewport transform
    // note: y gets flipped 
    double w2 = 2./width;
    double h2 = 2./height;
    Vinv[0] = w2;
    Vinv[1] = 0.;
    Vinv[2] = 0.;
    Vinv[3] = -1.;
    
    Vinv[4] = 0.;
    Vinv[5] = -h2;
    Vinv[6] = 0.;
    Vinv[7] = 1.;

    Vinv[8] = 0.;
    Vinv[9] = 0.;
    Vinv[10] = 2.;
    Vinv[11] = -1.;

    Vinv[12] = 0.;
    Vinv[13] = 0.;
    Vinv[14] = 0.;
    Vinv[15] = 1.;

    // temp = Vinv * K1
    for (int r=0; r<4; r++){
      for (int c=0; c<4; c++){
        temp[r*4+c] = 0.;
        for (int k=0;k<4;k++){
          temp[r*4+c] += Vinv[r*4+k] * K1[k*4+c];
        }
      }
    }
  
    // transpose into column-major
    for (int i=0; i<4; i++){
      for (int j=0; j<4; j++){
        GLProjection[i*4+j] = temp[j*4+i];
      }
    }

    // create modelview matrix from camera extrinsics

    // fill in row-major matrix first
    for (int i=0; i<3; i++){
      for (int j=0; j<3; j++){
        temp[i*4+j] = R(i,j);
      }
      temp[i*4+3] = t(i);
    }
    temp[12] = 0.;
    temp[13] = 0.;
    temp[14] = 0.;
    temp[15] = 1.;
    
    // transpose into column-major
    for (int i=0; i<4; i++){
      for (int j=0; j<4; j++){
        GLModelview[i*4+j] = temp[j*4+i];
      }
    }
  }

};

#endif // #ifndef CAMERA_H_INCLUDED_
