#ifndef __remote_render_h__
#define __remote_render_h__

#include <cstdio>
#include <cstring>
#include <cstdio>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <zlib.h>
#include <mpi.h>
#include <math.h>

#include "../../common/OpenGLProjector.h"
#include "../../common/util.h"
#include "../../common/Vector3.h"
#include "../../common/Matrix3.h"
#include "../../common/Image.h"
#include "../../common/displayParser.h"

#include "shader.h"  // (has all the gl includes)

#include "Mesh.hpp"
#include "VolumetricData.hpp"
#include "SocketReader.h"
#include "LiWindow.hxx"
#include "TextureMapMap.hpp"
#include "DirLock.hpp"
#include "../argparser.h"

#include "../../applications/paint/text.h"

// ===============================================
// A BUNCH OF GLOBAL VARIABLES

extern ArgParser *ARGS;


const char red_string[256]="/home/grfx/red.ppm";
const char green_string[256]="/home/grfx/green.ppm";
const char blue_string[256]="/home/grfx/blue.ppm";
const int group_ranks[6] ={0,1,2,3,4,5};
//FIX ME!!!!
#define WIDTH 640
#define HEIGHT 480
#define UNCOMP_BUFF_SIZE 1024*1024*4
#define COMP_BUFF_SIZE 1024*1024*4
#define LINE_SIZE 4000

//unsigned int throwaway;
extern char large_buffer[UNCOMP_BUFF_SIZE];
extern char uncompressed_buffer[UNCOMP_BUFF_SIZE];
extern char compressed_buffer[COMP_BUFF_SIZE];
extern char *walls[5];
extern string sides[5];
extern bool texture_changed[5];// = {true, true, true, true, true};
extern bool renderGeom;//=true;
extern bool puzzle,puzzlefirst;
extern int RanK;
extern int numGenTextures;//=0;

static const double projector_brightness[10] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0,1.0,1.0,1.0,1.0};

// ===============================================


class RemoteProjector {
 public:

  static RemoteProjector* Instance(int width, int height){
    if (the_instance == NULL){
      (*ARGS->output) << "instance is zero\r" << std::endl;
      the_instance = new RemoteProjector(width, height);
    }
    else
      (*ARGS->output) << "instance wasn't zero\r" << std::endl;
    return the_instance;
  }

  ~RemoteProjector(){
    if (the_instance != NULL){
      delete the_instance;
    }
  }
  
  
  void LoadShader();

  void drawText();

  // potentially temporary
  void renderToFbo(v3d center);
 
  void LoadCompressedTexture_z(GLuint texture_idx, SocketReader & sr, RRTextureMapMap & rrmap);

  // -------------------------------------------------
  // for the standalone remote renderer
  // -------------------------------------------------
  // this run function replaces ParseCommandStream
  // by cannibalizing the loop from render_controller
  // added 25 June 2013
  // -------------------------------------------------
  
