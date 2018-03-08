#include <iostream>
//using namespace std;
#define F 1.0
#include <cassert>
#include <cmath>
#include "../common/Image.h"


/*

 ../../build/cubemap_to_fisheye surface_camera_VIEW_person0.ppm surface_camera_VIEW_person0_alternate1.ppm surface_camera_VIEW_person0_alternate2.ppm surface_camera_VIEW_person0_alternate3.ppm surface_camera_VIEW_person0_alternate4.ppm surface_camera_VIEW_person0_alternate5.ppm cube_person0.ppm fisheye_person0.ppm  


../../build/cubemap_to_fisheye surface_camera_VIEW_person1.ppm surface_camera_VIEW_person1_alternate1.ppm surface_camera_VIEW_person1_alternate2.ppm surface_camera_VIEW_person1_alternate3.ppm surface_camera_VIEW_person1_alternate4.ppm surface_camera_VIEW_person1_alternate5.ppm cube_person1.ppm fisheye_person1.ppm

 */

void convertFisheyeToCube(double x,double y,double  xCenter,double yCenter, double &xCube, double &yCube,double &zCube )
{
  double xadj,yadj,r,xnorm,ynorm,betaZ,/*k,*/c;
      xCube=1;
    yCube=1;
    zCube=1;

    //Adjust x and y so that 0,0 is the center
    xadj=x-xCenter;
    yadj=y-yCenter;

    //Finds the distance, r, from the center
    r=sqrt(xadj*xadj+yadj*yadj);
    std::cout<<"r: "<<r<<std::endl;
    //Finds normalized versions of x and y
    xnorm=xadj/r;
    ynorm=yadj/r;
    //cout<<"norm x,y: "<<xnorm<<" "<<ynorm<<endl;

    //Finds betaZ, the z coord on a sphere
    //betaZ=cos(r);
    betaZ=cos(2*asin(r/2));
    //cout<<"betaz: "<<betaZ<<endl;
    //Finds the scale factor for the x and y co-ordinates to get back to
    //Sphere co-ordinates
    c=sqrt((1.0-(betaZ*betaZ))/(xnorm*xnorm+ynorm*ynorm));

    //Calculates the sphere co-ordinates
    xCube=c*xnorm;
    yCube=c*ynorm;
    zCube=betaZ;
    ////cout<<"sphere coord: "<<xCube<<" "<<yCube<<" "<<zCube<<endl;
    //Still needs to be converted back to cube co-ordinates

    //For now assume x=1

    yCube=yCube/-zCube;
    xCube=xCube/-zCube;
    zCube=zCube/-zCube;
    //cout<<"cube co-ord: "<<xCube<<" "<<yCube<<" "<<zCube<<endl;
        //xCube=floor(xCube*100)/100.0;
    //yCube=floor(yCube*100)/100.0;
}


void convertCubeToFisheye(double x,double y,double z, double &xFish, double &yFish )
{
          double px,py,pz,sum,theta,r;
              px=x; py=y; pz=z;
              sum=px*px+py*py+pz*pz;
              sum=sqrt(sum);
              px/=sum; py/=sum; pz/=sum;
              theta=acos(pz);

            r=2*F*sin(theta/2);

        sum=sqrt(px*px+py*py);
        px=px/sum;
        py=py/sum;

        xFish=r*px;
        yFish=r*py;
}

/*
int main_old()
{
    double xc,yc,zc;
    double xF,yF;
    double a,b;
    //cout<<acos(.408)<<" zzz"<<endl;
    //convertCubeToFisheye(1,.5,.5,a,b);
    //cout<<a<<" "<<b<<endl;
    while(1)
    {
      std::cin>>xc>>yc>>zc;
        convertCubeToFisheye(xc,yc,zc,xF,yF);
	std::cout<<xF<<" "<<yF<<std::endl;
        convertFisheyeToCube(xF,yF,0,0,xc,yc,zc);
        //if(xc>=-1&&xc<=1&&yc>=-1&&yc<=1)
	std::cout<<xc<<" "<<yc<<" "<<zc<<std::endl;

        //cout<<xF<<" "<<yF<<endl;
    }
     return 0;

}
*/


