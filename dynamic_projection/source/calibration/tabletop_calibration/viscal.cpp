// viscal.cpp: visualize calibration data
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <Vector3.h>
#include "zpr.h"
#include <CalibratedCamera.h>

int n_points = 188;
int n_planes = 3;

CalibratedCamera camera;
CalibratedCamera proj_a;
CalibratedCamera proj_b;
CalibratedCamera proj_c;
CalibratedCamera proj_d;

struct pixel 
{
  pixel(double r, double c){
    row = r;
    col = c;
  }
  double row, col;
};

std::vector<v3d> cam_points;
std::vector<v3d> proj_a_points;
std::vector<v3d> proj_b_points;
std::vector<v3d> proj_c_points;
std::vector<v3d> proj_d_points;
std::vector<pixel> model_points;

const double pi = 3.14159265358979323846;

bool spheres_visible;
bool a_visible;
bool b_visible;
bool c_visible;
bool d_visible;
bool bp_visible;
bool cam_visible;
bool table_visible;

void drawtable(){
  if (!table_visible) return;
  int num_tri = 100;
  double r = 0.5334;


  glColor4f(0.75, 0.75, 0.75, 1.0);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3d(0., 0., 0.0381);
  for (int i=0; i<num_tri; i++){
    double th = 2*pi - (2. * pi * i) / (num_tri-1);
    double x = r * cos(th);
    double z = r * sin(th);
    double y = -0.0381;
    glVertex3d(x,y,z);
  }
  glEnd();

  glColor4f(0.5, 0.5, 0.5, 1.0);
  glBegin(GL_TRIANGLE_STRIP);
  for (int i=0; i<num_tri; i++){
    double th1 = 2*pi - (2. * pi * i) / (num_tri-1);
    double th2 = 2*pi - (2. * pi * (i + 0.5)) / (num_tri-1);
    double x1 = r * cos(th1);
    double z1 = r * sin(th1);
    double x2 = r * cos(th2);
    double z2 = r * sin(th2);
    double y1 = -0.;
    double y2 = -0.0381;
    glVertex3d(x1,y1,z1);
    glVertex3d(x2,y2,z2);
  }
  glEnd();


  glColor4f(0.75, 0.75, 0.75, 1.0);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3d(0., 0., 0.);
  for (int i=0; i<num_tri; i++){
    double th = 2*pi - (2. * pi * i) / (num_tri-1);
    double x = r * cos(th);
    double z = r * sin(th);
    double y = -0.;
    glVertex3d(x,y,z);
  }
  glEnd();



}

void drawcamera(){

  glColor4d(1., 1., 1., 0.01);
  v3d center = camera.getCenter();
  glBegin(GL_LINES);
  for (int i=0; i<n_points*n_planes; i++){
    v3d p1 = proj_a_points.at(i);
    glVertex3d(p1.x(), p1.y(), p1.z());
    glVertex3d(center.x(), center.y(), center.z());
  }
  for (int i=0; i<n_points*n_planes; i++){
    v3d p1 = proj_b_points.at(i);
    glVertex3d(p1.x(), p1.y(), p1.z());
    glVertex3d(center.x(), center.y(), center.z());
  }
  for (int i=0; i<n_points*n_planes; i++){
    v3d p1 = proj_c_points.at(i);
    glVertex3d(p1.x(), p1.y(), p1.z());
    glVertex3d(center.x(), center.y(), center.z());
  }

  for (int i=0; i<n_points*n_planes; i++){
    v3d p1 = proj_d_points.at(i);
    glVertex3d(p1.x(), p1.y(), p1.z());
    glVertex3d(center.x(), center.y(), center.z());
  }

  glEnd();
  
   return;



  if (cam_visible){
    glColor4d(1., 1., 0., 1.);

    glBegin(GL_POINTS);
    for (unsigned int i = 0; i<cam_points.size(); i++){
      glVertex3d(cam_points.at(i).x(),
		 cam_points.at(i).y(),
                 cam_points.at(i).z());
    }
    glEnd();
    
    glColor4d(1., 1., 1., 0.2);
    glBegin(GL_LINES);
    v3d center = camera.getCenter();
    for (unsigned int i = 0; i<cam_points.size(); i++){
      glVertex3d(cam_points.at(i).x(),
		 cam_points.at(i).y(),
		 cam_points.at(i).z());
      glVertex3d(center.x(), center.y(), center.z());
    }
    glEnd();
  }
}

