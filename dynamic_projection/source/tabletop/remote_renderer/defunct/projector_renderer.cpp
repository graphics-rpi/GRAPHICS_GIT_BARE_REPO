#include "projector_renderer.h"

#define WIDTH 640
#define HEIGHT 480
#define UNCOMP_BUFF_SIZE 1024*1024*4
#define COMP_BUFF_SIZE 1024*1024*4
#define LINE_SIZE 256
#define fgets sr.Jgets

// should these really be globally defined? (FIX)
unsigned int throwaway;
char large_buffer[UNCOMP_BUFF_SIZE];
char uncompressed_buffer[UNCOMP_BUFF_SIZE];
char compressed_buffer[COMP_BUFF_SIZE];
int RanK; // ugh... this is spelled this way bc somehow "rank" is ambiguously defined... likely in one of the included files...


ProjectorRenderer::ProjectorRenderer(int w, int h){
  width=w;
  height=h;
  listBuilt = false;
}


void ProjectorRenderer::loadCompressedTexture(GLuint texture_idx, 
					      SocketReader & sr){
  //printf("loading texture %i\n", texture_idx);
  assert(GL_NO_ERROR==glGetError());
  char line[LINE_SIZE];
  int height,width, bytes;
  sr.Jgets(line,LINE_SIZE);
  

  sscanf(line,"%i %i %i",&height, &width,&bytes);
  //printf("height %i  width %i \n", height,width);
  //sleep(3);
  GLuint texture_id;
  glEnable(GL_TEXTURE_2D);    
  //glGenTextures(1, &texture_id); // commented out by dolcea
  
  assert(GL_NO_ERROR==glGetError());    
  if (textures.size() < (texture_idx+1)){
    textures.resize(texture_idx+1,0);
  }
  if(0==textures.at(texture_idx)){
    glGenTextures(1, &texture_id);
    //numGenTextures++;
    //printf("%i textures generated\n", numGenTextures);
  }
  textures.at(texture_idx) = texture_id;
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  
  assert(GL_NO_ERROR==glGetError());
  sr.Jread((char*)large_buffer, 1, bytes);
  
  
  glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			 width, height, 0, bytes, large_buffer);
  assert(GL_NO_ERROR==glGetError());
  
}

void ProjectorRenderer::loadCompressedTexture_z(GLuint texture_idx, 
						SocketReader & sr){

  // printf("loading texturez%i\n", texture_idx);
  bool first=false;
  assert(GL_NO_ERROR==glGetError());
  
  int height,width, bytes;
  int compressed_size;
  char line[LINE_SIZE]; 
  sr.Jgets(line,LINE_SIZE); 

  unsigned long restored_size;
  sscanf(line,"%i %i %i",&height, &width,&compressed_size);
  //printf("height %i  width %i \n", height,width);
  //sleep(3);
  glEnable(GL_TEXTURE_2D);    

  assert(GL_NO_ERROR==glGetError());    
  if (textures.size() < (texture_idx+1)){
    textures.resize(texture_idx+1,0);
  }
  if(0==textures.at(texture_idx)){
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    first=true;
    textures.at(texture_idx) = texture_id;
    //numGenTextures++;
    //printf("%i textures generated\n", numGenTextures);
  }
  //printf("number of textures: %i \n", textures.size());
  glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  
  unsigned long unc=UNCOMP_BUFF_SIZE;
  assert(GL_NO_ERROR==glGetError());
  unsigned long long_compressed_size=compressed_size;
  
  sr.Jread((char*)compressed_buffer, 1, compressed_size);
  
  
  int ret_val2 = uncompress((Bytef*)uncompressed_buffer,  &unc,
                            (Bytef*)compressed_buffer, long_compressed_size);
  
  
  assert (Z_OK == ret_val2);
  if(first){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
		 //                 height,width, 0,
                 width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE,
                 (void*)uncompressed_buffer);
    }
    else{
    glTexSubImage2D(GL_TEXTURE_2D, 0,
		    //                    0, 0, height, width,
                    0, 0, width, height,
                    GL_RGB, GL_UNSIGNED_BYTE,
                    (void*)uncompressed_buffer);
    }
  assert(GL_NO_ERROR==glGetError());
  
  //printf("end load texture z\n");
}