  void run(const LI::Window& w, const ConfigParser& parser) {



    // from render.h
    RanK = 0;
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    
    // FROM PARSECOMMANDSTREAM()
    LoadShader();
    // set the blending id to be the rank
    mesh.setBlendingID(0);
    mesh.setBlendingBrightness(1);


    // -------------------------------------------------
    // from here on out is copied
    // pretty much verbatim from RenderController::run()
        

    // public vars from RenderController
    // we can hopefully get rid of some of these maybe?
    bool blending = false;//true;
    bool volume_texture_enabled = false;
    bool puzzle=false;
    bool red_green_walls=false;
    bool pong = false;
    bool pen_demo = false;
    bool multidisplay=false;
    bool space_invaders = false;
    bool army = false;
    bool send_obj = true;
    bool send_mtl = true;
    


    //#define OBJDIR "/home/greg/ramdisk/"
#define OBJDIR "/ramdisk/"

    // REPLACE ME -- DONE
    //ConfigParser parser(FILE_LOCATION);
    printf("in the new remote_renderer run\r\n");
    //Keeps track of the number of acknowledgment messages received.
    bool score_changed=true;
    
    

    // REPLACE ME -- DONE
    
    // OKAY TO DELETE I THINK
    //RRController* renderers [parser.displays.size()];
    //for(int i=0; i<parser.displays.size(); i++){
    //  //printf("adding display %i\r\n",i);
    //  renderers[i]=new RRController(i, parser);
    //  display.addDisplay(renderers[i]);
    //}
    

    // REPLACE ME -- DONE
    //display.blank();
    Blank();
    
    // UNNECESSARY
    char buffer[50];
     
    //This is the structure which keeps track of the correspondency between images and textures
    TextureMapMap map;
    int cnt=0;




    //These are the two locks used
    
    DirLock Tween(OBJDIR "tween");
    DirLock Slow(OBJDIR "images");


    Slow.Lock();
    //Loads all the textures found in the mtl file
    if(!puzzle&&!pen_demo&&!space_invaders && !army){
      for (unsigned i=0; i<map.getNumTextures(); i++){
	// REPLACE ME -- DONE
	//display.loadTexture(i, map.getTextureFilename(i).c_str(), renderers);
	newLoadTexture(i, map.getTextureFilename(i).c_str());
      }
    }
    
    Slow.Unlock();
    //printf("after loading all texture \r\n");

    char str[LINE_SIZE];
    char c;
    //Loads all the glcam files (it is assumed they start lettered with 'A'
    //char projector_name[256];

    // REPLACE ME -- DONE
    //for(int i=0; i < parser.displays.size(); i++)
    //  {
    //char projector_name[256];
    //	sprintf(projector_name,"%s",parser.displays[i].display_name.c_str());
    //	renderers[i]->setGLCam(projector_name);
    //     }
    
    char projector_name[256];
    // assume the display is the 0th projector
    //sprintf(projector_name,"%s",parser.displays[0].display_name.c_str());
    //sprintf(projector_name,"/home/grfx/GIT_CHECKOUT/dynamic_projection/state/table_top_calibration/projector_glcams/projector_A.glcam");
    sprintf(projector_name, OBJDIR "projector_glcams/projector_A.glcam");
    gl_projector.LoadFile(projector_name);
    gl_projector.setOpenGLCamera();

    //count, start time and end time are used to calculate fps
    int count=0;
    timeval starttime,endtime;
    gettimeofday(&starttime, NULL);

    int slow_index=0;

    int curr_texture=-1;

    bool blankThisIter=false;


    // texture test stuff -- REMOVE ME
    newLoadTexture(13452, "/ramdisk/testure.ppm");
    std:: cout << "loaded testure" << std::endl;
    usleep(100000);


    
    // this loop is run instead of the parsecommandstream() loop
    while(1){
      printf("in while\r\n");
      blankThisIter=false;

      FILE *fp, *fp2;
      fp=fopen("/ramdisk/tween/stop","r");
      fp2=fopen("/ramdisk/tween/stop2","r");
      while(fp||fp2) {
	blankThisIter=true;
	// REPLACE ME -- DONE
	//display.blank();
	Blank();
      
	// REPLACE ME -- DONE
	//renderers[0]->get_ack(); 
	if(fp)   
	  fclose(fp); //do nothing
	else
	  fclose(fp2); //do nothing
	if( remove( "/ramdisk/tween/stop" ) != 0 )
	  {
	    printf( "Error deleting file" );
	    //assert(0);
	  }
	fp=fopen("/ramdisk/tween/stop","r");
	fp2=fopen("/ramdisk/tween/stop2","r");
      }
      

      slow_index++;
      if(slow_index==map.getNumTextures()){
	slow_index=0;
      }

      //Don't update geometry if it's play a day
      // play a day runs through the times of day
      FILE* filep;
      filep=fopen("/home/grfx/Checkout/archdisplay/play.day", "r");
      if(filep==NULL)
	{
	  //Hack to make frame counter works
	  gettimeofday(&starttime, NULL);
	  cnt=0;
	  //The obj and mtl files are loaded each iteration
	  Tween.Lock();

	  // need to lock /ramdisk/images to check whether the images exist
	  if(multidisplay)
	    {
	      printf("sending multidisplay file\r\n");
	      // REPLACE ME -- DONE
	      //display.sendMultidisplayFile(map,"/home/grfx/app_data/state.txt");
	      //printf("FIX MULTIDISPLAY SENDING RIGHT NOW\r\n");
	      mesh.LoadMultidisplay("/home/grfx/app_data/state.txt", sides, texture_changed);
	      parseMultidisplayFile(map, "/home/grfx/app_data/state.txt");

	      printf("after sending multidisplay file\r\n");
	    }//end if multi
	  else
	    {	
	      //Slow.Lock();    
	      // REPLACE ME -- DONE
	      //display.loadMtl(map, OBJDIR "tween/foo.mtl");
	      //mesh.loadMtlFileFromFile(OBJDIR "tween/foo.mtl", volume_texture_enable);
	      //mesh.loadMtlFileFromFile("/home/greg/foo.mtl", volume_texture_enable);
	      mesh.loadMtlFileFromFile(OBJDIR "tween/foo.mtl", volume_texture_enable);
	      parseMtl(map, OBJDIR "tween/foo.mtl");

	      //Slow.Unlock();    
	    }//endelse

	  // REPLACE ME -- DONE
	  //display.loadMesh(OBJDIR "tween/foo.obj");
	  mesh.Load(OBJDIR "tween/foo.obj");

	  Tween.Unlock();
	    
	}//end if filep
      else{
	  
	fclose(filep);
      }
	
	

      // disable blending if we are told to do so
      if(blending){
	// REPLACE ME -- DONE
	//display.loadBle(OBJDIR "tween/foo.ble"); 
	mesh.loadBleFile(OBJDIR "tween/foo.ble");
      }
      else {
	// REPLACE ME -- DONE
	//display.disableBlending();
	mesh.disableBlending();
      }

      Tween.Unlock();
      if(blankThisIter!=true) {
	// REPLACE ME -- DONE
	//display.render();
	Render();
      }

      //Renderer 0 must always send an ack so that the sockets don't get too full
      if(blankThisIter!=true) {
	// REPLACE ME -- DONE
	//renderers[0]->get_ack();    
      }


	
      //float texcoords[8] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0};
      //float texcoords[8] = {1/16, 300/384/6, 2/16, 300/384/6, 2/16, 0, 1/16, 0};
      //float texcoords[8] = {1.0/16.0, 1.0/7.0, 2.0/16.0, 1.0/7.0, 2.0/16.0, 0.0, 1.0/16.0, 0.0};
      //float texcoords[8] = {0.0625, 0.25, 0.0625*2, 0.25, 0.0625*2, 0.0, 0.0625, 0.0};
      
      /* 
      float texcoords[8 * 26] = { 1.0/16.0, 2.0/8.0, 2.0/16.0, 2.0/8.0, 2.0/16.0, 1.0/8.0, 1.0/16.0, 1.0/8.0,
				  2.0/16.0, 2.0/8.0, 3.0/16.0, 2.0/8.0, 3.0/16.0, 1.0/8.0, 2.0/16.0, 1.0/8.0,
				  3.0/16.0, 2.0/8.0, 4.0/16.0, 2.0/8.0, 4.0/16.0, 1.0/8.0, 3.0/16.0, 1.0/8.0,
				  4.0/16.0, 2.0/8.0, 5.0/16.0, 2.0/8.0, 5.0/16.0, 1.0/8.0, 4.0/16.0, 1.0/8.0,
				  5.0/16.0, 2.0/8.0, 6.0/16.0, 2.0/8.0, 6.0/16.0, 1.0/8.0, 5.0/16.0, 1.0/8.0,
				  6.0/16.0, 2.0/8.0, 7.0/16.0, 2.0/8.0, 7.0/16.0, 1.0/8.0, 6.0/16.0, 1.0/8.0,
				  7.0/16.0, 2.0/7.0, 8.0/16.0, 2.0/7.0, 8.0/16.0, 1.0/7.0, 7.0/16.0, 1.0/7.0,
				  8.0/16.0, 2.0/7.0, 9.0/16.0, 2.0/7.0, 9.0/16.0, 1.0/7.0, 8.0/16.0, 1.0/7.0,
				  9.0/16.0, 2.0/7.0, 10.0/16.0, 2.0/7.0, 10.0/16.0, 1.0/7.0, 9.0/16.0, 1.0/7.0,
				  10.0/16.0, 2.0/7.0, 11.0/16.0, 2.0/7.0, 11.0/16.0, 1.0/7.0, 10.0/16.0, 1.0/7.0,
				  11.0/16.0, 2.0/7.0, 12.0/16.0, 2.0/7.0, 12.0/16.0, 1.0/7.0, 11.0/16.0, 1.0/7.0,
				  12.0/16.0, 2.0/7.0, 13.0/16.0, 2.0/7.0, 13.0/16.0, 1.0/7.0, 12.0/16.0, 1.0/7.0,
				  13.0/16.0, 2.0/7.0, 14.0/16.0, 2.0/7.0, 14.0/16.0, 1.0/7.0, 13.0/16.0, 1.0/7.0,
				  14.0/16.0, 2.0/7.0, 15.0/16.0, 2.0/7.0, 15.0/16.0, 1.0/7.0, 14.0/16.0, 1.0/7.0,
				  15.0/16.0, 2.0/7.0, 16.0/16.0, 2.0/7.0, 16.0/16.0, 1.0/7.0, 15.0/16.0, 1.0/7.0,
				  0.0/16.0, 1.0/7.0, 1.0/16.0, 1.0/7.0, 1.0/16.0, 0.0, 0.0/16.0, 0.0,
				  1.0/16.0, 1.0/7.0, 2.0/16.0, 1.0/7.0, 2.0/16.0, 0.0, 1.0/16.0, 0.0,
				  2.0/16.0, 1.0/7.0, 3.0/16.0, 1.0/7.0, 3.0/16.0, 0.0, 2.0/16.0, 0.0,
				  3.0/16.0, 1.0/7.0, 4.0/16.0, 1.0/7.0, 4.0/16.0, 0.0, 3.0/16.0, 0.0,
				  4.0/16.0, 1.0/7.0, 5.0/16.0, 1.0/7.0, 5.0/16.0, 0.0, 4.0/16.0, 0.0,
				  5.0/16.0, 1.0/7.0, 6.0/16.0, 1.0/7.0, 6.0/16.0, 0.0, 5.0/16.0, 0.0,
				  6.0/16.0, 1.0/7.0, 7.0/16.0, 1.0/7.0, 7.0/16.0, 0.0, 6.0/16.0, 0.0,
				  7.0/16.0, 1.0/7.0, 8.0/16.0, 1.0/7.0, 8.0/16.0, 0.0, 7.0/16.0, 0.0,
				  8.0/16.0, 1.0/7.0, 9.0/16.0, 1.0/7.0, 9.0/16.0, 0.0, 8.0/16.0, 0.0,
				  9.0/16.0, 1.0/7.0, 10.0/16.0, 1.0/7.0, 10.0/16.0, 0.0, 9.0/16.0, 0.0,
				  10.0/16.0, 1.0/7.0, 11.0/16.0, 1.0/7.0, 11.0/16.0, 0.0, 10.0/16.0, 0.0
      };
      */

      /*
      float texcoords[8 * 26] = {
				  1.0/16.0, 5.0/16.0, 2.0/16.0, 5.0/16.0, 2.0/16.0, 4.0/16.0, 1.0/16.0, 4.0/16.0,
				  2.0/16.0, 5.0/16.0, 3.0/16.0, 5.0/16.0, 3.0/16.0, 4.0/16.0, 2.0/16.0, 4.0/16.0,
				  3.0/16.0, 5.0/16.0, 4.0/16.0, 5.0/16.0, 4.0/16.0, 4.0/16.0, 3.0/16.0, 4.0/16.0,
				  4.0/16.0, 5.0/16.0, 5.0/16.0, 5.0/16.0, 5.0/16.0, 4.0/16.0, 4.0/16.0, 4.0/16.0,
				  5.0/16.0, 5.0/16.0, 6.0/16.0, 5.0/16.0, 6.0/16.0, 4.0/16.0, 5.0/16.0, 4.0/16.0,
				  6.0/16.0, 5.0/16.0, 7.0/16.0, 5.0/16.0, 7.0/16.0, 4.0/16.0, 6.0/16.0, 4.0/16.0,
				  7.0/16.0, 5.0/16.0, 8.0/16.0, 5.0/16.0, 8.0/16.0, 4.0/16.0, 7.0/16.0, 4.0/16.0,
				  //8.0/16.0, 5.0/16.0, 9.0/16.0, 5.0/16.0, 9.0/16.0, 4.0/16.0, 8.0/16.0, 4.0/16.0,
				  9.0/16.0, 5.0/16.0, 8.0/16.0, 5.0/16.0, 8.0/16.0, 4.0/16.0, 9.0/16.0, 4.0/16.0,
				  9.0/16.0, 5.0/16.0, 10.0/16.0, 5.0/16.0, 10.0/16.0, 4.0/16.0, 9.0/16.0, 4.0/16.0,
				  10.0/16.0, 5.0/16.0, 11.0/16.0, 5.0/16.0, 11.0/16.0, 4.0/16.0, 10.0/16.0, 4.0/16.0,
				  11.0/16.0, 5.0/16.0, 12.0/16.0, 5.0/16.0, 12.0/16.0, 4.0/16.0, 11.0/16.0, 4.0/16.0,
				  12.0/16.0, 5.0/16.0, 13.0/16.0, 5.0/16.0, 13.0/16.0, 4.0/16.0, 12.0/16.0, 4.0/16.0,
				  13.0/16.0, 5.0/16.0, 14.0/16.0, 5.0/16.0, 14.0/16.0, 4.0/16.0, 13.0/16.0, 4.0/16.0,
				  14.0/16.0, 5.0/16.0, 15.0/16.0, 5.0/16.0, 15.0/16.0, 4.0/16.0, 14.0/16.0, 4.0/16.0,
				  15.0/16.0, 5.0/16.0, 16.0/16.0, 5.0/16.0, 16.0/16.0, 4.0/16.0, 15.0/16.0, 4.0/16.0,
				  0.0/16.0, 6.0/16.0, 1.0/16.0, 6.0/16.0, 1.0/16.0, 5.0/16.0, 0.0/16.0, 5.0/16.0,
				  1.0/16.0, 6.0/16.0, 2.0/16.0, 6.0/16.0, 2.0/16.0, 5.0/16.0, 1.0/16.0, 5.0/16.0,
				  2.0/16.0, 6.0/16.0, 3.0/16.0, 6.0/16.0, 3.0/16.0, 5.0/16.0, 2.0/16.0, 5.0/16.0,
				  3.0/16.0, 6.0/16.0, 4.0/16.0, 6.0/16.0, 4.0/16.0, 5.0/16.0, 3.0/16.0, 5.0/16.0,
				  4.0/16.0, 6.0/16.0, 5.0/16.0, 6.0/16.0, 5.0/16.0, 5.0/16.0, 4.0/16.0, 5.0/16.0,
				  5.0/16.0, 6.0/16.0, 6.0/16.0, 6.0/16.0, 6.0/16.0, 5.0/16.0, 5.0/16.0, 5.0/16.0,
				  6.0/16.0, 6.0/16.0, 7.0/16.0, 6.0/16.0, 7.0/16.0, 5.0/16.0, 6.0/16.0, 5.0/16.0,
				  7.0/16.0, 6.0/16.0, 8.0/16.0, 6.0/16.0, 8.0/16.0, 5.0/16.0, 7.0/16.0, 5.0/16.0,
				  8.0/16.0, 6.0/16.0, 9.0/16.0, 6.0/16.0, 9.0/16.0, 5.0/16.0, 8.0/16.0, 5.0/16.0,
				  9.0/16.0, 6.0/16.0, 10.0/16.0, 6.0/16.0, 10.0/16.0, 5.0/16.0, 9.0/16.0, 5.0/16.0	
      };
      */


      //----------------
	// texture test
	// this one works!
	//glClearColor(0.0,0.0,1.0,0); 
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
      // most recent vbo test
      /*
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      //gluOrtho2D(0, 1024, 0, 768);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
	glClearColor(0.0,0.0,1.0,0); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	unsigned int vaoID[1]; // Our Vertex Array Object  
	unsigned int vboID[2]; // Our Vertex Buffer Object  
	float* vertices = new float[12];  // Vertices for our square  
	float* texcoords = new float[8];  // texture coordinates for our square  
	vertices[0] = 0; vertices[1] = 0; vertices[2] = 0;
	vertices[3] = 0; vertices[4] = 1; vertices[5] = 0;
	vertices[6] = 1; vertices[7] = 1; vertices[8] = 0;
	vertices[9] = 1; vertices[10] = 0; vertices[11] = 0; 
	texcoords[0] = 0; texcoords[1] = 0;
	texcoords[2] = 1; texcoords[3] = 0;
	texcoords[4] = 1; texcoords[5] = 1;
	texcoords[6] = 0; texcoords[7] = 1;

	glGenVertexArrays(1, &vaoID[0]); // Create our Vertex Array Object  
	glBindVertexArray(vaoID[0]); // Bind our Vertex Array Object so we can use it  
  	glGenBuffers(2, &vboID[0]); // Generate our Vertex Buffer Object  

	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]); // Bind our Vertex Buffer Object  
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertices, GL_STATIC_DRAW); // Set the size and data of our VBO and set it to STATIC_DRAW  
  	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); // Set up our vertex attributes pointer  
  	glEnableVertexAttribArray(0); // Disable our Vertex Array Object  
	glBindVertexArray(0); // Disable our Vertex Buffer Object   	

	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]); // Bind our second Vertex Buffer Object  
      	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texcoords, GL_STATIC_DRAW); // Set the size and data of our VBO and set it to STATIC_DRAW  
      	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0); // Set up our vertex attributes pointer  
      	glEnableVertexAttribArray(1); // Enable the second vertex attribute array  
	delete [] vertices; // Delete our vertices from memory 
	delete [] texcoords;
	glBindTexture(GL_TEXTURE_2D, 13452);
	glBindVertexArray(vaoID[0]); // Bind our Vertex Array Object  
      	glDrawArrays(GL_QUADS, 0, 4); // Draw our square  
      	glBindVertexArray(0); // Unbind our Vertex Array Object  
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      */

      // REPLACE ME -- DONE
      //display.flip();
      w.flushGL();
      w.flushDisplay();

      int retval;
        
      Slow.Lock();
	
	
	
      if(blankThisIter==true){}
      else if(multidisplay)
        {
	  printf("sending multidisplay file\r\n");
	  // REPLACE ME -- DONE
	  //display.sendMultidisplayFile(map,"/home/grfx/app_data/state.txt");
	  mesh.LoadMultidisplay("/home/grfx/app_data/state.txt", sides, texture_changed);
	  parseMultidisplayFile(map, "/home/grfx/app_data/state.txt");

	  printf("after sending multidisplay file\r\n");
        }//endelse

	

      else if(!volume_texture_enabled&&!red_green_walls&&!pong&&!pen_demo&&!space_invaders||(score_changed&&pong))    
        {


          FILE* filep;
          bool first_time_in_loop=true;
        		
          std::ifstream fin(OBJDIR "images/counter.txt");
	  assert(fin.is_open());
	  int val;
	  fin >> val;
	  fin.close();
          if(val!=curr_texture)
	    {
	      for (unsigned i=0; i<map.getNumTextures(); i++)	
          	{
#ifdef COMPRESSED_TEX
		  // REPLACE ME -- DONE
          	  //display.loadTexture(i, map.getTextureFilename(i).c_str(), renderers);
		  newLoadTexture(i, map.getTextureFilename(i).c_str());
#else
		  // REPLACE ME -- DONE
          	  //display.updateTexture(i, map.getTextureFilename(i).c_str());
		  // doesn't seem to ever be used. compressed texts are where it's AT
#endif
          	}//endfor
	    }//endif
          first_time_in_loop=false;
        

        }//end elseif

	

        if(fopen("/ramdisk/toggle_color_weights", "r")){
          remove("/ramdisk/toggle_color_weights");
          // REPLACE ME -- DONE
	  //display.toggleColorWeights();
	  ToggleColorWeights();
        }//end if
      Slow.Unlock();

      //Timing stuff
      gettimeofday(&endtime, NULL);
      cnt++;
      printf("fps %f count %d\r\n", (float)cnt/(endtime.tv_sec+endtime.tv_usec/
						1000000.0-starttime.tv_sec-starttime.tv_usec/1000000.0),cnt);
     
    }//end while1

  }


