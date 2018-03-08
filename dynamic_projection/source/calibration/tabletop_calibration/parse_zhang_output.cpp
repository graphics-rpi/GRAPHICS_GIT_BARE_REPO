#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char **argv){
  if (argc != 5){
    fprintf(stderr, "usage:\n" 
            "%s <zhang_input> <width> <height> <calibration_output>\n",
            argv[0]);
    return -1;
  }
  int width  = atoi(argv[2]);
  int height = atoi(argv[3]);

  FILE *fp = fopen(argv[1], "rt");
  if (NULL == fp){
    fprintf(stderr, "unable to open %s\n", argv[1]);
    return -1;
  }

  // read intrinsic matrix
  double K[9];
  fscanf(fp, "%lf", &K[0]);
  fscanf(fp, "%lf", &K[1]);
  fscanf(fp, "%lf", &K[4]);
  fscanf(fp, "%lf", &K[2]);
  fscanf(fp, "%lf", &K[5]);
  K[3] = 0.;
  K[6] = 0.;
  K[7] = 0.;
  K[8] = 1.;

  // read radial distortion parameters; not used for now
  double k1, k2;
  fscanf(fp, "%lf %lf", &k1, &k2);

  // nb: last extrinsics matrix defines the world coordinate system
  // read all extrinsics, retaining only last one
  double R[9];
  double t[3];
  while (!feof(fp)){
    for (int i=0; i<3; i++){
      for (int j=0; j<3; j++){
        fscanf(fp, "%lf", &R[i*3+j]);
      }
    }
    for (int i=0; i<3; i++){
      fscanf(fp, "%lf", &t[i]);
    }
  }
  fclose(fp);

  // remove offset for calibration target thickness
  //  this makes the table top the coord. sys. origin
  // note:
  // x = PX
  // x = K[R|t]X
  // x = KRX + Kt
  // x'= KR(X+dX) + Kt
  //   = KRX + KRdX + Kt
  //   = KRX + K(RdX+t)
  //   = K(R|RdX+t)
  // ==>  t' = RdX+t
  const double target_thickness = 0.0127;  // 0.5" in meters
  double dX[3] = {0., 0., target_thickness};
  double RdX[3];
  for (int i=0; i<3; i++){
    RdX[i] = 0.;
    for (int j=0; j<3; j++){
      RdX[i] += R[i*3+j] * dX[j];
    }
  }
  for (int i=0; i<3; i++){
    t[i] += RdX[i];
  }

  // transform camera coordinate system to construct world coordinate system
  // as output by Zhang's code, -z is normal to the table
  // our system requires +y to be normal to the table
  double T[9] = {1., 0., 0.,
                 0., 0., 1.,
                 0., -1., 0.};

  // transform R
  double RT[9];
  for (int i=0; i<9; i++) RT[i] = 0;
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++){
      for (int k=0; k<3; k++){
        RT[i*3+k] += R[i*3+j] * T[j*3+k];
      }
    }
  }

  // write the output file
  fp = fopen(argv[4], "wt");
  if (NULL == fp){
    fprintf(stderr, "unable to open %s\n", argv[4]);
    return -1;
  }
  
  fprintf(fp, "# width height\n");
  fprintf(fp, "%d %d\n", width, height);
  
  fprintf(fp, "# intrinsics matrix (K)\n");
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++){
      fprintf(fp, "%15.10lf ", K[3*i+j]);
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "# extrinsics matrix ([R|t])\n");
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++){
      fprintf(fp, "%15.10lf ", RT[3*i+j]);
    }
    fprintf(fp, "%15.10lf\n", t[i]);
  }
  
  fprintf(fp, "# sx\n");
  double sx = 1.;
  fprintf(fp, "%15.10lf\n", sx);

  fprintf(fp, "# k1 [k2]\n");
  fprintf(fp, "%15.10lf %15.10lf\n", k1, k2);

  // write world offset
  fprintf(fp, "# world coordinate frame offset\n");
  for (int i=0; i<3; i++){
    double v = 0.;
    fprintf(fp, "%15.10lf ", v);
  }
  fprintf(fp, "\n");

  fclose(fp);

  return 0;
}
