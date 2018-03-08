#ifndef POLYNOMIAL_H_INCLUDED
#define POLYNOMIAL_H_INCLUDED

class Polynomial {
public:
  //
  // P(x) = c[0] + c[1]*x + c[2]*x^2 + ...
  //

  Polynomial(){
    this->n = 0;
    a = 0;
  }

  Polynomial(int n, const double *c){
    this->n = n;
    a = new double[n];
    for (int i=0; i<n; i++){
      a[i] = c[i];
    }
  }
  Polynomial(const Polynomial &poly){
    this->n = poly.n;
    a = new double[n];
    for (int i=0; i<n; i++){
      a[i] = poly.a[i];
    }
  }
  Polynomial& operator= (const Polynomial &poly){
    if (&poly != this){
      if (NULL != a){
	delete [] a;
      }
      this->n = poly.n;
      a = new double[n];
      for (int i=0; i<n; i++){
        a[i] = poly.a[i];
      }
    }
    return *this;
  }
  ~Polynomial(){
    if (NULL != a){
      delete [] a;
    }
  }
  // evaluate with Horner's rule
  double operator()(double x) const {
    double val = a[n-1];
    for (int i=n-2; i>=0; i--){
      val =  a[i] + x*val;
    }
    return val;
  }
private:
  int n;
  double *a;
};

#endif //#ifndef POLYNOMIAL_H_INCLUDED
