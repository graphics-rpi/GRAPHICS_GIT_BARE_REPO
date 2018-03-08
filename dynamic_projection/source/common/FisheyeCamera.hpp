#ifndef FISHEYE_CAMERA_HPP_INCLUDED_
#define FISHEYE_CAMERA_HPP_INCLUDED_
#include "Vector3.h"
#include "Matrix3.h"
#include "Polynomial.h"
#include <fstream>
#include <string>
#include <iostream>

class FisheyeCamera {
public:
  FisheyeCamera(){
    // initialize polynomial for error estimation (see desc. below)
    double error_coefs[4] = 
      {  1.572689424827184e-03,
	 2.202612683352972e-06,
        -7.831279598878904e-09,
	 2.807811258461925e-11};
    error_function = Polynomial(4,  error_coefs);

    // init camera intrinsics

#if 0
    // intrinsics from January 2010
    xc = 4.490210874557817e+02;
    yc = 6.293267113612636e+02;
    a0 = -6.578447201102779e+02;
    a1 =  0.;
    a2 =  6.376875611760997e-04;
    a3 = -1.872325440160610e-07;
    a4 =  5.058608245455292e-10;
    Ainv = m3d( 1.000082393428760e+00, -8.996866581596869e-05, 0.,
		-5.646933160641177e-05, 1.000000005080052e+00, 0.,
		0., 0., 0.);

    // intrinsics from December 2010
    xc = 4.216230678578523e+02;
    yc = 6.317737210242257e+02;
    a0 = -8.259864173648920e+02;
    a1 = 0;
    a2 = 2.819738933572939e-04;
    a3 = 6.405341754388564e-07;
    a4 = -5.319423507848856e-10;

    Ainv = m3d(0.999780171227607,  -0.000042515739880, 0,
	       0.000125246115881,   0.999999994673898, 0,
	       0, 0, 0);
#else
    // default intrinsics (ir camera, December 2010)
    xc = 4.411827247064227e+02;
    yc = 6.435628359270908e+02;
    a0 = -8.203952256111920e+02;
    a1 = 0;
    a2 = 4.297247137145188e-04;
    a3 = 8.507398739173784e-08;
    a4 = 6.476841968785985e-11;

    Ainv = m3d(0.999227318947195,   0.000159304458230, 0,
	       0.000118680572019,   1.000000018920964, 0,
	       0, 0, 0);

#endif
    
    // default extrinsics (ir camera, December 2010)
    R = m3d(-0.549373688573636,  -0.051694165019690,   0.833976176881519,
	    -0.057159634240136,   0.998071410999463,   0.024212285292276,
	    -0.833619413471478,  -0.034368180755720,  -0.551268992085543);
    t = v3d(2.1753,    0.2924,   -6.8428);

  }

  FisheyeCamera(const char* filename){
    double error_coefs[4] = 
      {  1.572689424827184e-03,
	 2.202612683352972e-06,
        -7.831279598878904e-09,
	 2.807811258461925e-11};
    error_function = Polynomial(4,  error_coefs);
    loadCalibrationFile(filename);
  }

  void loadCalibrationFile(const char* filename){
    std::ifstream fin(filename);
    std::string s;

    // intrinsics
    // read in (xc, yc) coordinates
    fin >> s >> s >> xc;
    fin >> s >> s >> yc;

    // read in coefficients of f(rho)
    fin >> s >> s >> a0;
    fin >> s >> s >> a1;
    fin >> s >> s >> a2;
    fin >> s >> s >> a3;
    fin >> s >> s >> a4;

    std::cout << "xc = " << xc << std::endl;
    std::cout << "yc = " << yc << std::endl;

    std::cout << "a0 = " << a0 << std::endl;
    std::cout << "a1 = " << a1 << std::endl;
    std::cout << "a2 = " << a2 << std::endl;
    std::cout << "a3 = " << a3 << std::endl;
    std::cout << "a4 = " << a4 << std::endl;

    // read in Ainv
    fin >> s;
    std::cout << "Ainv = " << std::endl;
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
	fin >> Ainv(i,j);
	std::cout << Ainv(i,j) << " ";
      }
      std::cout << std::endl;
    }
    
    // extrinsics
    // read in R
    fin >> s;
    std::cout << "R = " << std::endl;
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
	fin >> R(i,j);
	std::cout << R(i,j) << " ";
      }
      std::cout << std::endl;
    }
    
    // read in t
    fin >> s;
    std::cout << "t = ";
    for(int i = 0; i < 3; i++){
      fin >> t(i);
      std::cout << t(i) << " ";
    }
    std::cout << std::endl;

    fin.close();
  }

  // return the estimated error (in meters) for a given image point
  double PointError(double row, double col, double height){
    double r = sqrt((row-yc)*(row-yc) + (col-xc)*(col-xc));
    return 2.5*error_function(r);
  }

  // calculate world coordinates for a pixel at a known height above the floor
  //   by back-projection

  v3d GetView(double row, double col){
    double x = row - xc;
    double y = col - yc;
    v3d xy0 = Ainv * v3d(x, y, 0.);
    x = xy0.x();
    y = xy0.y();
    double r = sqrt(x*x + y*y);
    double z = a0 + r * (a1 + r * (a2 + r * (a3 + r * a4)));
    v3d view(x,y,z);
    view.normalize();
    return view;
  }

  v3d PixelToWorldFromX(double row, double col, double xpos){
    v3d view = GetView(row, col);
    view = R.transpose() * view;
    v3d center = -(R.transpose() * t);
    double l = (xpos - center.x())/view.x();
    v3d world_point = center + l * view;
    //    world_point += v3d(0., 0., 0.033); // led marker height offset
    // transforms camera to projector coords
    /*
    m3d transform( 0.,   1.,  0.,
		   0.,   0.,  -1.,
		   -1.,   0,   0.);
    
    return transform * world_point;
    */
    return world_point;
  }

  v3d PixelToWorldFromY(double row, double col, double ypos){
    v3d view = GetView(row, col);
    view = R.transpose() * view;
    v3d center = -(R.transpose() * t);
    double l = (ypos - center.y())/view.y();
    v3d world_point = center + l * view;
    //    world_point += v3d(0., 0., 0.033); // led marker height offset
    // transforms camera to projector coords
    /*
    m3d transform( 0.,   1.,  0.,
		   0.,   0.,  -1.,
		   -1.,   0,   0.);
    return transform * world_point;
    */
    return world_point;
  }

  v3d PixelToWorldFromZ(double row, double col, double zpos){
    v3d view = GetView(row, col);
    view = R.transpose() * view;
    v3d center = -(R.transpose() * t);
    double l = (zpos - center.z())/view.z();
    v3d world_point = center + l * view;
    //    world_point += v3d(0., 0., 0.033); // led marker height offset
    // transforms camera to projector coords
    /*
    m3d transform( 0.,   1.,  0.,
		   0.,   0.,  -1.,
		   -1.,   0,   0.);
    return transform * world_point;
    */
    return world_point;
  }

  v3d PixelToWorld(double row, double col, double height){
    return PixelToWorldFromZ(row, col, height);
  }

