//
// originally implemented by Justin Legakis
//

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cassert>

#include "matrix.h"
#include "vectors.h"


// ===================================================================
// ===================================================================
// COPY CONSTRUCTOR

Mat::Mat(const Mat& m) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = m.data[y][x]; }
  }
}

Mat::Mat(const double *m) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = m[4*y+x]; }
  }
}

// ===================================================================
// ===================================================================
// MODIFIERS

void Mat::SetToIdentity() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = (x == y); 
    }
  }
}

void Mat::Clear() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = 0; 
    }
  }
}

void Mat::Transpose(Mat &m) const {
  // be careful, <this> might be <m>
  Mat tmp = Mat(*this);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      m.data[y][x] = tmp.data[x][y];
    }
  }
}

// ===================================================================
// ===================================================================
// INVERSE

int Mat::Inverse(Mat &m, double epsilon) const {
  m = *this;

  double a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;
  a1 = m.data[0][0]; b1 = m.data[0][1]; c1 = m.data[0][2]; d1 = m.data[0][3];
  a2 = m.data[1][0]; b2 = m.data[1][1]; c2 = m.data[1][2]; d2 = m.data[1][3];
  a3 = m.data[2][0]; b3 = m.data[2][1]; c3 = m.data[2][2]; d3 = m.data[2][3];
  a4 = m.data[3][0]; b4 = m.data[3][1]; c4 = m.data[3][2]; d4 = m.data[3][3];

  double det = det4x4(a1,a2,a3,a4,b1,b2,b3,b4,c1,c2,c3,c4,d1,d2,d3,d4);

  if (fabs(det) < epsilon) {
    printf ("Mat::Inverse --- singular matrix, can't invert!\n");
    assert (0);
    return 0;
  }

  m.data[0][0] =   det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
  m.data[1][0] = - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
  m.data[2][0] =   det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
  m.data[3][0] = - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);

  m.data[0][1] = - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
  m.data[1][1] =   det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
  m.data[2][1] = - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
  m.data[3][1] =   det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
  
  m.data[0][2] =   det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
  m.data[1][2] = - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
  m.data[2][2] =   det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
  m.data[3][2] = - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
  
  m.data[0][3] = - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
  m.data[1][3] =   det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
  m.data[2][3] = - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
  m.data[3][3] =   det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
  
  m *= 1/det;
  return 1;
}

double Mat::det4x4(double a1, double a2, double a3, double a4, 
		     double b1, double b2, double b3, double b4, 
		     double c1, double c2, double c3, double c4, 
		     double d1, double d2, double d3, double d4) {
  return 
      a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
    - b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
    + c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
    - d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

double Mat::det3x3(double a1,double a2,double a3,
		     double b1,double b2,double b3,
		     double c1,double c2,double c3) {
  return
      a1 * det2x2( b2, b3, c2, c3 )
    - b1 * det2x2( a2, a3, c2, c3 )
    + c1 * det2x2( a2, a3, b2, b3 );
}

double Mat::det2x2(double a, double b,
		     double c, double d) {
  return a * d - b * c;
}

// ===================================================================
// ===================================================================
// OVERLOADED OPERATORS

Mat& Mat::operator=(const Mat& m) {
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      data[y][x] = m.data[y][x]; 
    }
  }
  return (*this); 
}

int Mat::operator==(const Mat& m) const {
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      if (this->data[y][x] != m.data[y][x]) {
	return 0; 
      }
    }
  }
  return 1; 
}

Mat operator+(const Mat& m1, const Mat& m2) {
  Mat answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      answer.data[y][x] = m1.data[y][x] + m2.data[y][x];
    }
  }
  return answer; 
}

Mat operator-(const Mat& m1, const Mat& m2) {
  Mat answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      answer.data[y][x] = m1.data[y][x] - m2.data[y][x];
    }
  }
  return answer; 
}

Mat operator*(const Mat& m1, const Mat& m2) {
  Mat answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      for (int i=0; i<4; i++) {
	answer.data[y][x] 
	  += m1.data[y][i] * m2.data[i][x];
      }
    }
  }
  return answer;
}

Mat operator*(const Mat& m, double f) {
  Mat answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      answer.data[y][x] = m.data[y][x] * f;
    }
  }
  return answer;
}

// ====================================================================
// ====================================================================
// TRANSFORMATIONS