void drawpoints(){

  glPointSize(2.1);
  glEnable(GL_POINT_SMOOTH);

  if (spheres_visible){
    if (a_visible){
      glColor4d(1., 0., 0., 0.2);
      for (unsigned int i = 0; i<proj_a_points.size(); i++){
	v3d p1 = proj_a.PointFromPixel(model_points.at(i%n_points).row,
                                       model_points.at(i%n_points).col,
				       proj_a_points.at(i).y());
	v3d delta = p1 - proj_a_points.at(i);
	double radius = delta.length();
	glPushMatrix();
	glTranslated(proj_a_points.at(i).x(),
		     proj_a_points.at(i).y(),
		     proj_a_points.at(i).z());
	glutSolidSphere(radius, 10, 20);
	glPopMatrix();
      }
    }

    if (b_visible){
      glColor4d(0., 1., 0., 0.2);
      for (unsigned int i = 0; i<proj_b_points.size(); i++){
	v3d p1 = proj_b.PointFromPixel(model_points.at(i%n_points).row,
                                       model_points.at(i%n_points).col,
				       proj_b_points.at(i).y());
	v3d delta = p1 - proj_b_points.at(i);
	double radius = delta.length();
	glPushMatrix();
	glTranslated(proj_b_points.at(i).x(),
		     proj_b_points.at(i).y(),
		     proj_b_points.at(i).z());
	glutSolidSphere(radius, 10, 20);
	glPopMatrix();
      }
    }

    if (c_visible){
      glColor4d(0., 1., 1., 0.2);
      for (unsigned int i = 0; i<proj_c_points.size(); i++){
	v3d p1 = proj_c.PointFromPixel(model_points.at(i%n_points).row,
                                       model_points.at(i%n_points).col,
				       proj_c_points.at(i).y());
	v3d delta = p1 - proj_c_points.at(i);
	double radius = delta.length();
	glPushMatrix();
	glTranslated(proj_c_points.at(i).x(),
		     proj_c_points.at(i).y(),
		     proj_c_points.at(i).z());
	glutSolidSphere(radius, 10, 20);
	glPopMatrix();
      }
    }

    if (d_visible){
      glColor4d(1., 0., 1., 0.2);
      for (unsigned int i = 0; i<proj_d_points.size(); i++){
	v3d p1 = proj_d.PointFromPixel(model_points.at(i%n_points).row,
                                       model_points.at(i%n_points).col,
				       proj_d_points.at(i).y());
	v3d delta = p1 - proj_d_points.at(i);
	double radius = delta.length();
	glPushMatrix();
	glTranslated(proj_d_points.at(i).x(),
		     proj_d_points.at(i).y(),
		     proj_d_points.at(i).z());
	glutSolidSphere(radius, 10, 20);
	glPopMatrix();
      }
    }
  }

  if (a_visible){
    glColor4d(1., 0., 0., 1.);
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i<proj_a_points.size(); i++){
      glVertex3d(proj_a_points.at(i).x(),
		 proj_a_points.at(i).y(),
		 proj_a_points.at(i).z());
    }
    glEnd();
  }

  if (b_visible){
    glColor4d(0., 1., 0., 1.);
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i<proj_b_points.size(); i++){
      glVertex3d(proj_b_points.at(i).x(),
		 proj_b_points.at(i).y(),
		 proj_b_points.at(i).z());
    }
    glEnd();
  }

  if (c_visible){
    glColor4d(0., 1., 1., 1.);
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i<proj_c_points.size(); i++){
      glVertex3d(proj_c_points.at(i).x(),
		 proj_c_points.at(i).y(),
		 proj_c_points.at(i).z());
    }
    glEnd();
  }

  if (d_visible){
    glColor4d(1., 0., 1., 1.);
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i<proj_d_points.size(); i++){
      glVertex3d(proj_d_points.at(i).x(),
		 proj_d_points.at(i).y(),
		 proj_d_points.at(i).z());
    }
    glEnd();
  }
}

