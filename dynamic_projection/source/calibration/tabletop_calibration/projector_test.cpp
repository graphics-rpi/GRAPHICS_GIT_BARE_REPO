#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <Vector3.h>
#include <util.h>
#include <vector>
#include <math.h>
#include <Image.h>
#include "OpenGLProjector.h"


const double pi = 3.14159265358979323846;


std::vector<v3d> model_points;
Image<sRGB> image;
int height, width;

struct window {
  double x[4];
  double z[4];
  v3d color;
  void render(double y){
    glColor3d(color.r(), color.g(), color.b());
    glBegin(GL_QUADS);
    for (int i=0; i<4; i++){
      glVertex3d(x[i], y+0.001, z[i]);
    }
    glEnd();
  }
};

struct wall {
  std::vector<window> windows;
  double x[4];
  double z[4];
  double y;
  v3d color;
  v3d top_color;
  void render(){
    glColor3d(top_color.r(), top_color.g(), top_color.b());
    glBegin(GL_QUADS);
    for (int i=0; i<4; i++){
      glVertex3d(x[i], y, z[i]);
    }
    glColor3d(color.r(), color.g(), color.b());
    glVertex3d(x[0], y, z[0]);
    glVertex3d(x[1], y, z[1]);
    glVertex3d(x[1], 0, z[1]);
    glVertex3d(x[0], 0, z[0]);

    glVertex3d(x[1], y, z[1]);
    glVertex3d(x[2], y, z[2]);
    glVertex3d(x[2], 0, z[2]);
    glVertex3d(x[1], 0, z[1]);

    glVertex3d(x[2], y, z[2]);
    glVertex3d(x[3], y, z[3]);
    glVertex3d(x[3], 0, z[3]);
    glVertex3d(x[2], 0, z[2]);

    glVertex3d(x[3], y, z[3]);
    glVertex3d(x[0], y, z[0]);
    glVertex3d(x[0], 0, z[0]);
    glVertex3d(x[3], 0, z[3]);
    glEnd();
  }
};

struct curvedwall {
  double xc;
  double zc;
  double inner_r;
  double outer_r;
  double min_angle;
  double max_angle;
  double y;
  v3d color;
  v3d top_color;

  void render(){
    glColor3d(color.r(), color.g(), color.b());
    glBegin(GL_QUAD_STRIP);
    int nsteps = int(45*(max_angle - min_angle)/pi);
    for (int i=0; i<nsteps; i++){
      double theta = min_angle + (max_angle - min_angle)*double(i)/(nsteps-1);
      double x = xc + inner_r * cos(theta);
      double z = zc + inner_r * sin(theta);
      glVertex3d(x, y, z);
      glVertex3d(x, 0, z);
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    for (int i=0; i<nsteps; i++){
      double theta = min_angle + (max_angle - min_angle)*double(i)/(nsteps-1);
      double x = xc + outer_r * cos(theta);
      double z = zc + outer_r * sin(theta);
      glVertex3d(x, y, z);
      glVertex3d(x, 0, z);
    }
    glEnd();
    glColor3d(top_color.r(), top_color.g(), top_color.b());
    glBegin(GL_QUAD_STRIP);
    for (int i=0; i<nsteps; i++){
      double theta = min_angle + (max_angle - min_angle)*double(i)/(nsteps-1);
      double x1 = xc + outer_r * cos(theta);
      double z1 = zc + outer_r * sin(theta);
      double x2 = xc + inner_r * cos(theta);
      double z2 = zc + inner_r * sin(theta);
      glVertex3d(x1, y, z1);
      glVertex3d(x2, y, z2);
    }
    glEnd();

    glBegin(GL_QUADS);
    double x1 = xc + outer_r * cos(min_angle);
    double z1 = zc + outer_r * sin(min_angle);
    double x2 = xc + inner_r * cos(min_angle);
    double z2 = zc + inner_r * sin(min_angle);
    glVertex3d(x1, 0, z1);
    glVertex3d(x1, y, z1);
    glVertex3d(x2, y, z2);
    glVertex3d(x2, 0, z2);
    glEnd();

    glBegin(GL_QUADS);
    x1 = xc + outer_r * cos(max_angle);
    z1 = zc + outer_r * sin(max_angle);
    x2 = xc + inner_r * cos(max_angle);
    z2 = zc + inner_r * sin(max_angle);
    glVertex3d(x1, 0, z1);
    glVertex3d(x1, y, z1);
    glVertex3d(x2, y, z2);
    glVertex3d(x2, 0, z2);
    glEnd();
  }
};

struct Scene {
  std::vector<wall> walls;
  std::vector<curvedwall> curvedwalls;
  v3d floor_color;
  v3d ceiling_color;
  std::vector<v3d> wall_colors;