Mat Mat::MakeTranslation(const Vec3f &v) {
  Mat t;
  t.SetToIdentity();
  t.data[0][3] = v.x();
  t.data[1][3] = v.y();
  t.data[2][3] = v.z();
  return t;
}

Mat Mat::MakeScale(const Vec3f &v) {
  Mat s; 
  s.SetToIdentity();
  s.data[0][0] = v.x();
  s.data[1][1] = v.y();;
  s.data[2][2] = v.z();
  s.data[3][3] = 1;
  return s;
}

Mat Mat::MakeXRotation(double theta) {
  Mat rx;
  rx.SetToIdentity();
  rx.data[1][1]= (double)cos((double)theta);
  rx.data[1][2]=-(double)sin((double)theta);
  rx.data[2][1]= (double)sin((double)theta);
  rx.data[2][2]= (double)cos((double)theta);
  return rx;
}

Mat Mat::MakeYRotation(double theta) {
  Mat ry;
  ry.SetToIdentity();
  ry.data[0][0]= (double)cos((double)theta);
  ry.data[0][2]= (double)sin((double)theta);
  ry.data[2][0]=-(double)sin((double)theta);
  ry.data[2][2]= (double)cos((double)theta);
  return ry;
}

Mat Mat::MakeZRotation(double theta) {
  Mat rz;
  rz.SetToIdentity();
  rz.data[0][0]= (double)cos((double)theta);
  rz.data[0][1]=-(double)sin((double)theta);
  rz.data[1][0]= (double)sin((double)theta);
  rz.data[1][1]= (double)cos((double)theta);
  return rz;
}

Mat Mat::MakeAxisRotation(const Vec3f &v, double theta) {
  Mat r;
  r.SetToIdentity();

  double x = v.x(); double y = v.y(); double z = v.z();

  double c = cosf(theta);
  double s = sinf(theta);
  double xx = x*x;
  double xy = x*y;
  double xz = x*z;
  double yy = y*y;
  double yz = y*z;
  double zz = z*z;

  r.Set(0,0, (1-c)*xx + c);
  r.Set(0,1, (1-c)*xy + z*s);
  r.Set(0,2, (1-c)*xz - y*s);
  r.Set(0,3, 0);

  r.Set(1,0, (1-c)*xy - z*s);
  r.Set(1,1, (1-c)*yy + c);
  r.Set(1,2, (1-c)*yz + x*s);
  r.Set(1,3, 0);

  r.Set(2,0, (1-c)*xz + y*s);
  r.Set(2,1, (1-c)*yz - x*s);
  r.Set(2,2, (1-c)*zz + c);
  r.Set(2,3, 0);

  r.Set(3,0, 0);
  r.Set(3,1, 0);
  r.Set(3,2, 0);
  r.Set(3,3, 1);

  return r;
}

// ====================================================================
// ====================================================================

void Mat::Transform(Vec4f &v) const {
  Vec4f answer;
  for (int y=0; y<4; y++) {
    answer.data[y] = 0;
    for (int i=0; i<4; i++) {
      answer.data[y] += data[y][i] * v[i];
    }
  }
  v = answer;
}

// ====================================================================
// ====================================================================

void Mat::Write(FILE *F) const {
  assert (F != NULL);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      double tmp = data[y][x];
      if (fabs(tmp) < 0.00001) tmp = 0;
      fprintf (F, "%12.6f ", tmp);
    }
    fprintf (F,"\n"); 
  } 
}

void Mat::Write3x3(FILE *F) const {
  assert (F != NULL);
  for (int y = 0; y < 4; y++) {
    if (y == 2) continue;
    for (int x = 0; x < 4; x++) {
      if (x == 2) continue;
      double tmp = data[y][x];
      if (fabs(tmp) < 0.00001) tmp = 0;
      fprintf (F, "%12.6f ", tmp);
    }
    fprintf (F,"\n"); 
  } 
}

void Mat::Read(FILE *F) {
  assert (F != NULL);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      int scanned = fscanf (F,"%lf",&data[y][x]);
      assert (scanned == 1); 
    }
  } 
}

void Mat::Read3x3(FILE *F) {
  assert (F != NULL);
  Clear();
  for (int y = 0; y < 4; y++) {
    if (y == 2) continue;
    for (int x = 0; x < 4; x++) {
      if (x == 2) continue;
      int scanned = fscanf (F,"%lf",&data[y][x]);
      assert (scanned == 1); 
    } 
  } 
}

// ====================================================================
// ====================================================================
