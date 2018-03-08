#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glext.h>
//#include <SDL/SDL.h>
//#include <SDL/SDL_opengl.h>
#include <OpenGLProjector.h>
#include <util.h>
#include <Vector3.h>
#include <Matrix3.h>
#include <Image.h>
#include <X11/Xlib.h>
#include <mpi.h>
#include "Mesh.hpp"
#include <zlib.h>
#include <cstdio>
#include "VolumetricData.hpp"
#include <string>
#include <sstream>

#include "shader.h"

//char montage_string[256]="~/Checkout/JOSH_EMPAC_2010/puzzle_textures/gary_hudson_montage/hudson_river_montage_";
char red_string[256]="/home/grfx/red.ppm";
char green_string[256]="/home/grfx/green.ppm";
char blue_string[256]="/home/grfx/blue.ppm";
//FIX ME!!!!
#define WIDTH 640
#define HEIGHT 480
#define UNCOMP_BUFF_SIZE 1024*1024*4
#define COMP_BUFF_SIZE 1024*1024*4
#define LINE_SIZE 256
#define fgets sr.Jgets
unsigned int throwaway;
char large_buffer[UNCOMP_BUFF_SIZE];
char uncompressed_buffer[UNCOMP_BUFF_SIZE];
char compressed_buffer[COMP_BUFF_SIZE];
char *walls[5];
string sides[5];
bool texture_changed[5] = {true, true, true, true, true};
bool renderGeom=true;
bool puzzle,puzzlefirst;
    int RanK;
int numGenTextures=0;
static const double projector_brightness[10] 
= {1.0, 1.0, 1.0, 1.0, 1.0, 
   1.0,1.0,1.0,1.0,1.0};

class RemoteProjector {
public:

  static RemoteProjector* Instance(int width, int height){
    if (0 == instance){
    instance = new RemoteProjector(width, height);
    }

    return instance;
    
  }


  ~RemoteProjector(){

    if (0 != instance){

      delete instance;

    }
  }
  
  void LoadShader(){ //int projector_id){    
    // init glew for shaders
    GLenum err = glewInit();
    if (GLEW_OK != err){ 
      printf("Error: %s\n", glewGetErrorString(err));
    }

    if(!(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)){     
      std::cerr << "Error: missing GL extensions" << std::endl;
      return;
    }

    // load vertex shaders
    sh_weights = new ShaderSet();
    sh_no_weights = new ShaderSet();

    // load fragment shaders
    sh_weights->loadVertexShader("vert.glsl");
    sh_no_weights->loadVertexShader("vert.glsl");

    // load frag shader with color weights
    sh_weights->loadFragmentShader("weight_colors.glsl");
      
    // read RBG weights from config file
    float r, g, b;
    r = g = b = 1;
    
    char filename[265];
    sprintf(filename, "color_weights/color_weights_%d.dat", RanK); //projector_id);
    std::ifstream fin(filename);
    fin >> r >> g >> b;
    std::cerr << "RGB = " << r << " " << g << " " << b << std::endl;
    
    sh_weights->initUniform3f("weights", r, g, b);
    
    //std::cout << "Loading standard fragment shader" << std::endl;
    sh_no_weights->loadFragmentShader("frag.glsl");

    if(use_color_weights){
      sh_weights->enable();
    }
    else{
      sh_no_weights->enable();
    }
  }
  

