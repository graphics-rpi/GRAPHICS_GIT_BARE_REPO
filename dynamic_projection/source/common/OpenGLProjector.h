#ifndef OPENGLPROJECTOR_H_INCLUDED_
#define OPENGLPROJECTOR_H_INCLUDED_

#include <cstdlib>
#include <cstdio>
#include <GL/gl.h>
#include "SocketReader.h"

// represent projector as OpenGL camera
class OpenGLProjector {
public:

  OpenGLProjector(){}

  // create a camera object from a *.glcam file
  OpenGLProjector(const char *filename){
    LoadFile(filename);
  }

  void LoadFile(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "Unable to open file %s\n", filename);
      exit(-1);
    }

    Load(fp);

    fclose(fp);
  }

  void Load(FILE *fp){
    char comment[1024];
    char content[1024];
    fgets(comment, 1024, fp);
    fscanf(fp, "%d %d", &width, &height);
     //printf("%d %d\n", width, height);
    

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

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    for(int i=0; i<4; ++i) {
      for(int j=0; j<3; ++j) {
	fscanf(fp, "%lf", &near_plane_verts[i][j]);
	//printf("%f ", near_plane_verts[i][j]);
      }
      //printf("\n");
    }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    fscanf(fp, "%lf", &near_plane_dist);
    //printf("%f ", near_plane_dist);
    fscanf(fp, "%lf", &far_plane_dist);
    //printf("%f ", far_plane_dist);

    // calculate far plane verts
    //   calculate vector u from center through near plane vertex
    //   extending along this vector until the far plane is hit
    // !!! will fail for near_plane_dist == 0
    if (near_plane_dist != 0.){
      for (int i=0; i<4; i++){
	double ux = (near_plane_verts[i][0] - cx);
	double uy = (near_plane_verts[i][1] - cy);
	double uz = (near_plane_verts[i][2] - cz);
	double factor = far_plane_dist / near_plane_dist;
	far_plane_verts[i][0] = cx + factor * ux;
	far_plane_verts[i][1] = cy + factor * uy;
	far_plane_verts[i][2] = cz + factor * uz;
      }
    } else {
      for (int i=0; i<4; i++){
	for (int j=0; j<3; j++){
	  far_plane_verts[i][j] = 0.;
	}
      }
    }
  }

 void Load(SocketReader & sr){
    char comment[256];
    char content[256];
    //int *x;
    while(sr.Jgets(comment, 256)==-1);
    //printf("dline 1 %s \n",comment);
    while(sr.Jgets(content, 256)==-1);
    //printf("past failed part\n");
    //printf("line 1%s \n",content);
    sscanf(content, "  %d    %d", &width, &height);
    // printf("%d %d\n", width, height);
    //printf("Got width: %d height: %d \n",width, height);
    for (int c=' ';c != EOF && c != '#'; c=sr.Jgetc()); 
    sr.Jgets(comment, 256);
    for (int i=0; i<16; i+=4){
          sr.Jgets(content, 256);
          sscanf(content, "%lf %lf %lf %lf", &projection_matrix[i],&projection_matrix[i+1],&projection_matrix[i+2],&projection_matrix[i+3]);
      // if(i==0)printf("%f\n", projection_matrix[14]);
    }
    //printf("proj matrix:%lf %lf %lf\n",projection_matrix[0],projection_matrix[5],projection_matrix[14]);
    for (int c=' ';c != EOF && c != '#'; c=sr.Jgetc()); 
    sr.Jgets(comment, 256);
    //printf("comment %s\n",comment);
     for (int i=0; i<16; i+=4){
          sr.Jgets(content, 256);
          sscanf(content, "   %lf    %lf    %lf    %lf", &modelview_matrix[i],&modelview_matrix[i+1],&modelview_matrix[i+2],&modelview_matrix[i+3]);
     //  if(i==0)      printf("content:%s\n", content);
    }
    //printf("proj matrix: %lf %lf %lf %lf \n",&modelview_matrix[0],&modelview_matrix[1],
     //   &modelview_matrix[2], &modelview_matrix[3]);
    fflush(stdout);
    for (int c=' ';c != EOF && c != '#'; c=sr.Jgetc()); 
    sr.Jgets(comment, 256);
    sr.Jgets(content, 256);
    sscanf(content, "%lf %lf %lf", &cx,&cy,&cz);

    for (int c=' ';c != EOF && c != '#'; c=sr.Jgetc()); 
    sr.Jgets(comment, 1024);

    for(int i=0; i<4; ++i) {
        sr.Jgets(content, 1024);
        sscanf(content, "%lf %lf %lf", &near_plane_verts[i][0],&near_plane_verts[i][1],&near_plane_verts[i][2]);
    }

    for (int c=' ';c != EOF && c != '#'; c=sr.Jgetc()); 
    sr.Jgets(comment, 1024);
    sr.Jgets(content, 1024);
    sscanf(content, "%lf %lf", &near_plane_dist,&far_plane_dist);
    //printf("%f ", near_plane_dist);

    //printf("%f ", far_plane_dist);

    // calculate far plane verts
    //   calculate vector u from center through near plane vertex
    //   extending along this vector until the far plane is hit
    // !!! will fail for near_plane_dist == 0
    if (near_plane_dist != 0.){
      for (int i=0; i<4; i++){
	double ux = (near_plane_verts[i][0] - cx);
	double uy = (near_plane_verts[i][1] - cy);
	double uz = (near_plane_verts[i][2] - cz);
	double factor = far_plane_dist / near_plane_dist;
	far_plane_verts[i][0] = cx + factor * ux;
	far_plane_verts[i][1] = cy + factor * uy;
	far_plane_verts[i][2] = cz + factor * uz;
      }
    } else {
      for (int i=0; i<4; i++){
	for (int j=0; j<3; j++){
	  far_plane_verts[i][j] = 0.;
	}
      }
    }
  }


  void getCenter(GLdouble &x, GLdouble &y, GLdouble &z) const{
    x = cx;
    y = cy;
    z = cz;
  }

  GLint getHeight() const{
    //    return 512;
    return height;
  }

  GLint getWidth() const{
    //    return 512;
    return width;
  }

  inline GLdouble getNearPlaneDist() const
  {
    return near_plane_dist;
  }
  
  inline GLdouble getFarPlaneDist() const
  {
    return far_plane_dist;
  }

  inline const GLdouble* getNearPlaneVertex(int i) const
  {
    if(i<4){
      return near_plane_verts[i];
    } else { 
      return 0;
    }
  }

  inline const GLdouble* getFarPlaneVertex(int i) const
  {
    if(i<4){
      return far_plane_verts[i];
    } else { 
      return 0;
    }
  }

  GLdouble *getProjectionMatrix() const{
    return (GLdouble*)&projection_matrix;
  }

  GLdouble *getModelviewMatrix() const{
    return (GLdouble*)&modelview_matrix;
  }

  // set the opengl "camera" to the view for this projector
  // NOTE: this overwrites the current projection and modelview matrices
  void setOpenGLCamera() const{
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
  // 4 vertices of near plane
  GLdouble near_plane_verts[4][3];
  // 4 vertices of far plane: calculated from near ones on load
  GLdouble far_plane_verts[4][3];
  GLdouble near_plane_dist; // near plane distance from camera center
  GLdouble far_plane_dist;  // far plane distance from camera center
};

#endif // #ifndef OPENGLPROJECTOR_H_INCLUDED_
