#ifndef SOLAR_SYSTEM_INCLUDED_
#define SOLAR_SYSTEM_INCLUDED_

#include <Vector3.h>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include "spherical.h"

struct Date {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  double second;

  Date(){
  }

  Date(const Date &d){
    year = d.year;
    month = d.month;
    day = d.day;
    hour = d.hour;
    minute = d.minute;
    second = d.second;
  }

  Date(int y, int mo, int d, int h, int mi, double s){
    year = y;
    month = mo;
    day = d;
    hour = h;
    minute = mi;
    second = s;
  }
  
  Date(char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr,"unable to open %s for reading\n", filename);
      exit(-1);
    }
    fscanf(fp, "%d", &year);
    fscanf(fp, "%d", &month);
    fscanf(fp, "%d", &day);
    fscanf(fp, "%d", &hour);
    fscanf(fp, "%d", &minute);
    fscanf(fp, "%lf", &second);
    fclose(fp);
  }

  // hack for now
  void setHMS(int hour, int minute, double second){
    this->hour = hour;
    this->minute = minute;
    this->second = second;
  }

  // from : A physically-base night sky model, SIGGRAPH 2001 (appendix)
  double JulianDate() const {
    int y, m;
    if (month == 1 || month == 2){
      y = year - 1;
      m = month + 12;
    } else {
      y = year;
      m = month;
    }
    return 1720996.5 - floor(y/100) + floor(y/400) + floor(365.25*y) +
      floor(30.6001*(m+1)) + day + 
      (hour + (minute + (second + delta_t())/60.)/60.)/24.;
  }

  // from : A physically-based night sky model, SIGGRAPH 2001 (appendix)
  // centuries from Jan 1, 2000
  double T() const {
    return (JulianDate() - 2451545.0)/36525;
  }

  // return difference between GMT and terrestrial time in seconds
  double delta_t() const {
    return 65.15;
  }
};

Vector3<double> EclipticToCartesian(const Vector3<double> &point){
  double rho = point(0);
  double lat = point(1);
  double lon = point(2);
  double cos_lat = cos(lat);

  Vector3<double> xyz(rho * cos(lon) * cos_lat,
                      rho * sin(lon) * cos_lat,
                      rho * sin(lat));
  return xyz;
}

Vector3<double> EclipticToEquatorial(const Vector3<double> &point,
				     const Date &date){
  double T = date.T();
  // calculate obliquity of ecliptic
  double epsilon = 0.409093 - 0.000227 * T;
  Matrix3<double> mat = RotationMatrix(epsilon, 0., 0.);
  return mat * point;
}

// note: this function takes RECTANGULAR ecliptic coordinates (x,y,z)
//       and converts them to RECTANGULAR equatorial coordinates (x,y,z) 
// from: A physically-based night sky model, SIGGRAPH 2001 (appendix)
// note: latitude, longitude in degrees
Vector3<double> EclipticToHorizon(const Vector3<double> &point,
				  double latitude,
				  double longitude,
				  const Date &date){
  double T = date.T();
  // calculate obliquity of ecliptic
  double epsilon = 0.409093 - 0.000227 * T;
  double lat = DegreesToRadians(latitude);
  double lon = DegreesToRadians(longitude);
  // calculate local sidereal time

  double lmst = 4.894961 + 230121.675315 * 
    (T - date.delta_t()/(60.* 60.* 24. * 36525.)) + lon;

  Matrix3<double> mat = 
    RotationMatrix(0., lat - pi/2, 0.) * 
    RotationMatrix(0., 0., -lmst) *
    RotationMatrix(epsilon, 0., 0.);
  return mat * point;
}

// from: A physically-based night sky model, SIGGRAPH 2001 (appendix)
// note: latitude, longitude in degrees
m3d EclipticToHorizonMatrix(double latitude,
                            double longitude,
                            const Date &date){
  double T = date.T();
  // calculate obliquity of ecliptic
  double epsilon = 0.409093 - 0.000227 * T;
  double lat = DegreesToRadians(latitude);
  double lon = DegreesToRadians(longitude);
  // calculate local sidereal time

  double lmst = 4.894961 + 230121.675315 * 
    (T - date.delta_t()/(60.* 60.* 24. * 36525.)) + lon;

  Matrix3<double> mat = 
    RotationMatrix(0., lat - pi/2, 0.) * 
    RotationMatrix(0., 0., -lmst) *
    RotationMatrix(0., 0., 0.01118*T) *
    RotationMatrix(0., -0.00972*T, 0.) *
    RotationMatrix(0., 0., 0.01118*T) *
    RotationMatrix(epsilon, 0., 0.);
  return mat;
}

