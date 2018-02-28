#ifndef _MATRIX_H_
#define _MATRIX_H_

//
// originally implemented by Justin Legakis
//

#include <cmath>

#include "vectors.h"

// ====================================================================
// ====================================================================

class Mat {

public:

  // CONSTRUCTORS & DESTRUCTOR
  Mat() { Clear(); }
  Mat(const Mat& m);
  Mat(const double *m);
  ~Mat() {}
  
  // ACCESSORS
  float* glGet(void) const {
    float *glMat = new float[16];
    glMat[0]=data[0][0];  glMat[1]=data[1][0];  glMat[2]=data[2][0];  glMat[3]=data[3][0];
    glMat[4]=data[0][1];  glMat[5]=data[1][1];  glMat[6]=data[2][1];  glMat[7]=data[3][1];
    glMat[8]=data[0][2];  glMat[9]=data[1][2];  glMat[10]=data[2][2]; glMat[11]=data[3][2];
    glMat[12]=data[0][3]; glMat[13]=data[1][3]; glMat[14]=data[2][3]; glMat[15]=data[3][3];
    return glMat; }
  double Get(int x, int y) const { 
    assert (x >= 0 && x < 4);
    assert (y >= 0 && y < 4);
    return data[y][x]; }
  
  // MODIFIERS
  void Set(int x, int y, double v) {
    assert (x >= 0 && x < 4);
    assert (y >= 0 && y < 4);
    data[y][x] = v; }
  void SetToIdentity();
  void Clear();

  void Transpose(Mat &m) const;
  void Transpose() { Transpose(*this); }

  int Inverse(Mat &m, double epsilon = 1e-08) const;
  int Inverse(double epsilon = 1e-08) { return Inverse(*this,epsilon); }

  // OVERLOADED OPERATORS
  Mat& operator=(const Mat& m);
  int operator==(const Mat& m) const;
  int operator!=(const Mat &m) const { return !(*this==m); }
  friend Mat operator+(const Mat &m1, const Mat &m2);
  friend Mat operator-(const Mat &m1, const Mat &m2);
  friend Mat operator*(const Mat &m1, const Mat &m2);
  friend Mat operator*(const Mat &m1, double f);
  friend Mat operator*(double f, const Mat &m) { return m * f; }
  Mat& operator+=(const Mat& m) { *this = *this + m; return *this; }
  Mat& operator-=(const Mat& m) { *this = *this - m; return *this; }
  Mat& operator*=(const double f)   { *this = *this * f; return *this; }
  Mat& operator*=(const Mat& m) { *this = *this * m; return *this; }

  // TRANSFORMATIONS
  static Mat MakeTranslation(const Vec3f &v);
  static Mat MakeScale(const Vec3f &v);
  static Mat MakeScale(double s) { return MakeScale(Vec3f(s,s,s)); }
  static Mat MakeXRotation(double theta);
  static Mat MakeYRotation(double theta);
  static Mat MakeZRotation(double theta);
  static Mat MakeAxisRotation(const Vec3f &v, double theta);

  // Use to transform a point with a matrix
  // that may include translation
  void Transform(Vec4f &v) const;
  void Transform(Vec3f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),v.z(),1);
    Transform(v2);
    v.Set(v2.x(),v2.y(),v2.z()); }
  void Transform(Vec2f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),1,1);
    Transform(v2);
    v.Set(v2.x(),v2.y()); }

  // Use to transform the direction of the ray
  // (ignores any translation)
  void TransformDirection(Vec3f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),v.z(),0);
    Transform(v2);
    v.Set(v2.x(),v2.y(),v2.z()); }

  // INPUT / OUTPUT
  void Write(FILE *F = stdout) const;
  void Write3x3(FILE *F = stdout) const;
  void Read(FILE *F);
  void Read3x3(FILE *F);

  static double det4x4(double a1, double a2, double a3, double a4, 
                       double b1, double b2, double b3, double b4, 
                       double c1, double c2, double c3, double c4, 
                       double d1, double d2, double d3, double d4);
  static double det3x3(double a1,double a2,double a3,
                       double b1,double b2,double b3,
                       double c1,double c2,double c3);
  static double det2x2(double a, double b,
                       double c, double d);

private:

  // REPRESENTATION
  double	data[4][4];

};

// ====================================================================
// ====================================================================

#endif
