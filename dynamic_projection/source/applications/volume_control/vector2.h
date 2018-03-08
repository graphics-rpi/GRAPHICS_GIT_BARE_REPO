#ifndef VECTOR2_H_
#define VECTOR2_H_

#include <cmath>
#include <fstream>

class Vector2 {
public:
  Vector2() {}
  Vector2(double _x, double _y)
    : x(_x),
      y(_y)
  {}

  double x, y;

  Vector2 operator+(const Vector2& rhs) {
    return Vector2(x + rhs.x, y + rhs.y);
  }

  Vector2 operator-(const Vector2& rhs) {
    return Vector2(x - rhs.x, y - rhs.y);
  }

  Vector2 operator*(double factor) const {
    return Vector2(x * factor, y * factor);
  }

  double magnitude(){
    return sqrt(x*x + y*y);
  }

  void normalize(){
    double mag = magnitude();
    x /= mag;
    y /= mag;
  }

  Vector2 normal(){
    return Vector2(y, -x);
  }

  double dot(Vector2& other){
    return x * other.x + y * other.y;
  }

};

/*
std::ostream& operator<<(std::ostream& ostr, const Vector2& v){
  ostr << "(" << v.x << ", " << v.y << ")";
  return ostr;
}
*/

#endif