void drawlines(){
  int pn = 4;
  double d = 15;
  if (a_visible){
    glColor4d(1., 0., 0., 0.25);
    glBegin(GL_LINES);
    for (int i=0; i<n_points; i++){
      v3d p1 = proj_a_points.at(i);
      v3d p2 = proj_a_points.at(i + pn * n_points);
      v3d p3 = p1 + d*(p2-p1);
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }

  if (b_visible){
    glColor4d(0., 1., 0., 0.25);
    glBegin(GL_LINES);
    for (int i=0; i<n_points; i++){
      v3d p1 = proj_b_points.at(i);
      v3d p2 = proj_b_points.at(i + pn * n_points);
      v3d p3 = p1 + d*(p2-p1);
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }


  if (c_visible){
    glColor4d(0., 1., 1., 0.25);
    glBegin(GL_LINES);
    for (int i=0; i<n_points; i++){
      v3d p1 = proj_c_points.at(i);
      v3d p2 = proj_c_points.at(i + pn * n_points);
      v3d p3 = p1 + d*(p2-p1);
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }

  if (d_visible){
    glColor4d(1., 0., 1., 0.25);
    glBegin(GL_LINES);
    for (int i=0; i<n_points; i++){
      v3d p1 = proj_d_points.at(i);
      v3d p2 = proj_d_points.at(i + pn * n_points);
      v3d p3 = p1 + d*(p2-p1);
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }
}

void drawbplines(){
  if (!bp_visible) return;

  if (a_visible){
    glColor4d(1., 0., 0., 0.25);
    glBegin(GL_LINES);
    for (unsigned int i=0; i<model_points.size(); i++){
      v3d p1 = proj_a.PointFromPixel(model_points.at(i).row,
                                     model_points.at(i).col,
				     0.);
      v3d p3 = proj_a.getCenter();
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }

  if (b_visible){
    glColor4d(0., 1., 0., 0.25);
    glBegin(GL_LINES);
    for (unsigned int i=0; i<model_points.size(); i++){
      v3d p1 = proj_b.PointFromPixel(model_points.at(i).row,
                                     model_points.at(i).col,
				     0.);
      v3d p3 = proj_b.getCenter();
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }

  if (c_visible){
    glColor4d(0., 1., 1., 0.25);
    glBegin(GL_LINES);
    for (unsigned int i=0; i<model_points.size(); i++){
      v3d p1 = proj_c.PointFromPixel(model_points.at(i).row,
                                     model_points.at(i).col,
				     0.);
      v3d p3 = proj_c.getCenter();
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }

  if (d_visible){
    glColor4d(1., 0., 1., 0.25);
    glBegin(GL_LINES);
    for (unsigned int i=0; i<model_points.size(); i++){
      v3d p1 = proj_d.PointFromPixel(model_points.at(i).row,
				     model_points.at(i).col,
				     0.);
      v3d p3 = proj_d.getCenter();
      glVertex3d(p1.x(), p1.y(), p1.z());
      glVertex3d(p3.x(), p3.y(), p3.z());
    }
    glEnd();
  }
}

void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   drawtable();
   drawcamera();
   drawpoints();
   //   drawlines();
   drawbplines();

   glutSwapBuffers();
}


void KeyEvent(unsigned char key, int x, int y){
  if (key == 'q'){
    exit(0);
  } else if (key == 'a'){
    a_visible = ! a_visible;
  } else if (key == 'b'){
    b_visible = ! b_visible;
  } else if (key == 'c'){
    c_visible = ! c_visible;
  } else if (key == 'd'){
    d_visible = ! d_visible;
  } else if (key == 'p'){
    bp_visible = ! bp_visible;
  } else if (key == 'l'){
    cam_visible = ! cam_visible;
  } else if (key == 't'){
    table_visible = ! table_visible;
  } else if (key == 's'){
    spheres_visible = ! spheres_visible;
  }
  glutPostRedisplay();
}

bool by_y(const v3d &a, const v3d &b){
  return a.y() > b.y();
}

int main(int argc, char **argv){

  spheres_visible = false;
  a_visible = false;
  b_visible = false;
  c_visible = false;
  d_visible = false;
  bp_visible = false;
  cam_visible = true;
  table_visible = true;

  camera.loadCalibration("camera_images/camera_calibration.dat");

  // read the camera calibration points
  FILE *fp = fopen("camera_images/model.txt", "rt");
  if (NULL == fp){
    fprintf(stderr, "cannot open camera_images/model.txt\n");
    exit(-1);
  }
  double x, y, z = 0.;
  while (2 == fscanf(fp, "%lf %lf", &x, &y)){
    cam_points.push_back(v3d(x,-z,y));
  }
  fclose(fp);  

  fp = fopen("projector_images/projector_A_tsai.dat", "rt");
  if (NULL == fp){
    fprintf(stderr, "cannot open projector_images/projector_A_tsai.dat\n");
    exit(-1);
  }
  double r, c;
  while (5 == fscanf(fp, "%lf %lf %lf %lf %lf", &x, &y, &z, &c, &r)){
    proj_a_points.push_back(v3d(x,y,z));
  }
  fclose(fp);  
  std::stable_sort(proj_a_points.begin(), proj_a_points.end(), by_y);

  fp = fopen("projector_images/projector_B_tsai.dat", "rt");
  if (NULL == fp){
    fprintf(stderr, "cannot open projector_images/projector_B_tsai.dat\n");
    exit(-1);
  }
  while (5 == fscanf(fp, "%lf %lf %lf %lf %lf", &x, &y, &z, &c, &r)){
    proj_b_points.push_back(v3d(x,y,z));
  }
  fclose(fp);  
  std::stable_sort(proj_b_points.begin(), proj_b_points.end(), by_y);

  fp = fopen("projector_images/projector_C_tsai.dat", "rt");
  if (NULL == fp){
    fprintf(stderr, "cannot open projector_images/projector_C_tsai.dat\n");
    exit(-1);
  }
  while (5 == fscanf(fp, "%lf %lf %lf %lf %lf", &x, &y, &z, &c, &r)){
    proj_c_points.push_back(v3d(x,y,z));
  }
  fclose(fp);  
  std::stable_sort(proj_c_points.begin(), proj_c_points.end(), by_y);

  fp = fopen("projector_images/projector_D_tsai.dat", "rt");
  if (NULL == fp){
    fprintf(stderr, "cannot open projector_images/projector_D_tsai.dat\n");
    exit(-1);
  }
  while (5 == fscanf(fp, "%lf %lf %lf %lf %lf", &x, &y, &z, &c, &r)){
    proj_d_points.push_back(v3d(x,y,z));
  }
  fclose(fp);  
  std::stable_sort(proj_d_points.begin(), proj_d_points.end(), by_y);

  proj_a.loadCalibration("projector_images/projector_A_calibration.dat");
  proj_b.loadCalibration("projector_images/projector_B_calibration.dat");
  proj_c.loadCalibration("projector_images/projector_C_calibration.dat");
  proj_d.loadCalibration("projector_images/projector_D_calibration.dat");

  // read the projector calibration points
  fp = fopen("projector_images/projector_model.dat", "rt");
  if (NULL == fp){
    fprintf(stderr, "cannot open projector_images/projector_model.dat\n");
    exit(-1);
  }
  while (2 == fscanf(fp, "%lf %lf", &c, &r)){
    model_points.push_back(pixel(r,c));
  }
  fclose(fp);  

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(1024, 768);
  glutCreateWindow("Calibration Data");
  
  glutKeyboardFunc(KeyEvent);
  glutDisplayFunc(display);
  glScaled(2.0, 2.0, 2.0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  zprInit();    
  glutMainLoop();

  return 0;
}