void ProjectorRenderer::parseCommandStream(SocketReader & sr,
					   const LI::Window& w){
  int count = 0;
  MPI_Comm_rank ( MPI_COMM_WORLD, &RanK );

  //LoadShader();
  
  // set the blending id to be the rank
  mesh.setBlendingID(RanK);
  mesh.setBlendingBrightness(1);

  sr.send_ack();
  char textline[LINE_SIZE];
  bool once_per_five=false;
  int rows, cols, first_slice, last_slice;
  char file_template[1024];
  int texture_id;
  int blending_id;
  int rgb_hack;

  while(true){
    count++;
    sr.Jgets(textline, LINE_SIZE);

    if (!strcmp(textline, "BEGIN OBJ_FILE\n")){ 
      mesh.loadFromOBJ(sr);
    } else if (!strcmp(textline, "BEGIN TEXTURE_MAP_MAP\n")){ 
      mesh.loadTextureMapMap(sr);
    } else if (!strcmp(textline, "BEGIN BLE_FILE\n")){ 
      mesh.loadBleFile(sr);
    } else if (1 == sscanf(textline, "SWITCH TO MULTIDISPLAY %d\n",&rgb_hack)){
      mesh.loadMultidisplay(sr);
    } else if (!strcmp(textline, "DISABLE BLENDING\n")){ 
      mesh.disableBlending();      
    } else if (!strcmp(textline, "BEGIN MTL_FILE\n")){ 
      mesh.loadMtlFile(sr); //, volume_texture_enable);
    } else if (1 == sscanf(textline, "LOAD TEXTURE %d", &texture_id)){
      //printf("load tex\n");
#ifdef COMPRESSED_TEX
      loadCompressedTexture_z( texture_id,sr);        
#else
      //printf("loading compressed texture %i\n",texture_id);
      Image<sRGB> image(sr);
      displayImage(image);
      glXWaitVideoSyncSGI(1, 0, &throwaway);
      w.flushDisplay();
      InitTexture(texture_id, image);
#endif
      //sr.send_ack();
      //sr.send_ack_to_all();
    } else if (1 == sscanf(textline, "UPDATE TEXTURE %d", &texture_id)){
      //printf("up tex\n");
#ifdef COMPRESSED_TEX    
      loadCompressedTexture(texture_id, sr);      
#else
      Image<sRGB> image(sr);
      UpdateTexture(texture_id, image);
#endif      
      //sr.send_ack_to_all();      
    } else if (1 == sscanf(textline, "BLENDING ID %d", &blending_id)){
      mesh.setBlendingID(blending_id);      
    } else if (!strcmp(textline, "GLCAM\n")){
      glProjector.Load(sr);
      glProjector.setOpenGLCamera();
    } else if (!strcmp(textline, "DISPLAY IMAGE\n")){
      Image<sRGB> image(sr);
      displayImage(image);
      //	glXWaitVideoSyncSGI(1, 0, &throwaway);
      w.flushDisplay();
    } else if (!strcmp(textline, "BLANK\n")){
      printf("blanking\n");
      sr.send_ack();    
      //printf("remote renderer blanking~~~!!!n");
      blank();
      w.flushGL();
      w.flushDisplay();
      sleep(1); 
      printf("end blanking\n");
    } else if (!strcmp(textline, "RENDER\n")){
      //printf("before render\n");
      sr.send_ack();
      render();
      //printf("after render\n");
    } else if (!strcmp(textline, "FLIP\n")){
      //printf("before flip\n");
      w.flushGL();    
      MPI_Barrier(MPI_COMM_WORLD);
      //	glXWaitVideoSyncSGI(1, 0, &throwaway);
      w.flushDisplay();      
    } else if (!strcmp(textline, "QUIT\n")){      
      printf("quitting...\n");
      MPI_Finalize();
      exit(0);
    } else{
      printf("missed all conditions\n");
    }
  }
}


void ProjectorRenderer::blank(){
  assert(GL_NO_ERROR==glGetError());
  glClearColor(0.0,0.0,0.0,0); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  //printf("renderers blanking %i\n", RanK);
  assert(GL_NO_ERROR==glGetError());
  // sleep(10);
}