  // this loads a texture from a file instead of from a socket
  void newLoadTexture(int texture_id, const char *filename) {

    char file[1024*1024*4];
    FILE* filep;

    (*ARGS->output) << "opening texture file:" << filename << "\r" << std::endl;
    if(filep=fopen(filename,"r")) {
      
      fclose(filep);
	
      Image<sRGB> image(filename);
      assert(GL_NO_ERROR==glGetError());
	
      GLuint texture_idx = texture_id;

      bool first=false;
      assert(GL_NO_ERROR==glGetError());
   
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
	}

      glBindTexture(GL_TEXTURE_2D, textures.at(texture_idx));
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  

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
  }

  // needed by run()
  // transplanted verbatim from RemoteDisplay.h
  void parseMtl(TextureMapMap &map,const char *filename){
    FILE *fp = fopen(filename, "r");
    assert(fp!=NULL);
    char name[LINE_SIZE];
    char data[LINE_SIZE];
    char textline[LINE_SIZE];
    int val;
    while (!feof(fp)){
      fgets(textline, LINE_SIZE, fp);
      if (1 == sscanf(textline, "newmtl %1024s ", name));
      else if(1 == sscanf(textline, "map_Ka  %s", data)){
	//printf("found material in mtl : %s \r\n", name);
        val=map.getTextureID(name);
        if(val==-1)
          map.addToMap(name,data);
      }  
    }
    fclose(fp);
  }

