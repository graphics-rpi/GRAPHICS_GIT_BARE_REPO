#ifndef SPHERICAL_H_INCLUDED_
#define SPHERICAL_H_INCLUDED_

#include <Vector3.h>
#include <Matrix3.h>

const double pi = 3.1415926535897932384626433832795029L;

double RadiansToDegrees(double radians){
  return 180. * radians / pi;
}

double DegreesToRadians(double degrees){
  return pi * degrees / 180.;
}

Vector3<double> CartesianToSpherical(const Vector3<double> &point){
  double rho = point.length();
  double theta = atan2(point.y(), point.x());
  if (theta < 0.){
    theta += 2. * pi;
  }
  double phi = acos(point.z() / rho);
  return Vector3<double>(rho, theta, phi);    
}

Vector3<double> SphericalToCartesian(const Vector3<double> &point){
  double rho = point(0);
  double theta = point(1);
  double phi = point(2);
  double sin_phi = sin(phi);

  Vector3<double> xyz(rho * cos(theta) * sin_phi,
                      rho * sin(theta) * sin_phi,
                      rho * cos(phi));
  return xyz;
}

// from: http://en.wikipedia.org/wiki/Rotation_matrix#Axis_and_angle
Matrix3<double> AxisAngleRotation(const Vector3<double> &axis, double angle){
  double x = axis.x();
  double y = axis.y();
  double z = axis.z();
  double c = cos(angle);
  double s = sin(angle);
  double C = 1-c;
  double xs = x*s;
  double ys = y*s;
  double zs = z*s;
  double xC = x*C;
  double yC = y*C;
  double zC = z*C;
  double xyC = x*yC;
  double yzC = y*zC;
  double zxC = z*xC;
  return Matrix3<double>(x*xC+c, xyC-zs, zxC+ys,
			 xyC+zs, y*yC+c, yzC-xs,
			 zxC-ys, yzC+xs, z*zC+c);
}

Matrix3<double> RotationMatrix(double ax, double ay, double az){
  Matrix3<double> matrix (1., 0., 0.,
                          0., cos(ax), -sin(ax),
                          0., sin(ax), cos(ax));
  matrix = Matrix3<double>(cos(ay), 0., sin(ay),
                           0., 1., 0.,
                           -sin(ay), 0., cos(ay)) * matrix;
  matrix = Matrix3<double>(cos(az), -sin(az), 0.,
                           sin(az), cos(az), 0.,
                           0., 0., 1.) * matrix;
  return matrix;
}

#endif //#ifndef SPHERICAL_H_INCLUDED_
