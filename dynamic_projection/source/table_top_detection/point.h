#ifndef _POINT_H_
#define _POINT_H_

#include <cstdio>
#include <vector>

struct Point
{
  double row;
  double col;
  Point(){}
  Point(double row, double col){this->row = row; this->col = col;}
  double distance(Point p) const {
    return sqrt((p.row-row)*(p.row-row)+(p.col-col)*(p.col-col));
  }
  void dump() const {printf("%g %g\n", row, col);}

  Point operator+(const Point &p) const { return Point(row+p.row,col+p.col); }

};

inline Point operator*(double d, const Point &p) { return Point(d*p.row,d*p.col); }


struct Vec2f
{
  double x;
  double y;
  Vec2f(){};
  Vec2f(double x, double y){this->x = x; this->y = y;};
  void normalize(){
    double len = sqrt(x*x+y*y);
    x /= len;
    y /= len;
  }
};

// ======================================================================

// a b
// c d
struct Matrix2x2
{
  double a, b, c, d; 
  Matrix2x2(double a, double b, double c, double d)
  {
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
  };
  double trace(){return a+d;};
  double det(){return a*d-b*c;};
  double l1() // eigenvalue (not necessarily leading one !!!)
  {
    return trace()/2 + sqrt(trace()*trace()/4 - det());
  };  
  double l2() // eigenvalue
  {
    return trace()/2 - sqrt(trace()*trace()/4 - det());
  };
  Vec2f evect1() // eigenvector for l1()
  {
    if (c != 0){
      return Vec2f(l1()-d, c);
    } else if (b != 0){
      return Vec2f(b, l1()-d);
    } else {
      return Vec2f(0.0, 1.0);
    }
  };
  Vec2f evect2() // eigenvector for l2()
  {
    if (c != 0){
      return Vec2f(l2()-d, c);
    } else if (b != 0){
      return Vec2f(b, l2()-d);
    } else {
      return Vec2f(1.0, 0.0);
    }
  };  
  Matrix2x2 inverse()
  {
    double ia, ib, ic, id;
    double dt = det();
    ia =  d / dt;
    ib = -b / dt;
    ic = -c / dt;
    id =  a / dt;
    return Matrix2x2(ia, ib, ic, id);
  };
  Vec2f operator*(const Vec2f &v)
  {
    Vec2f temp;
    temp.x = a * v.x + b * v.y;
    temp.y = c * v.x + d * v.y;
    return temp;
  };
};


inline Matrix2x2 point_stats(std::vector<Point> &points, Vec2f &mean)
{
  std::vector<Point>::iterator pt;
  mean = Vec2f(0.0, 0.0);
  int n = points.size();

  for (pt = points.begin(); pt != points.end(); ++pt){
    mean.x += pt->col;
    mean.y += pt->row;
  }
  mean.x /= double(n);
  mean.y /= double(n);

  double sxx = 0.0;
  double sxy = 0.0;
  double syy = 0.0;
  for (pt = points.begin(); pt != points.end(); ++pt){
    sxx += (pt->col - mean.x) * (pt->col - mean.x);
    sxy += (pt->col - mean.x) * (pt->row - mean.y);
    syy += (pt->row - mean.y) * (pt->row - mean.y);
  }

  Matrix2x2 cov(sxx/n, sxy/n, sxy/n, syy/n);
  return cov;
}


#endif 