  // also needed by run()
  // transplanted verbatim from RemoteDisplay.h
  void parseMultidisplayFile(TextureMapMap &map,const char *filename){
    FILE *fp = fopen(filename, "r");
    assert(fp!=NULL);
    char wall_name[LINE_SIZE];
    char file_name[LINE_SIZE];
    char textline[LINE_SIZE];
    int val;
    while (!feof(fp)){
      fgets(textline, LINE_SIZE, fp);
      assert (2 == sscanf(textline, "%1024s %1024s", wall_name, file_name));
      printf("Got %s %s\r\n",wall_name,file_name);
      for(int i=0;i<=4;i++){
        val=map.getTextureID(wall_name);
        if(val==-1){
          map.addToMultiMap(wall_name,file_name);
        }
      }
    }
    fclose(fp);
  }



  void ParseCommandStream(SocketReader & sr, const LI::Window& w);


  void Blank();


  void Render();

  void Flip() {}

  void DisplayImage(Image<sRGB> &image){
    printf("displaying image\r\n");
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
    //printf("begin load puzzle\r\n");

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
  MPI_Group group;
  MPI_Comm   comm;

  static RemoteProjector *the_instance;
  int height, width;
  OpenGLProjector gl_projector;
  std::vector<OpenGLProjector> gl_projectors;
  Mesh mesh;
  std::vector<int> textures;
  VolumetricData volume_data;
  bool volume_texture_enable;

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

    //printf("drawing stuff for pens\r\n");
    printf("target at %f %f %f %f\r\n", targetCenter.x(), targetCenter.y(), targetCenter.z(), targetRadius);
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
    //    listBuilt = false;
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

//RemoteProjector * RemoteProjector::instance;

#endif
