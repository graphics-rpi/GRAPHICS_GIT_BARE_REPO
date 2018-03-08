#ifndef VECTOR3_H_INCLUDED_
#define VECTOR3_H_INCLUDED_

#include <algorithm>
#include <cstdio>
#include <cmath>

template <typename T1, typename T2>
struct promotion_traits {
  static T1 t1;
  static T2 t2;  
// BARB: had to put this in so it would compile on Mac
//    this is probably a huge error...  but the gcc with 
//    c++0x is not on mac yet.
#ifdef __APPLE__
  typedef T1 promoted_t;
#else
// Tyler: was told to do this same hack for windows
#ifdef __CYGWIN__
  typedef T1 promoted_t;
#else
// Josh: So it's not working in linux either, trying your hack!
  typedef T1 promoted_t;
  //typedef decltype(t1 + t2) promoted_t;
#endif
#endif
};

template <class Type>
struct Vector3
{
  Vector3(){v[0] = v[1] = v[2] = Type(0);}
  Vector3(Type const &val){v[0] = v[1] = v[2] = val;}
  Vector3(Type const &v0, Type const &v1, Type const &v2){
    v[0] = v0; v[1] = v1; v[2] = v2;
  }
  Vector3(Type const *val){
    for (int i=0;i<3;i++){
      v[i] = val[i];
    }
  }

  template<typename T>
  Vector3(const Vector3<T> &a){
    for (int i=0; i<3; i++){
      v[i] = Type(a.v[i]);
    }
  }

  // ADDED BECAUSE 
  //Type& operator[](int i) { return v[i]; }

  Vector3<Type>& operator=(const Type &val){
    v[0] = v[1] = v[2] = Type(val);
    return *this;
  }
    
  bool operator== (const Vector3<Type> &a) const {
    for (int i=0; i<3; i++){
      if (!(v[i] == a.v[i])){
        return false;
      } 
    }
    return true;
  }

  bool operator!= (const Vector3<Type> &a) const {
    return !operator==(a);
  }

  //!!! TODO - convert these operators to use <<, >> and iostream
  // so that they work with vectors of whatever
  // read ASCII representation of vector from file
  void read(FILE *fp){
    double val;
    // note everything read as doubles, then converted, if needed
    for (int i=0; i<3;i++){
      fscanf(fp, "%lf", &val);
      v[i] = Type(val);
    }
  }

  // write ASCII representation of vector to file
  void write(FILE *fp){
    double val;
    // note everything converted to double, then printed
    for (int i=0; i<3;i++){
      val = double(v[i]);
      fprintf(fp, "%lf ", val);
    }
    fprintf(fp, "\n");
  }

  Type &operator()(int i){return v[i];}
  Type operator()(int i) const {return v[i];}

  // accessors for xyz points
  Type &x(){return v[0];}
  Type x() const {return v[0];}
  Type &y(){return v[1];}
  Type y() const {return v[1];}
  Type &z(){return v[2];}
  Type z() const {return v[2];}
  
  // accessors for polar/spherical coordinates
  Type &rho(){return v[0];}
  Type rho() const {return v[0];}
  Type &theta(){return v[1];}
  Type theta() const {return v[1];}
  Type &phi(){return v[2];}
  Type phi() const {return v[2];}

  // accessors for rgb points
  Type &r(){return v[0];}
  Type r() const {return v[0];}
  Type &g(){return v[1];}
  Type g() const {return v[1];}
  Type &b(){return v[2];}
  Type b() const {return v[2];}

  Vector3<Type> operator-(){
    Vector3<Type> temp;
    for (int i=0; i < 3; i++) {temp.v[i] = -v[i];}
    return temp;    
  }

  Type dot(const Vector3<Type> &a) const {
    Type sum = Type(0);
    for (int i=0; i<3; i++){
      sum += v[i] * a.v[i];
    }
    return sum;
  }

  Vector3<Type> cross(const Vector3<Type> &a) const {
    return Vector3<Type>(y()*a.z() - a.y()*z(),
			 a.x()*z() - x()*a.z(),
			 x()*a.y() - a.x()*y());
  }

#if 0
  // !! todo
  // separate declaration headers from implementation headers for matrix, vector
  // to get this to work
  Matrix3<Type> outer(const Vector3<Type> &a) const {
    Matrix3<Type> mat;
    for (int r=0; r<3; r++){
      for (int c=0; c<3; c++){
	mat(r,c) = v[r] * a[c];
      }
    }
    return mat;
  }
#endif

  Type length() const {
    return (Type)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  }

  Type normalize(){
    Type l = length();
    if (l != Type(0)){
      v[0] /= l;
      v[1] /= l;
      v[2] /= l;
    }
    return l;
  }
  
  // apply a scalar function to each component (idependently)
  Vector3<Type> &apply(Type (*func)(Type)){
    for (int i=0; i<3; i++){
      v[i] = func(v[i]);
    }
    return *this;
  }

