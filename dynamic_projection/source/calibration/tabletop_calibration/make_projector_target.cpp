#include <Image.h>
#include <ImageOps.h>
#include <math.h>
#include <stdio.h>

void corner(Image<sRGB> &target, int r, int c, int radius, int flip){
  int line_width = 3;
  for (int row=r-radius; row<=r+radius; row++){
    for (int col=c-radius; col<=c+radius; col++){
      if (abs(row-r+radius) < line_width ||
	  abs(row-r-radius) < line_width ||
	  abs(col-c+radius) < line_width ||
	  abs(col-c-radius) < line_width){
	target(row,col) = sRGB(0,0,0);
      } else {
	if ((row < r) ^ (col < c) ^ flip){
	  target(row,col) = sRGB(0,0,0);
	}
      }
    }
  }
}


void disk(Image<sRGB> &target, int r, int c, int radius, sRGB color){
  int r2 = radius * radius;
  for (int row=r-radius; row<=r+radius; row++){
    for (int col=c-radius; col<=c+radius; col++){
      double dist = (row-r)*(row-r) + (col-c)*(col-c);
      if (dist < double(r2)){
	target(row,col) = color;
      }
    }
  }
}



int main(int argc, char **argv){
  int rows = 768;
  int cols = 1024;
  Image<sRGB> target(rows, cols);
  //  ImageClearer<sRGB> clearer(sRGB(255, 255, 255));
  ImageClearer<sRGB> clearer(sRGB(63, 63, 63));
  ImagePointOperation(target, clearer);
  
  int Nx = 16;
  int Ny = 12;
  int radius = 25;
  int disk_radius = 30;
  int spacing = 63;
  int margin = 40;

  FILE *fp = fopen(argv[2], "wt");
  if (NULL == fp){
    fprintf(stderr, "unable to open %s\n", argv[2]);
    exit(-1);
  }

  for (int i=0; i<Ny; i++){
    int row = margin + spacing * i;
    for (int j=0; j<Nx; j++){
      int col = margin + spacing * j;
      if (i == 0 && j == 0){
	disk(target, row, col, disk_radius, sRGB(127,0,0));
      } else if (i == 0 && j == Nx-1){
	disk(target, row, col, disk_radius, sRGB(0,0,127));
      } else if (i == Ny-1 && j == 0){
	disk(target, row, col, disk_radius, sRGB(0,127,0));
      } else if (i == Ny-1 && j == Nx-1){
	disk(target, row, col, disk_radius, sRGB(127,0,127));
      } else {
	fprintf(fp, "%f %f\n", col-0.5, row-0.5);
	corner(target, row, col, radius, (i%2)^(j%2));
      }
    }
  }

  fclose(fp);

  target.write(argv[1]);
  return 0;
}