  void render(){
    drawtable();
    for (unsigned int i=0; i<walls.size(); i++){
      walls[i].render();
      for (unsigned int j=0; j<walls[i].windows.size(); j++){
	walls[i].windows[j].render(walls[i].y);
      }
    }
    for (unsigned int i=0; i<curvedwalls.size(); i++){
      curvedwalls[i].render();
    }
  }

  void load(const char *filename){
    char line[1024];
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "can't open %s\n", filename);
      exit(-1);
    }

    fgets(line, 1024, fp);
    fscanf(fp, "floor_material %lf %lf %lf\n",
	   &floor_color.r(),
	   &floor_color.g(),
	   &floor_color.b());
    fscanf(fp, "ceiling_material %lf %lf %lf\n",
	   &ceiling_color.r(),
	   &ceiling_color.g(),
	   &ceiling_color.b());

    int num_wall_materials;
    fscanf(fp, "num_wall_materials %d\n", &num_wall_materials);
    for (int i=0; i<num_wall_materials; i++){
      v3d color;
      fscanf(fp, "%lf %lf %lf\n",
             &color.r(),
             &color.g(),
             &color.b());
      wall_colors.push_back(color);
    }

    int num_walls;
    fscanf(fp, "num_walls %d\n", &num_walls);
    for (int i=0; i<num_walls; i++){
      wall w;
      fscanf(fp, "wall ");
      for (int j=0;j<4;j++){
	fscanf(fp, "%lf %lf", &w.x[j], &w.z[j]);
      }
      fscanf(fp, "%lf", &w.y);
      if (w.y < 0.15){
	w.top_color = v3d(0.,1.,0.);
      } else if (w.y < 0.225){
	w.top_color = v3d(0.,0.,1.);
      } else {
	w.top_color = v3d(1.,0.,0.);
      }
      int mat_idx;
      fscanf(fp, "%d\n", &mat_idx);
      w.color = wall_colors[mat_idx];

      int num_windows;
      fscanf(fp, "num_windows %d\n", &num_windows);
      for (int j=0;j<num_windows;j++){
	window win;
	fscanf(fp, "window ");
	for (int k=0;k<4;k++){
	  fscanf(fp, "%lf %lf", &win.x[k], &win.z[k]);
	}
	char color[1024];
	fscanf(fp," %1024s\n", color);
	if (!strcmp(color, "yellow")){
	  win.color = v3d(1., 1., 0.);
	} else if (!strcmp(color, "cyan")){
	  win.color = v3d(0., 1., 1.);
	} else if (!strcmp(color, "magenta")){
	  win.color = v3d(1., 0., 1.);
	}
	w.windows.push_back(win);
      }
      walls.push_back(w);
    }

    int num_curved_walls;
    fscanf(fp, "num_curved_walls %d\n", &num_curved_walls);
    for (int i=0; i<num_curved_walls; i++){
      curvedwall cw;
      fscanf(fp, "curved_wall ");
      int mat_idx;
      fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf %d\n",
             &cw.xc, &cw.zc, &cw.inner_r, &cw.outer_r, 
             &cw.min_angle, &cw.max_angle, &cw.y, &mat_idx);
      cw.color = wall_colors[mat_idx];
      if (cw.y < 0.15){
	cw.top_color = v3d(0.,1.,0.);
      } else if (cw.y < 0.225){
	cw.top_color = v3d(0.,0.,1.);
      } else {
	cw.top_color = v3d(1.,0.,0.);
      }