#if 0
  // old, hard-coded PixelToWorld
  v3d PixelToWorld(double row, double col, double height){
    double x = row - xc;
    double y = col - yc;
    v3d xy0 = Ainv * v3d(x, y, 0.);
    x = xy0.x();
    y = xy0.y();
    double r = sqrt(x*x + y*y);
    double z = a0 + r * (a1 + r * (a2 + r * (a3 + r * a4)));
    v3d view(x,y,z);
    view.normalize();

#if 0
    // extrinsics from January 2010
    m3d R(-0.998167838048340,   0.007784760488182,   0.060003038173457,
          0.007808744716535,   0.999969497635940,   0.000165238249486,
          -0.059999921598746,   0.000633483913565,  -0.998198180776882);
    v3d t(0.775196326948624,  -0.050678105574938,  -9.750197422076880);

    // extrinsics from December 2010
    m3d R(-0.187865704975193,  -0.475165645356821,  -0.859606937132786,
	  0.973858675263240,  -0.203896641334520,  -0.100127120536994,
	  -0.127693999501548,  -0.855946125130045,   0.501049372184180);
    v3d t(1.112988553149364, 0.372917799870921,  -9.264404041153853);

    // *****************************************************************
    // extrinsics from December 2010 for above camera
    m3d R(-0.014386819900577,  -0.995090523979934,  -0.097917661831197,
	  0.999890976895615,  -0.013991975017057,  -0.004717940001139,
	  0.003324715909768,  -0.097974862696829,   0.995183336146489);
    v3d t(0.090298542687999, 0.553037384069395, -5.633014723108115);

    // x not flipped, y not flipped
    m3d R(-0.015146944414832,  -0.994935175023866,  -0.099370858782253,
	  0.999800204768185,  -0.013774280015232,  -0.014485156387075,
	  0.013043029571001,  -0.099570410817122,   0.994945029973676);
    v3d t(0.087168934150160,   0.551381726309317,  -5.633496654293230);

    // x flipped
    m3d R(0.014384706824474,  -0.995089406286806,   0.097929330158775,
	  -0.999891057743575,  -0.013991306983373,   0.004702762308759,
	  -0.003309509632803,  -0.097986309373654,  -0.995182259850587);
    v3d t(1.923266824080434, -0.335637821884577, -5.456862702978236);

    // y flipped
    m3d R(-0.014384703064782,   0.995089405851095,   0.097929335138422,
	  0.999891058114389,   0.013991312595911,   0.004702666767959,
	  0.003309413939813,   0.097986312997065,  -0.995182259812049);
    v3d t(1.923266838314991,  -0.335637772173965,  -5.456862587698139);

    // xy swapped
    m3d R(-0.995089408230112,  -0.014384701409606,   0.097929311207633,
	  -0.013991306446305,   0.999891057982314,   0.004702713145962,
	  -0.097986289715299,   0.003309461038310,  -0.995182261947763);
    v3d t(1.923266838525481,  -0.335637803275440,  -5.456862683688069);

    // *****************************************************************

    // current ir extrinsics
    m3d R(-0.549373688573636,  -0.051694165019690,   0.833976176881519,
	  -0.057159634240136,   0.998071410999463,   0.024212285292276,
	  -0.833619413471478,  -0.034368180755720,  -0.551268992085543);
    v3d t(2.1753,    0.2924,   -6.8428);
    
#endif

    view = R.transpose() * view;
    v3d center = R.transpose() * t;
    double l = (height - center.z())/view.z();
    v3d world_point = center + l * view;
    //    world_point += v3d(0., 0., 0.033); // led marker height offset
    // transforms camera to projector coords
    m3d transform( 0.,   1.,  0.,
		   0.,   0.,  -1.,
		   -1.,   0,   0.);
    return transform * world_point;
  }
#endif

private:
  Polynomial error_function;
  // fisheye camera intrinsic parameters
  double xc, yc;
  double a0, a1, a2, a3, a4;
  m3d Ainv; // inverse affine transformation: transform camera to undistorted coords

  // extrinsics
  m3d R;
  v3d t;
};


#endif // #ifndef FISHEYE_CAMERA_HPP_INCLUDED_