  void LoadCompressedTexture(GLuint texture_idx, SocketReader & sr)
  {
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
    glGenTextures(1, &texture_id);


    assert(GL_NO_ERROR==glGetError());    
    if (textures.size() < (texture_idx+1)){
      textures.resize(texture_idx+1,0);
    }
    if(0==textures.at(texture_idx))
    {
      glGenTextures(1, &texture_id);
      numGenTextures++;
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

  void LoadCompressedTexture_z(GLuint texture_idx, SocketReader & sr)
  {

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
    if(0==textures.at(texture_idx))
    {
      GLuint texture_id;
      glGenTextures(1, &texture_id);
      first=true;
      textures.at(texture_idx) = texture_id;
   numGenTextures++;
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


  void ParseCommandStream(SocketReader & sr,const LI::Window& w){
    int count=0;


    MPI_Comm_rank ( MPI_COMM_WORLD, &RanK );

    //LoadShader();

    // set the blending id to be the rank
    mesh.setBlendingID(RanK);
    mesh.setBlendingBrightness(1);

    sr.send_ack();
    char textline[LINE_SIZE];
    puzzle=false;
    puzzlefirst=true;
    bool once_per_five=false;
    v3d center;
    v3d targetCenter;
    double targetRadius;
    int score1, score2;
    int rows, cols, first_slice, last_slice;
    double x_court_dim, z_court_dim;
    char file_template[1024];
    int texture_id;
    int blending_id;
    int pen_detected, penVertical, targetVertical;
    int rgb_hack;

    std::vector<GLuint> space_invaders_textures;

    while(true){
      count++;
      sr.Jgets(textline, LINE_SIZE);

 
      if (!strcmp(textline, "BEGIN OBJ_FILE\n")){ 
        //printf("obj\n");
      	mesh.Load(sr);
      } else if (!strcmp(textline, "BEGIN TEXTURE_MAP_MAP\n")){ 
        //printf("tmm\n");
      	mesh.loadTextureMapMap(sr);
      } else if (!strcmp(textline, "BEGIN BLE_FILE\n")){ 
        //printf("before ble file\n");
        //printf("ble\n");
        mesh.loadBleFile(sr);
				//printf("after ble file\n");
      } else if (1 == sscanf(textline, "SWITCH TO PUZZLE %d\n",&rgb_hack)){
        //printf("puzz\n");
				//} else if (!strcmp(textline, "SWITCH TO PUZZLE\n")){ 
        //printf("before puzzle stuff\n");
        puzzle=true;
        mesh.LoadPuzzleFile(sr, sides, texture_changed);
        LoadPuzzleTextures(rgb_hack);
        puzzlefirst=false;
				//printf("after puzzle stuff\n");

      }else if (1 == sscanf(textline, "SWITCH TO MULTIDISPLAY %d\n",&rgb_hack)){
        //printf("begin load multidisplay\n");
        //sleep(1);
				//} else if (!strcmp(textline, "SWITCH TO PUZZLE\n")){ 
        //printf("before puzzle stuff\n");
        //puzzle=true;
        mesh.LoadMultidisplay(sr, sides, texture_changed);
      //  LoadMultidisplayTextures(rgb_hack);
        //puzzlefirst=false;
				//printf("after puzzle stuff\n");
        //printf("after loading multidisplay\n");
      } else if (!strcmp(textline, "DISABLE BLENDING\n")){ 
        //once_per_five=true;
	      mesh.disableBlending();

      } else if(!strcmp(textline, "TOGGLE COLOR WEIGHTS\n")){
	ToggleColorWeights();
      } else if (!strcmp(textline, "BEGIN MTL_FILE\n")){ 
        //printf("mtl\n");

	      mesh.loadMtlFile(sr, volume_texture_enable);

      } else if (7 == sscanf(textline, "PONG %lf %lf %lf %lf %lf %d %d",
                             &center.x(), &center.y(), &center.z(),
                             &x_court_dim, &z_court_dim,
                             &score1, &score2)){
	
      	pong(center, x_court_dim, z_court_dim, score1, score2);

      } else if(10 == sscanf(textline, "PEN_DEMO %d %lf %lf %lf %d %lf %lf %lf %d %lf",
			    &pen_detected, &center.x(), &center.y(), 
			    &center.z(), &penVertical, &targetCenter.x(), 
			    &targetCenter.y(), &targetCenter.z(), &targetVertical,
			    &targetRadius)){
	pen_demo(pen_detected, center, penVertical, targetCenter, targetVertical, targetRadius);
      } else if(std::string(textline).find("SPACE_INVADERS") == 0){
	if(space_invaders_textures.empty()){
	  load_space_invaders_textures(space_invaders_textures);
	}
	space_invaders(std::string(textline), space_invaders_textures);
      } else if (1 == sscanf(textline, "LOAD TEXTURE %d", &texture_id)){
        //printf("load tex\n");
#ifdef COMPRESSED_TEX
      	LoadCompressedTexture_z( texture_id,sr);
        
#else
        //printf("loading compressed texture %i\n",texture_id);
      	Image<sRGB> image(sr);
      	DisplayImage(image);
      	glXWaitVideoSyncSGI(1, 0, &throwaway);
      	w.flushDisplay();
      	InitTexture(texture_id, image);
#endif
            //sr.send_ack();
  //sr.send_ack_to_all();
      } else if (1 == sscanf(textline, "UPDATE TEXTURE %d", &texture_id)){
        //printf("up tex\n");
#ifdef COMPRESSED_TEX
    
	      LoadCompressedTexture(texture_id, sr);
   
#else
	      Image<sRGB> image(sr);
	      UpdateTexture(texture_id, image);
#endif

    //sr.send_ack_to_all();

      } else if (5 == sscanf(textline, "LOAD 3DTEXTURE %d %d %d %d %1024s", 
                             &rows, &cols, &first_slice, &last_slice, 
                             file_template)){
        volume_data.load(rows, cols, first_slice, last_slice, file_template);
        volume_data.initTexture();

      } else if (!strcmp(textline, "ENABLE VOLUME_TEXTURE\n")){ 

	      volume_texture_enable = true;

      } else if (!strcmp(textline, "DISABLE VOLUME_TEXTURE\n")){ 

	      volume_texture_enable = false;

      } else if (1 == sscanf(textline, "BLENDING ID %d", &blending_id)){

	      mesh.setBlendingID(blending_id);

      } else if (!strcmp(textline, "GLCAM\n")){
        //printf("glcam\n");
        //printf("glcams loading\n");
	      gl_projector.Load(sr);
	      gl_projector.setOpenGLCamera();
        //printf("glcams done loading\n");

      } else if (!strcmp(textline, "DISPLAY IMAGE\n")){

     
	Image<sRGB> image(sr);
	DisplayImage(image);
//	glXWaitVideoSyncSGI(1, 0, &throwaway);
	w.flushDisplay();

      } else if (!strcmp(textline, "BLANK\n")){
        printf("blanking\n");
	      sr.send_ack();    
        //printf("remote renderer blanking~~~!!!n");
       	Blank();	

        w.flushGL();
        w.flushDisplay();
         sleep(1); 
        printf("end blanking\n");

      } else if (!strcmp(textline, "RENDER\n")){
				//printf("before render\n");
      	sr.send_ack();
      	Render();
				//printf("after render\n");
                
      } else if (!strcmp(textline, "FLIP\n")){
	//printf("before flip\n");
	      w.flushGL();    
				//printf("before barrier\n");
        if(once_per_five){
  	      if(0==count%5)
	          MPI_Barrier(MPI_COMM_WORLD);

        }
        else{
          MPI_Barrier(MPI_COMM_WORLD);
				}
				//printf("after barrier\n");
				//printf("after flip\n");
        
  

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

  void Blank(){
    assert(GL_NO_ERROR==glGetError());
    glClearColor(0.0,0.0,0.0,0); 
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //printf("renderers blanking %i\n", RanK);
    assert(GL_NO_ERROR==glGetError());
   // sleep(10);
  }

  void Render(){
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
    gl_projector.getCenter(center.x(), center.y(), center.z());
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
      mesh.Render(center, textures, 	   volume_data.getTextureID(),		   volume_texture_enable);
      //printf("exiting mesh render\n");

      glEndList();
      listBuilt = true;
    //}


#ifdef GL_EXT_framebuffer_sRGB
  //  glDisable(GL_FRAMEBUFFER_SRGB_EXT);
#endif
assert(GL_NO_ERROR==glGetError());
  }

  void Flip(){
    
  }

  void DisplayImage(Image<sRGB> &image){
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

  void LoadUncompressedTexture(int texture_idx, Image<sRGB> &image){
    bool first=true;
    if(puzzle&&!puzzlefirst)
      first=false;

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
    }else{
      glTexSubImage2D(GL_TEXTURE_2D, 0,
		      0, 0, image.getCols(), image.getRows(),
		      GL_RGB, GL_UNSIGNED_BYTE,
		      (void*)image.getData());      
    }
 
  assert(GL_NO_ERROR==glGetError());
  }

  void LoadPuzzleTextures(int rgb_hack){
    //printf("begin load puzzle\n");

    if(textures.empty()) {
      textures.resize(5,0);
    }
    char file[256];

	  for(int i=0;i<=4;i++){
  	    if(texture_changed[i]) {
					if(rgb_hack==0)
			      sprintf(file,"%s%s.ppm",montage_string,sides[i].c_str());
					else if(rgb_hack==1)
	          sprintf(file,"%s",red_string);
					else if(rgb_hack==2)
	          sprintf(file,"%s",green_string);
					else if(rgb_hack==3)
            sprintf(file,"%s",blue_string);

				  Image <sRGB> image(file);
				  LoadUncompressedTexture(i,image);

    		}
				
			
		}
		
      
      
    assert(GL_NO_ERROR==glGetError());
  }

  void UpdateTexture(int texture_idx, Image<sRGB> &image){


    glEnable(GL_TEXTURE_2D);    
    glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0, image.getCols(), image.getRows(),
                    GL_RGB, GL_UNSIGNED_BYTE,
                    (void*)image.getData());
    assert(GL_NO_ERROR==glGetError());
  }

  void ToggleColorWeights(){
    use_color_weights = !use_color_weights;
    if(use_color_weights){
      sh_weights->enable();
      std::cout << "color weights ON" << std::endl;
    }
    else{
      sh_no_weights->enable();
      std::cout << "color weights OFF" << std::endl;
    }
  }

private:
  static RemoteProjector *instance;
  int height, width;
  OpenGLProjector gl_projector;
  Mesh mesh;
  std::vector<int> textures;
  VolumetricData volume_data;
  bool volume_texture_enable;
  GLuint displayList;  // display list used to render
  bool listBuilt;

  ShaderSet* sh_weights;
  ShaderSet* sh_no_weights;
  bool use_color_weights;

  void pong(v3d center, double xdim, double zdim, int score1, int score2){
    glColor3f(1.f, 1.f, 1.f);
    glBegin(GL_TRIANGLE_FAN);
    int n_sides = 30;
    double offset = 0.001;
    double r = 0.1;
    glVertex3d(center.x(), center.y()+offset, center.z());
    for (int i=0; i<n_sides; i++){
      double th = (6.28318530717959 * i) / (n_sides-1);
      double x = center.x() + r * cos(th);
      double z = center.z() + r * sin(th);
      glVertex3d(x, center.y()+offset, z);
    }
    glEnd();
    glLineWidth(3.5);
    glBegin(GL_LINES);
    //    double s = 3.658;
    glVertex3d(xdim,0.,zdim);
    glVertex3d(xdim,0.,-zdim);

    glVertex3d(xdim,0.,-zdim);
    glVertex3d(-xdim,0.,-zdim);

    glVertex3d(-xdim,0.,-zdim);
    glVertex3d(-xdim,0.,zdim);

    glVertex3d(-xdim,0.,zdim);
    glVertex3d(xdim,0.,zdim);

    glVertex3d(xdim,0.,0.);
    glVertex3d(-xdim,0.,0.);

    glEnd();
  }

  v3d cameraToProjector(v3d point){
    m3d m(1, 0, 0,
	  0, 0, -1,
	  0, 1, 0);
    return m * point;
  }

  void draw_disk(double cx, double cy, double cz, bool vertical, double radius){
    double offset = 0.001;
    int n_sides = 30;
    double x, y, z;
    glBegin(GL_TRIANGLE_FAN);
    if(vertical){
      glVertex3d(cx - 2*offset, cy, cz);	
    }
    else{
      glVertex3d(cx, cy + 2*offset, cz);
    }
    for (int i=0; i<n_sides; i++){
      double th = (6.28318530717959 * i) / (n_sides-1);	
      if(vertical){
	x = cx - 2*offset;
	y = cy + radius * cos(th);
	z = cz + radius * sin(th);
      }
      else{
	x = cx + radius * cos(th);
	y = cy + 2*offset;
	z = cz + radius * sin(th);
      }
      glVertex3d(x, y, z);
    }
    glEnd();
  }

  void draw_textured_square(GLuint texture, double cx, double cy, double cz, bool vertical, double sideLen){    
    double offset = 0.001;
    double halfLen = sideLen/2;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    if(vertical){
      cx -= offset;
      glTexCoord2d(0.0,0.0); 
      glVertex3d(cx, cy + halfLen, cz - halfLen);
      glTexCoord2d(1.0,0.0); 
      glVertex3d(cx, cy + halfLen, cz + halfLen);
      glTexCoord2d(1.0,1.0); 
      glVertex3d(cx, cy - halfLen, cz + halfLen);
      glTexCoord2d(0.0,1.0); 
      glVertex3d(cx, cy - halfLen, cz - halfLen);
    }
    else{
      cy += offset;
      glTexCoord2d(0.0,0.0); 
      glVertex3d(cx + halfLen, cy, cz - halfLen);
      glTexCoord2d(1.0,0.0); 
      glVertex3d(cx + halfLen, cy, cz + halfLen);
      glTexCoord2d(1.0,1.0); 
      glVertex3d(cx - halfLen, cy, cz + halfLen);
      glTexCoord2d(0.0,1.0); 
      glVertex3d(cx - halfLen, cy, cz - halfLen);
    }
    glEnd();
  }

  void load_space_invaders_textures(std::vector<GLuint>& textures){
    textures.clear();
    
    std::string filenames[1] = {"space_invader.ppm"};
    std::string filename, dirname = "/home/grfx/Checkout/JOSH_EMPAC_2010/space_invaders_textures/";
    int width, height, numTextures = 1;
    GLuint texture;
    Bytef * data;
    FILE * file;

    for(int i = 0; i < numTextures; i++){
      std::string filename = dirname;
      filename += filenames[i];
      std::cout << "file tttttttttttt " << filename << std::endl;
      Image<sRGB> image(filename.c_str());
      
      // allocate buffer
      width = image.getCols();
      height = image.getRows();
      data = (Bytef*)image.getData();
      
      // allocate a texture name
      glGenTextures( 1, &texture );
      
      // select our current texture
      glBindTexture( GL_TEXTURE_2D, texture );
      
      // select modulate to mix texture with color for shading
      glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
      
      // when texture area is small, bilinear filter the closest mipmap
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		       GL_LINEAR_MIPMAP_NEAREST );
      // when texture area is large, bilinear filter the first mipmap
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

      // build our texture mipmaps
      gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
			 GL_RGB, GL_UNSIGNED_BYTE, data );

      // free buffer
      //free( data );

      textures.push_back(texture);

    }
  }

  // format of textline:
  // starts with : SPACE_INVADERS 
  // followed by :
  //   NONE (for no pen)
  //   FLOOR X Z (for pen on floor)
  //   WALL Z -Y (for pen on wall)
  // followed by unspecified number of targets in form:
  //   FLOOR X Z (for target on floor)
  //   WALL  Z -Y (for target on wall)
  void space_invaders(std::string textline, std::vector<GLuint>& textures){
    std::cout << "space invaders " << textline << std::endl;
    // create stringstream for parsing textline
    std::stringstream ss (std::stringstream::in | std::stringstream::out);
    ss << textline;
    std::string s;
    double cx, cy, cz;
    bool hard;
    double backWallDist = 5.2578;
    ss >> s; // SPACE_INVADERS
    
    // get pen
    ss >> s;
    bool pen_detected = true;
    bool vertical = true;
    
    double radius = 0.1;

    if(s == std::string("FLOOR")){
      ss >> cx >> cz;
      cy = 0;
      vertical = false;
    }
    else if(s == std::string("WALL")){
      ss >> cz >> cy;
      cy = -cy;
      cx = backWallDist;
      vertical = true;
    }
    else{
      pen_detected = false;
    }

    if(pen_detected){
      glColor3f(0.f, 1.f, 0.f);
      draw_disk(cx, cy, cz, vertical, radius);
    }
    
    // draw targets
    while(ss >> s){
      if(s == std::string("FLOOR")){
	ss >> cx >> cz >> hard;
	cy = 0;
	vertical = false;
      }
      else{
	ss >> cz >> cy >> hard;
	cy = -cy;
	cx = backWallDist;
	vertical = true;
      }
      if(hard){
	glColor3f(1.f, 0.f, 0.f);
      }
      else{
	glColor3f(1.f, 1.f, 1.f);
      }
      //draw_disk(cx, cy, cz, vertical, radius);
      draw_textured_square(textures[0], cx, cy, cz, vertical, 0.6);
    }

  }

  void pen_demo(int pen_detected, v3d center, bool penVertical, v3d targetCenter, 
		bool targetVertical, double targetRadius){
    targetCenter = cameraToProjector(targetCenter);
    center = cameraToProjector(center);
    // draw target
    //glEnable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

    //printf("drawing stuff for pens\n");
    printf("target at %f %f %f %f\n", targetCenter.x(), targetCenter.y(), targetCenter.z(), targetRadius);
    glColor3f(1.f, 0.f, 0.f);
    glBegin(GL_TRIANGLE_FAN);
    int n_sides = 30;
    double offset = 0.001;
    double backWallDist = 5.2578;
    double x, y, z;
    if(targetVertical){
      glVertex3d(targetCenter.x() - offset, targetCenter.y(), targetCenter.z());
    }
    else{
      glVertex3d(targetCenter.x(), targetCenter.y()+offset, targetCenter.z());
    }
    for (int i=0; i<n_sides; i++){
      double th = (6.28318530717959 * i) / (n_sides-1);
      
      if(targetVertical){
	x = targetCenter.x() - offset;
	y = targetCenter.y() + targetRadius * cos(th);
	z = targetCenter.z() + targetRadius * sin(th);
      }
      else{
	x = targetCenter.x() + targetRadius * cos(th);
	y = targetCenter.y() + offset;
	z = targetCenter.z() + targetRadius * sin(th);
      }
      glVertex3d(x, y, z);
    }
    glEnd();

    if(pen_detected){
      glColor3f(1.f, 1.f, 1.f);
      glBegin(GL_TRIANGLE_FAN);
       n_sides = 30;
      double r = 0.1;
      if(penVertical){
	glVertex3d(center.x() - 2*offset, center.y(), center.z());
	printf("pen center at %f %f %f", center.x(), center.y(), center.z());
      }
      else{
	glVertex3d(center.x(), center.y()+2*offset, center.z());
      }
      for (int i=0; i<n_sides; i++){
	double th = (6.28318530717959 * i) / (n_sides-1);
	if(penVertical){
	  x = center.x() - 2*offset;
	  y = center.y() + r * cos(th);
	  z = center.z() + r * sin(th);
	}
	else{
	  x = center.x() + r * cos(th);
	  y = center.y() + 2*offset;
	  z = center.z() + r * sin(th);
	}
	glVertex3d(x, y, z);
      }
      glEnd();
    }
  }

  RemoteProjector(int w, int h){
    width=w;
    height=h;
    volume_texture_enable = false;
    listBuilt = false;
    sh_weights = sh_no_weights = 0;
    use_color_weights = false;
  }

  static void getRootWindowSize(const char *name, int &rows, int &cols){
    Display *disp = XOpenDisplay(name);
    int screen_number = XDefaultScreen(disp);
    rows = DisplayHeight(disp, screen_number); 
    cols = DisplayWidth(disp, screen_number); 
    XCloseDisplay(disp);
  }

};
RemoteProjector * RemoteProjector::instance;