      curvedwalls.push_back(cw);
    }
    fclose(fp);
  }

  void drawtable(){
    int num_tri = 100;
    double r = 0.525;
    
    glColor4f(floor_color.r(), floor_color.g(), floor_color.b(), 1.0);
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
    
    glColor4f(floor_color.r(), floor_color.g(), floor_color.b(), 1.0);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i=0; i<num_tri; i++){
      double th1 = 2*pi - (2. * pi * i) / (num_tri-1);
      double th2 = 2*pi - (2. * pi * (i + 0.5)) / (num_tri-1);
      double x1 = r * cos(th1);
      double z1 = r * sin(th1);
      double x2 = r * cos(th2);
      double z2 = r * sin(th2);
      double y1 = 0.;
      double y2 = -0.0381;
      glVertex3d(x1,y1,z1);
      glVertex3d(x2,y2,z2);
    }
    glEnd();

    glColor4f(floor_color.r(), floor_color.g(), floor_color.b(), 1.0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0., 0., 0.);
    for (int i=0; i<num_tri; i++){
      double th = 2*pi - (2. * pi * i) / (num_tri-1);
      double x = r * cos(th);
      double z = r * sin(th);
      double y = 0.;
      glVertex3d(x,y,z);
    }
    glEnd();
  }


};

Scene scene;


void render(){
  scene.render();
}

char *filename;
void snapshot(){
  GLubyte *image;
  FILE *fp = fopen(filename, "wb");
  if (NULL == filename){
    fprintf(stderr, "unable to open %s\n", filename);
    exit(-1);
  }

  image = new GLubyte[width * 3];

  /* OpenGL's default 4 byte pack alignment would leave extra bytes at the
     end of each image row so that each full row contained a number of bytes
     divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit componets would
     be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
     just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
     the rows are packed as tight as possible (no row padding), set the pack
     alignment to 1. */
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  fprintf(fp, "P6\n%d %d\n255\n", width, height);
  glReadBuffer(GL_FRONT);
  for (int row=height-1;row>=0;row--){
    glReadPixels(0, row, (GLsizei)width, (GLsizei)1,
                 GL_RGB, GL_UNSIGNED_BYTE, image);
    fwrite(image, 3, width, fp);
  }

  delete[] image;
  fclose(fp);
}


void KeyEvent(unsigned char key, int x, int y){
  if (key == 'q'){
    exit(0);
  }
}

void TimerEvent(int v){
  snapshot();
  exit(0);
}

bool done;

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  render();
  glutSwapBuffers();
  glFlush();
  glFinish();
  done = true;
}

void read_data(const char *filename){
  FILE *fp = fopen(filename, "rt");
  if (NULL == fp){
    FATAL_ERROR("cannot open %s for reading", filename);
  }
  while (!feof(fp)){
    v3d point;
    fscanf(fp, "%lf %lf %lf", &point.x(), &point.y(), &point.z());
    if (!feof(fp)){
      model_points.push_back(point);
    }
  }
  fclose(fp);
}

int main(int argc, char **argv){
  scene.load(argv[1]);
  OpenGLProjector cam(argv[2]);
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(cam.getWidth(),cam.getHeight());
  glutCreateWindow("OpenGL projector test");

  width = cam.getWidth();
  height = cam.getHeight();

  glutKeyboardFunc(KeyEvent);
  glutDisplayFunc(display);

  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LEQUAL); glDepthRange(0.0, 1.0); glEnable(GL_DEPTH_TEST);
  cam.setOpenGLCamera();
                      
  filename = argv[3];
  glutTimerFunc(1000, TimerEvent, 0);
  glutMainLoop();
  return 0;
}
