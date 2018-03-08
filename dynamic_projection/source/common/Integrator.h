#ifndef INTEGRATOR_INCLUDED_
#define INTEGRATOR_INCLUDED_
#include <util.h>

//
// simple 1D numerical inegrator; replace with good higher-order method
// 8/4/08/tcy

template <typename Function>
class Integrator {
public:
  enum method_t {METHOD_RECTANGLE, METHOD_TRAPEZOID, METHOD_SIMPSON};

  double integrate(const Function &f, double a, double b, 
		   method_t method = METHOD_SIMPSON, 
		   double stepsize = 0.){
    const int default_num_steps = 100;
    if (stepsize == 0.){
      stepsize = (b - a) / default_num_steps;
    }

    switch (method){
    case METHOD_RECTANGLE:
      return rectangle(f, a, b, stepsize);
      break;
    case METHOD_TRAPEZOID:
      return trapezoid(f, a, b, stepsize);
      break;
    case METHOD_SIMPSON:
      return simpson(f, a, b, stepsize);
      break;
    default:
      FATAL_ERROR("Unknown integration method");
    }
  }
private:
  double rectangle(const Function &f, double a, double b, double stepsize){
    double sum = 0.;
    double halfstep = stepsize/2.;
    double x;
    for (x=a; x<b-stepsize; x+=stepsize){
      sum += stepsize * f(x+halfstep);
    }
    sum += (b-x) * f((x+b)/2.);
    return sum;
    }
  
  double trapezoid(const Function &f, double a, double b, double stepsize){
    double sum = 0.;
    double x;
    for (x=a; x<b-stepsize; x+=stepsize){
      sum += stepsize * (f(x) + f(x+stepsize));
    }
    sum += (b-x) * (f(x) + f(b));
    sum /= 2.;
    return sum;
  }
  
  double simpson(const Function &f, double a, double b, double stepsize){
    double sum = 0.;
    double halfstep = stepsize / 2.;
    double x;
    for (x=a; x<b-stepsize; x+=stepsize){      
      sum += stepsize * (f(x) + 4.*f(x+halfstep) + f(x+stepsize));
    }
    sum += (b-x) * (f(x) + 4.*f((x+b)/2.)+ f(b));
    sum /= 6.;
    return sum;
  }
};

#endif // #ifndef INTEGRATOR_INCLUDED_