void ProjectorRenderer::render(){
  assert(GL_NO_ERROR==glGetError());
  glClearColor(0.0,0.0,0.0,0); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  assert(GL_NO_ERROR==glGetError());
  // render with sRGB
#ifdef GL_EXT_framebuffer_sRGB
  //glEnable(GL_FRAMEBUFFER_SRGB_EXT);
  assert(GL_NO_ERROR==glGetError());
#endif
  assert(GL_NO_ERROR==glGetError());
  v3d center;
  glProjector.getCenter(center.x(), center.y(), center.z());
  assert(GL_NO_ERROR==glGetError());
  
  static int JUNK = -10;
  
  //    if(listBuilt) {
  //  glCallList(displayList);
  //} else {
  
  if (JUNK != -10) {
    glDeleteLists(JUNK,1);
  }
  JUNK = glGenLists(1);
  //    displayList = glGenLists(1);
  //glNewList(displayList, GL_COMPILE_AND_EXECUTE);
  glNewList(JUNK, GL_COMPILE_AND_EXECUTE);
  //printf("entering mesh render\n");
  mesh.render(center, textures); 
  //printf("exiting mesh render\n");
  
  glEndList();
  listBuilt = true;
  //}
  
  
#ifdef GL_EXT_framebuffer_sRGB
  //  glDisable(GL_FRAMEBUFFER_SRGB_EXT);
#endif
  assert(GL_NO_ERROR==glGetError());
}

// necessary ?
void ProjectorRenderer::flip(){
  
}

void ProjectorRenderer::displayImage(Image<sRGB> &image){
  printf("displaying image\n");
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glOrtho(0.0,                
	  (GLdouble)width,
	  (GLdouble)height,
	  0.0,                
	  0.0, 1.0);          
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glPixelZoom(1.0, -1.0);
  glRasterPos2i(0, 0);
  glDrawPixels(image.getCols(), image.getRows(), GL_RGB, GL_UNSIGNED_BYTE, 
	       image.getData());
  assert(GL_NO_ERROR==glGetError());
}

void ProjectorRenderer::loadUncompressedTexture(int texture_idx, Image<sRGB> &image){
  bool first=true;
  //if(puzzle&&!puzzlefirst)
  //  first=false;
  
  GLuint texture_id;
  glEnable(GL_TEXTURE_2D);    
  
  if (textures.size() < (texture_idx+1)){
    textures.resize(texture_idx+1,0);
  }
  
  if(first) {
    glGenTextures(1, &texture_id);
    textures.at(texture_idx) = texture_id;
  } else {
    texture_id = textures.at(texture_idx);
  }    
  
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
  
  assert(GL_NO_ERROR==glGetError());

  // use texture mipmap
  // glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
  //                  GL_LINEAR_MIPMAP_LINEAR );
  // glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
  //                  GL_LINEAR_MIPMAP_LINEAR );
  
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  
  assert(GL_NO_ERROR==glGetError());
  
  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, image.getCols(), 
		     image.getRows(),
		     GL_RGB, GL_UNSIGNED_BYTE, (void*)image.getData());
  
  assert(GL_NO_ERROR==glGetError());
  
  if(first){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
		 image.getCols(), image.getRows(), 0,
		 GL_RGB, GL_UNSIGNED_BYTE,
		 (void*)image.getData());
  }
  else{
    glTexSubImage2D(GL_TEXTURE_2D, 0,
		    0, 0, image.getCols(), image.getRows(),
		    GL_RGB, GL_UNSIGNED_BYTE,
		    (void*)image.getData());      
  }
  
  assert(GL_NO_ERROR==glGetError());
}

void ProjectorRenderer::updateTexture(int texture_idx, Image<sRGB> &image){
  glEnable(GL_TEXTURE_2D);    
  glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
  glTexSubImage2D(GL_TEXTURE_2D, 0,
		  0, 0, image.getCols(), image.getRows(),
		  GL_RGB, GL_UNSIGNED_BYTE,
		  (void*)image.getData());
  assert(GL_NO_ERROR==glGetError());
}


// hopefully not needed 
v3d cameraToProjector(v3d point){
  m3d m(1, 0, 0,
	0, 0, -1,
	0, 1, 0);
  return m * point;
}