// from : A physically-based night sky model, SIGGRAPH 2001 (appendix)  
Vector3<double> SunPosition(const Date& d){
  double T = d.T();
  double M = 6.24 + 628.302 * T;
  double lambda = 4.895048 + 628.331951*T 
    + (0.033417 - 0.000084 * T) * sin(M)
    + 0.000351 * sin(2. * M);
  double rho = 1.000140 - (0.016708 - 0.000042*T)*cos(M) - 
    0.000141*cos(2. * M);
  rho *= 1.49598e11; // convert to meters from au
  double beta = 0.;
  // lambda, beta: ecliptic coordinates    
  Vector3<double> xyz = 
    EclipticToCartesian(Vector3<double>(rho, beta, lambda));
  return xyz;
}

// from : A physically-based night sky model, SIGGRAPH 2001 (appendix)  
Vector3<double> MoonPosition(const Date& date){
  double T = date.T();
  double lp = 3.8104 + 8399.7091 * T;
  double mp = 2.3554 + 8328.6911 * T;
  double m = 6.2300 + 628.3019 * T;
  double d = 5.1985 + 7771.3772 * T;
  double f = 1.6280 + 8433.4663 * T;
  double lambda = lp 
    + 0.1098 * sin(mp)
    + 0.0222 * sin(2*d - mp)
    + 0.0115 * sin(2*d)
    + 0.0037 * sin(2*mp)
    - 0.0032 * sin(m)
    - 0.0020 * sin(2*f)
    + 0.0010 * sin(2*d - 2*mp)
    + 0.0010 * sin(2*d - m - mp)
    + 0.0009 * sin(2*d + mp)
    + 0.0008 * sin(2*d - m)
    + 0.0007 * sin(mp - m)
    - 0.0006 * sin(d)
    - 0.0005 * sin(m + mp);
  double beta = 
    + 0.0895 * sin(f)
    + 0.0049 * sin(mp + f)
    + 0.0048 * sin(mp - f)
    + 0.0030 * sin(2*d - f)
    + 0.0010 * sin(2*d + f - mp)
    + 0.0008 * sin(2*d - f - mp)
    + 0.0006 * sin(2*d + f);
  double pip = 
    + 0.016593 
    + 0.000904 * cos(mp)
    + 0.000166 * cos(2*d - mp)
    + 0.000137 * cos(2*d)
    + 0.000049 * cos(2*mp)
    + 0.000015 * cos(2*d + mp)
    + 0.000009 * cos(2*d - m);
  // from: http://scienceworld.wolfram.com/astronomy/EarthRadius.html
  double earth_radius = 6378.137e3; // meters
  double rho = earth_radius / pip;
  Vector3<double> xyz = 
    EclipticToCartesian(Vector3<double>(rho, beta, lambda));
  return xyz;
}

Matrix3<double> MoonOrientationMatrix(const Date& date){
  Matrix3<double> matrix(1., 0., 0.,
			 0., 1., 0.,
			 0., 0., 1.);

  double T = date.T();
  double lp = 3.8104 + 8399.7091 * T;
  double f = 1.6280 + 8433.4663 * T;

  // orient in ecliptic coordinates
  matrix = RotationMatrix(0., 0., lp - f) * matrix;
  matrix = RotationMatrix(0.026920, 0., 0.) * matrix;
  matrix = RotationMatrix(0., 0., f) * matrix;

  return matrix;
}

// sun
const double solar_photospheric_radius = 695.508e6; // meters

// NB: the Earth *really is* the center of the universe
//     all measurments in meters
const Vector3<double> earth_center (0.0, 0.0, 0.0);
const double earth_surface_radius = 6378.137e3; // meters
//const double atmosphere_cutoff_distance = 100e3; // meters
//const double atmosphere_cutoff_distance = 31e3; // meters: 99% of atmosphere
const double atmosphere_cutoff_distance = 42e3; // meters: 99.9% of atmosphere


const double earth_atmosphere_radius = earth_surface_radius 
  + atmosphere_cutoff_distance;

// moon
const double moon_radius = 1737400.0;
//const double moon_radius = 8737400.0; // huge exaggerated moon for debugging

#endif // #ifndef SOLAR_SYSTEM_INCLUDED_
