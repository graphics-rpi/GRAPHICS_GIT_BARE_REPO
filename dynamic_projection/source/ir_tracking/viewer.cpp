// Included files for OpenGL Rendering
#include "../applications/paint/gl_includes.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "viewer.h"

/*  Create checkerboard texture  */
//#define checkImageWidth 800
//#define checkImageHeight 800

static GLubyte my_image[IMAGE_HEIGHT][IMAGE_WIDTH][3];

static GLuint texName;

void initialize_my_image(void)
{
  int i, j, c;
  // start with a silly checkerboard image
  for (i = 0; i < IMAGE_HEIGHT; i++) {
    for (j = 0; j < IMAGE_WIDTH; j++) {	
      int testx = (i&0x8)==(0);
      int testy = (j&0x8)==(0);
      c = (testx^testy) * 255;
      my_image[i][j][0] = (GLubyte) c;
      my_image[i][j][1] = (GLubyte) c;
      my_image[i][j][2] = (GLubyte) c;
      
    }
  }
}

   /*
void image_to_texture(const Image<byte> &image) {
  std::cout << "image_to_texture START" << std::endl;
  int mr = std::min (image.getRows(),IMAGE_HEIGHT);
  int mc = std::min (image.getCols(),IMAGE_WIDTH);

  assert (image.getRows() >= IMAGE_HEIGHT);
  assert (image.getCols() >= IMAGE_WIDTH);

  for (int i=0; i<mr; i++){
    for (int j=0; j<mc; j++){
      byte b = image(i,j);
      my_image[i][j][0] = b;
      my_image[i][j][1] = b;
      my_image[i][j][2] = b;
    }
  }

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMAGE_WIDTH, 
                IMAGE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 
                my_image);
  std::cout << "image_to_texture END" << std::endl;
}
   */

void image_to_texture(const Image<sRGB> &image) {
  int mr = std::min (image.getRows(),IMAGE_HEIGHT);
  int mc = std::min (image.getCols(),IMAGE_WIDTH);

  assert (image.getRows() >= IMAGE_HEIGHT);
  assert (image.getCols() >= IMAGE_WIDTH);


  for (int i=0; i<mr; i++){
    for (int j=0; j<mc; j++){
      sRGB color = image(i,j);
      //byte b = image(i,j);
      my_image[i][j][0] = color.r();
      my_image[i][j][1] = color.g();
      my_image[i][j][2] = color.b();
    }
  }

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMAGE_WIDTH, 
                IMAGE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 
                my_image);
}



void init(void)
{    
   glClearColor (0.0, 0.0, 0.5, 0.0);
   glShadeModel(GL_FLAT);
   glEnable(GL_DEPTH_TEST);

   initialize_my_image();
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glGenTextures(1, &texName);
   glBindTexture(GL_TEXTURE_2D, texName);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                   GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                   GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMAGE_WIDTH, 
                //IMAGE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 
                IMAGE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 
                my_image);
}

void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_TEXTURE_2D);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
   glBindTexture(GL_TEXTURE_2D, texName);
   glBegin(GL_QUADS);


   glTexCoord2f(0.0, 1.0); glVertex3f(-2.7697,-2.0765,0);
   glTexCoord2f(0.0, 0.0); glVertex3f(-2.7697, 2.0765,0);
   glTexCoord2f(1.0, 0.0); glVertex3f( 2.7697, 2.0765,0);
   glTexCoord2f(1.0, 1.0); glVertex3f( 2.7697,-2.0765,0);


   glEnd();
   glutSwapBuffers();
   glDisable(GL_TEXTURE_2D);
}

void reshape(int w, int h)
{

  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -3.6);
  
}

extern int viewer_which_camera;
extern bool save_micro_image;

void keyboard (unsigned char key, int x, int y)
{
  switch (key) {
  case 27:
    exit(0);
    break;
  case 's':
    if (!save_micro_image) {
      std::cout << "request save micro image " << std::endl;
      save_micro_image = true;
    }
    break;
  case 't':
    std::cout << "t " << viewer_which_camera << std::endl;
    viewer_which_camera++;
    break;
  case 'q':
    exit(0);
  default:
    break;
  }
}

void idle(void) {
  my_idle();
  display();
  usleep(100);
}

int viewer_start(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(IMAGE_WIDTH, IMAGE_HEIGHT);
   glutInitWindowPosition(100, 100);
   glutCreateWindow(argv[0]);
   init();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);
   return 0; 
}



int viewer_loop()
{
   glutMainLoop();
   return 0;
}