  // return minimum element
  Type min() const {
    Type m = v[0];
    for (int i=1;i<3;i++){
      if (v[i] < m) m = v[i];
    }
    return m;
  }

  // return middle element
  Type mid() const {
    Type m[3];
    for (int i=0; i<3; i++) m[i] = v[i];
    for (int i=0; i<3; i++){
      for (int j=i+1;j<3;j++){
	if (m[i] > m[j]){
	  Type temp = m[i];
	  m[i] = m[j];
	  m[j] = temp;
	}
      }
    }
    return m[1];
  }

  // return maximum element
  Type max() const {
    Type m = v[0];
    for (int i=1;i<3;i++){
      if (v[i] > m) m = v[i];
    }
    return m;
  }

  template <typename T1, typename T2> friend
  Vector3<typename promotion_traits<T1, T2>::promoted_t>
  operator+(const Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator+=(Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1> friend
  Vector3<T1>
  operator+(const Vector3<T1> &a, const T1 &b);

  template <typename T1> friend
  Vector3<T1>
  operator+(const T1 &a, const Vector3<T1> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator+=(Vector3<T1> &a, const T2 &b);

  template <typename T1, typename T2> friend
  Vector3<typename promotion_traits<T1, T2>::promoted_t>
  operator-(const Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator-=(Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1> friend
  Vector3<T1>
  operator-(const Vector3<T1> &a, const T1 &b);

  template <typename T1> friend
  Vector3<T1>
  operator-(const T1 &a, const Vector3<T1> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator-=(Vector3<T1> &a, const T2 &b);

  template <typename T1, typename T2> friend
  Vector3<typename promotion_traits<T1, T2>::promoted_t>
  operator*(const Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator*=(Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1> friend
  Vector3<T1>
  operator*(const Vector3<T1> &a, const T1 &b);

  template <typename T1> friend
  Vector3<T1>
  operator*(const T1 &a, const Vector3<T1> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator*=(Vector3<T1> &a, const T2 &b);

  template <typename T1, typename T2> friend
  Vector3<typename promotion_traits<T1, T2>::promoted_t>
  operator/(const Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1, typename T2> friend
  Vector3<T1>&
  operator/=(Vector3<T1> &a, const Vector3<T2> &b);

  template <typename T1> friend
  Vector3<T1>
  operator/(const Vector3<T1> &a, const T1 &b);

  template <typename T1> friend
  Vector3<T1>
  operator/(const T1 &a, const Vector3<T1> &b);

  template <typename T1> friend
  Vector3<T1>&
  operator/=(Vector3<T1> &a, const T1 &b);

  template<class T> friend struct Vector3;  
  template<class T> friend struct Matrix3;
private:
  Type v[3];
};

template <typename T>
Vector3<T> min(const Vector3<T> &a, const Vector3<T> &b){
  return Vector3<T>(std::min(a.x(), b.x()), 
                    std::min(a.y(), b.y()), 
                    std::min(a.z(), b.z()));
}

template <typename T>
Vector3<T> max(const Vector3<T> &a, const Vector3<T> &b){
  return Vector3<T>(std::max(a.x(), b.x()), 
                    std::max(a.y(), b.y()), 
                    std::max(a.z(), b.z()));
}

/*
template <class T1, class T2>
Vector3<T2> convertVector3(const Vector3<T1> &src, T2 dst_type)
  __attribute__ ((deprecated));

template <class T1, class T2>
Vector3<T2> convertVector3(const Vector3<T1> &src, T2 dst_type)
{
  Vector3<T2> temp;
  for (int i=0; i<3; i++){
    temp(i) = T2(src(i));
  }
  return temp;
}
*/
/*
template <class T1, class T2>
T2 convertVector3(const T1 &src, T2 dst_type)
  __attribute__ ((deprecated));

template <class T1, class T2>
T2 convertVector3(const T1 &src, T2 dst_type)
{
  return T2(src);
}
*/

template <typename T1, typename T2>
Vector3<typename promotion_traits<T1, T2>::promoted_t>
operator+(const Vector3<T1> &a, const Vector3<T2> &b){
  typedef typename promotion_traits<T1, T2>::promoted_t promoted_t;
  Vector3<promoted_t> result;
  for (int i=0;i<3;i++){
    result(i) = ( promoted_t(a(i)) + 
                  promoted_t(b(i)) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<T1>&
operator+=(Vector3<T1> &a, const Vector3<T2> &b){
  for (int i=0;i<3;i++){
    a(i) += T1(b(i));
  }
  return a;
}

template <typename T1>
Vector3<T1>
operator+(const Vector3<T1> &a, const T1 &b){
  Vector3<T1> result;
  for (int i=0;i<3;i++){
    result(i) = ( a(i) + 
                  b );
  }
  return result;
}

template <typename T1>
Vector3<T1>
operator+(const T1 &a, const Vector3<T1> &b){
  Vector3<T1> result;
  for (int i=0;i<3;i++){
    result(i) = ( a + 
                  b(i) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<T1>&
operator+=(Vector3<T1> &a, const T2 &b){
  for (int i=0;i<3;i++){
    a(i) += T1(b);
  }
  return a;
}

template <typename T1, typename T2>
Vector3<typename promotion_traits<T1, T2>::promoted_t>
operator-(const Vector3<T1> &a, const Vector3<T2> &b){
  typedef typename promotion_traits<T1, T2>::promoted_t promoted_t;
  Vector3<promoted_t> result;
  for (int i=0;i<3;i++){
    result(i) = ( promoted_t(a(i)) - 
                  promoted_t(b(i)) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<T1>&
operator-=(Vector3<T1> &a, const Vector3<T2> &b){
  for (int i=0;i<3;i++){
    a(i) -= T1(b(i));
  }
  return a;
}

template <typename T1, typename T2>
Vector3<typename promotion_traits<T1, T2>::promoted_t>
operator-(const Vector3<T1> &a, const T2 &b){
  typedef typename promotion_traits<T1, T2>::promoted_t promoted_t;
  Vector3<promoted_t> result;
  for (int i=0;i<3;i++){
    result(i) = ( promoted_t(a(i)) - 
                  promoted_t(b) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<typename promotion_traits<T1, T2>::promoted_t>
operator-(const T1 &a, const Vector3<T2> &b){
  typedef typename promotion_traits<T1, T2>::promoted_t promoted_t;
  Vector3<promoted_t> result;
  for (int i=0;i<3;i++){
    result(i) = ( promoted_t(a) - 
                  promoted_t(b(i)) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<T1>&
operator-=(Vector3<T1> &a, const T2 &b){
  for (int i=0;i<3;i++){
    a(i) -= T1(b);
  }
  return a;
}

// note this is element-wise multiplication
// equivalent to .* MATLAB operator
template <typename T1, typename T2>
Vector3<typename promotion_traits<T1, T2>::promoted_t>
operator*(const Vector3<T1> &a, const Vector3<T2> &b){
  typedef typename promotion_traits<T1, T2>::promoted_t promoted_t;
  Vector3<promoted_t> result;
  for (int i=0;i<3;i++){
    result(i) = ( promoted_t(a(i)) * 
                  promoted_t(b(i)) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<T1>&
operator*=(Vector3<T1> &a, const Vector3<T2> &b){
  for (int i=0;i<3;i++){
    a(i) *= T1(b(i));
  }
  return a;
}

template <typename T1>
Vector3<T1>
operator*(const Vector3<T1> &a, const T1 &b){
  Vector3<T1> result;
  for (int i=0;i<3;i++){
    result(i) = ( a(i) * 
                  b );
  }
  return result;
}

template <typename T1>
Vector3<T1>
operator*(const T1 &a, const Vector3<T1> &b){
  Vector3<T1> result;
  for (int i=0;i<3;i++){
    result(i) = ( a * 
                  b(i) );
  }
  return result;
}

template <typename T1, typename T2>
Vector3<T1>&
operator*=(Vector3<T1> &a, const T2 &b){
  for (int i=0;i<3;i++){
    a(i) *= T1(b);
  }
  return a;
}

// note this is element-wise division
// equivalent to ./ MATLAB operator
template <typename T1, typename T2>
Vector3<typename promotion_traits<T1, T2>::promoted_t>
operator/(const Vector3<T1> &a, const Vector3<T2> &b){
  typedef typename promotion_traits<T1, T2>::promoted_t promoted_t;
  Vector3<promoted_t> result;
  for (int i=0;i<3;i++){
    result(i) = ( promoted_t(a(i)) / 
                  promoted_t(b(i)) );
  }
  return result;
}

// note this is element-wise division
// equivalent to ./ MATLAB operator
template <typename T1, typename T2>
Vector3<T1>&
operator/=(Vector3<T1> &a, const Vector3<T2> &b){
  for (int i=0;i<3;i++){
    a(i) /= T1(b(i));
  }
  return a;
}

template <typename T1>
Vector3<T1>
operator/(const T1 &a, const Vector3<T1> &b){
  Vector3<T1> result;
  for (int i=0;i<3;i++){
    result(i) = ( a / 
                  b(i) );
  }
  return result;
}

template <typename T1>
Vector3<T1>
operator/(const Vector3<T1> &a, const T1 &b){
  Vector3<T1> result;
  for (int i=0;i<3;i++){
    result(i) = ( a(i) / 
                  b );
  }
  return result;
}

// note this is element-wise division
// equivalent to ./ MATLAB operator
template <typename T1>
Vector3<T1>&
operator/=(Vector3<T1> &a, const T1 &b){
  for (int i=0;i<3;i++){
    a(i) /= b;
  }
  return a;
}





// typedefs for commonly-used types
typedef Vector3<double> v3d;
typedef Vector3<float>  v3f;

#endif // #ifndef VECTOR3_H_INCLUDED_
