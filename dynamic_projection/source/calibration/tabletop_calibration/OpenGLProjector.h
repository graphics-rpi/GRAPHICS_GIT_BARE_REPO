#ifndef OPENGLPROJECTOR_H_INCLUDED_
#define OPENGLPROJECTOR_H_INCLUDED_

#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>

// represent projector as OpenGL camera
class OpenGLProjector {
public:

  // create a camera object from a *.cam file
  OpenGLProjector(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "Unable to open file %s\n", filename);
      exit(-1);
    }

    char comment[1024];
    fgets(comment, 1024, fp);
    fscanf(fp, "%d %d", &width, &height);
    // printf("%d %d\n", width, height);

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);
    for (int i=0; i<16; i++){
      fscanf(fp, "%lf", &projection_matrix[i]);
      // printf("%f\n", projection_matrix[i]);
    }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    for (int i=0; i<16; i++){
      fscanf(fp, "%lf", &modelview_matrix[i]);
      //      printf("%f\n", modelview_matrix[i]);
    }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    fscanf(fp, "%lf", &cx);
    fscanf(fp, "%lf", &cy);
    fscanf(fp, "%lf", &cz);
    fclose(fp);
  }

  void getCenter(GLdouble &x, GLdouble &y, GLdouble &z){
    x = cx;
    y = cy;
    z = cz;
  }

  GLint getHeight(){
    return height;
  }

  GLint getWidth(){
    return width;
  }

  // set the opengl "camera" to the view for this projector
  // NOTE: this overwrites the current projection and modelview matrices
  void setOpenGLCamera(){
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection_matrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(modelview_matrix);
  }

private:
  GLint height;
  GLint width;
  GLdouble cx, cy, cz;  // camera center
  GLdouble projection_matrix[16];
  GLdouble modelview_matrix[16];
};

#endif // #ifndef OPENGLPROJECTOR_H_INCLUDED_