int main(int argc, char* argv[])
{


  assert (argc == 9);
  
  Image<sRGB> cube[6];
  int h=-1;
  int w=-1;
  for (int i = 0; i < 6; i++) {
    cube[i].load(argv[i+1]);
    if (i == 0) {
      h = cube[i].getRows();
      w = cube[i].getCols();
      assert (h == w);
    } else {
      assert (h == cube[i].getRows());
      assert (w == cube[i].getCols());
    }
  }
  assert (w > 0 && h > 0 && w == h);

  Image<sRGB> &front = cube[0];
  Image<sRGB> &back = cube[2];

  Image<sRGB> &left = cube[3];
  Image<sRGB> &right = cube[1];

  Image<sRGB> &top = cube[4];
  Image<sRGB> &bottom = cube[5];


  // visualize the cube
  Image<sRGB> cube_out(h*3,w*4);
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      cube_out(h+i,w+j) = front(i,j);
    }
  }
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      cube_out(h+i,3*w+j) = back(i,j);
    }
  }
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      cube_out(h+i,j) = left(i,j);
    }
  }
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      cube_out(h+i,2*w+j) = right(i,j);
    }
  }
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      //????
      cube_out(i,w+j) = top(h-i,w-j);
    }
  }
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      //????
      cube_out(2*h+i,w+j) = bottom(i,j);
    }
  }
  cube_out.write(argv[7]);


  int out_h = 2*h;
  int out_w = 2*w;


  Image<sRGB> fisheye_out(out_h,out_w);
  for (int i = 0; i < out_h; i++) {
    for (int j = 0; j < out_w; j++) {
      //fisheye_out(i,j) = sRGB(255,255,0);
      // set all pixels to black
      fisheye_out(i,j) = sRGB(0,0,0);
    }
  }

  // loop over the front image pixels, put in fisheye view
  double step = 1;
  for (double i = 0; i < h; i+= step) {
    for (double j = 0; j < w; j+= step) {
      double x = 2*(i/(double)h - 0.5);
      double y = 2*(j/(double)w - 0.5);
      double z = 1;
      double fish_x;
      double fish_y;
      convertCubeToFisheye(x,y,z,fish_x,fish_y);
//      int fx = ((fish_x)/2.0 + 1) * h;
      int fx = ((fish_x)/2.0 + 1) * out_h/2.0;
//      int fy = ((fish_y)/2.0 + 1) * w;
      int fy = ((fish_y)/2.0 + 1) * out_w/2.0;
      if (fx < 0 || fx >= out_h ||
	  fy < 0 || fy >= out_w) {
	std::cout << i <<","<< j << "  =>  " << fx <<","<<fy <<std::endl;
	continue;
      }
      fisheye_out(fx,fy) = front(int(i),int(j));
    }
  }

  // loop over the back image pixels, put in fisheye view
  step = 0.1;
  for (double i = 0; i < h; i+=step) {
    for (double j = 0; j < w; j+=step) {
      double x = 2*(i/(double)h - 0.5);
      double y = 2*(0.5 - j/(double)w);
      double z = -1;
      double fish_x;
      double fish_y;
      convertCubeToFisheye(x,y,z,fish_x,fish_y);
      int fx = ((fish_x)/2.0 + 1) * h;
      int fy = ((fish_y)/2.0 + 1) * w;
      if (fx < 0 || fx >= out_h ||
	  fy < 0 || fy >= out_w) {
	std::cout << i <<","<< j << "  =>  " << fx <<","<<fy <<std::endl;
	continue;
      }
      fisheye_out(fx,fy) = back(int(i),int(j));
    }
  }

  // loop over the right image pixels, put in fisheye view
  step = 0.5;
  for (double i = 0; i < h; i+= step) {
    for (double j = 0; j < w; j+= step) {
      double x = 2*(i/(double)h - 0.5);
      double y = 1;//
      double z = 2*(0.5-j/(double)w);
      double fish_x;
      double fish_y;
      convertCubeToFisheye(x,y,z,fish_x,fish_y);
      int fx = ((fish_x)/2.0 + 1) * h;
      int fy = ((fish_y)/2.0 + 1) * w;
      if (fx < 0 || fx >= out_h ||
	  fy < 0 || fy >= out_w) {
	std::cout << i <<","<< j << "  =>  " << fx <<","<<fy <<std::endl;
	continue;
      }
      fisheye_out(fx,fy) = right(int(i),int(j));
    }
  }


  // loop over the bottom image pixels, put in fisheye view
  for (double i = 0; i < h; i+= step) {
    for (double j = 0; j < w; j+= step) {
      double x = 1;//
      double y = 2*(j/(double)w - 0.5);
      double z = 2*(0.5-i/(double)h );
      double fish_x;
      double fish_y;
      convertCubeToFisheye(x,y,z,fish_x,fish_y);
      int fx = ((fish_x)/2.0 + 1) * h;
      int fy = ((fish_y)/2.0 + 1) * w;
      if (fx < 0 || fx >= out_h ||
	  fy < 0 || fy >= out_w) {
	std::cout << i <<","<< j << "  =>  " << fx <<","<<fy <<std::endl;
	continue;
      }
      fisheye_out(fx,fy) = bottom(int(i),int(j));
    }
  }



  // loop over the left image pixels, put in fisheye view
  for (double i = 0; i < h; i+= step) {
    for (double j = 0; j < w; j+= step) {
      double x = 2*(i/(double)h - 0.5);
      double y = -1;//
      double z = 2*(j/(double)w - 0.5);
      double fish_x;
      double fish_y;
      convertCubeToFisheye(x,y,z,fish_x,fish_y);
      int fx = ((fish_x)/2.0 + 1) * h;
      int fy = ((fish_y)/2.0 + 1) * w;
      if (fx < 0 || fx >= out_h ||
	  fy < 0 || fy >= out_w) {
	std::cout << i <<","<< j << "  =>  " << fx <<","<<fy <<std::endl;
	continue;
      }
      fisheye_out(fx,fy) = left(int(i),int(j));
    }
  }


  // loop over the top image pixels, put in fisheye view
  for (double i = 0; i < h; i+= step) {
    for (double j = 0; j < w; j+= step) {
      double x = -1;//
      double y = 2*(0.5-j/(double)w);
      double z = 2*(0.5-i/(double)h);
      double fish_x;
      double fish_y;
      convertCubeToFisheye(x,y,z,fish_x,fish_y);
      int fx = ((fish_x)/2.0 + 1) * h;
      int fy = ((fish_y)/2.0 + 1) * w;
      if (fx < 0 || fx >= out_h ||
	  fy < 0 || fy >= out_w) {
	std::cout << i <<","<< j << "  =>  " << fx <<","<<fy <<std::endl;
	continue;
      }
      fisheye_out(fx,fy) = top(int(i),int(j));
    }
  }




  fisheye_out.write(argv[8]);
  //  main_old();


  /*

    double xc,yc,zc;
    double xF,yF;
    double a,b;
    //cout<<acos(.408)<<" zzz"<<endl;
    //convertCubeToFisheye(1,.5,.5,a,b);
    //cout<<a<<" "<<b<<endl;
    while(1)
    {
      std::cin>>xc>>yc>>zc;
        convertCubeToFisheye(xc,yc,zc,xF,yF);
	std::cout<<xF<<" "<<yF<<std::endl;
        convertFisheyeToCube(xF,yF,0,0,xc,yc,zc);
        //if(xc>=-1&&xc<=1&&yc>=-1&&yc<=1)
	std::cout<<xc<<" "<<yc<<" "<<zc<<std::endl;

        //cout<<xF<<" "<<yF<<endl;
    }
  */

     return 0;

}
