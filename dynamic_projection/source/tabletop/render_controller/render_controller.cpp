#include "render_controller.h"
//#define TABLE_GLCAMS
//#define DEBUG


#define GL_GLEXT_PROTOTYPES
#define GL_API


#include <netdb.h>
#include <sys/time.h>
#include <GL/gl.h>
#include "RemoteDisplay.h"
#include <Image.h>
#include "displayParser.h"
#include "sockets.h"
#include "DirLock.hpp"
#include "TextureMapMap.hpp"
#include "RRController.h"


#include "../argparser.h"
extern ArgParser *ARGS;

#define OBJDIR "/ramdisk/"



void RenderController::run() {

  // will this work????????
  std::ios_base::sync_with_stdio(true);
  
  ConfigParser parser(FILE_LOCATION);
  //printf("in run\r\n");
  //Keeps track of the number of acknowledgment messages received.
  bool score_changed=true;
  //The class which passes remote monitors messages
  RemoteDisplay display;
  disp = &display;
  //      atexit(cleanup);
  
  //This is where the remote renderers are opened on other machines
  //sockets are opened here as well
  
  RRController* renderers [parser.displays.size()];
  for(int i=0; i<parser.displays.size(); i++){
    //printf("adding display %i\r\n",i);
    renderers[i]=new RRController(i, parser);
    display.addDisplay(renderers[i]);
  }


  char buffer[50];
  
  //This is the structure which keeps track of the correspondency between images and textures
  TextureMapMap map;
  int cnt=0;
  
  //These are the two locks used
  
  DirLock PhysicalModel(OBJDIR "tween");
  DirLock SimulationModel(OBJDIR "images");
  
  //Both locks must be locked for the mtl file as reading through the 
  //  mtl file may result in checkin if images exist
  
  
  if (send_obj || send_mtl) {
    PhysicalModel.Lock();
    SimulationModel.Lock();
    if(send_mtl && !ARGS->multidisplay)
      display.loadMtl(map, OBJDIR "tween/foo.mtl");  
    
    printf("after mtl \r\n");
    SimulationModel.Unlock();
    printf("Before obj \r\n");
    if(send_obj){
      display.loadMesh(OBJDIR "tween/foo.obj");
    }
    
    PhysicalModel.Unlock();
    printf("end send objmtl\r\n");
  }

  if(ARGS->army){
    // hack so that obj and mtl are only sent initially
    send_obj = send_mtl = false;
  }
  
  SimulationModel.Lock();
  //Loads all the textures found in the mtl file
  if(!puzzle&&!pen_demo&&!space_invaders && !ARGS->army){
    unsigned int i;    
    for (i=0; i<map.getNumTextures(); i++){
      display.loadTexture(i, map.getTextureFilename(i).c_str(), renderers);
    }
    //display.loadTexture(i, "/ramdisk/testure.ppm", renderers);
    
  }
  
  if(ARGS->army){
    // only send the new floor texture
    display.loadTexture(0, map.getTextureFilename(0).c_str(), renderers);
  }

  SimulationModel.Unlock();
  //printf("after loading all texture \r\n");
  
  char str[LINE_SIZE];
  char c;
  //Loads all the glcam files (it is assumed they start lettered with 'A'
  //char projector_name[256];
  for(int i=0; i < parser.displays.size(); i++) {
    char projector_name[256];
    sprintf(projector_name,"%s",parser.displays[i].display_name.c_str());
    renderers[i]->setGLCam(projector_name);
    // add this glcam to the rest of the remote renderers
    // for the pixel blending
    // we'll get this eventually
    /*
      for (int j = 0; j < parser.displays.size(); j++) {
      if (i == j) continue;
      renderers[j]->addGLCam(projector_name);
    }
    */
  }
  
  
  
  //count, start time and end time are used to calculate fps
  int count=0;
  timeval starttime,endtime;
  gettimeofday(&starttime, NULL);
  
  int slow_index=0;
  
  int curr_texture=-1;
  
  bool blankThisIter=false;
  //The program runs until killed
  int loop_count=0;
  while(1){
    
    std::cout << "RENDER CONTROLLER RUN LOOP " << loop_count << "\r" << std::endl;

    if(false)//loop_count++==5)
    {
      display.quit();
      display.flush();
      printf("quitting rc \r\n");
      MPI_Finalize();
      printf("quitting rc 2 \r\n");
      exit(0);
    }
    // wrap in a VERBOSE flag
    //printf("in while %d\r\n", loop_count++);
    // this causes the super sleep
    
    // WHY IS IT TWO SECONDS?
    // OPTIMIZE
    //sleep(2);
    usleep (100000);

    blankThisIter=false;
    
    FILE *fp, *fp2;
    fp=fopen("/ramdisk/tween/stop","r");
    fp2=fopen("/ramdisk/tween/stop2","r");
    while(fp||fp2) {
      blankThisIter=true;
      display.blank_and_take_image(ARGS->take_pic);
      //sleep(1);
      renderers[0]->get_ack(); 
      if(fp)   
	fclose(fp); //do nothing
      else
	fclose(fp2); //do nothing
      if( remove( "/ramdisk/tween/stop" ) != 0 ) {
	printf( "Error deleting file" );
	//assert(0);
      }
      fp=fopen("/ramdisk/tween/stop","r");
      fp2=fopen("/ramdisk/tween/stop2","r");
    }//end while
    
    
    slow_index++;
    if(slow_index==map.getNumTextures()){
      slow_index=0;
    }//endif
    
    //Don't update geometry if it's play a day
    FILE* filep;
    filep=fopen("/home/grfx/Checkout/archdisplay/play.day", "r");
    if(filep==NULL) {
      //Hack to make frame counter works
      gettimeofday(&starttime, NULL);
      cnt=0;
      //The obj and mtl files are loaded each iteration
      PhysicalModel.Lock();

      
      // need to lock /ramdisk/images to check whether the images exist
      if(ARGS->multidisplay) {
	printf("sending multidisplay file\r\n");
	display.sendMultidisplayFile(map,"/home/grfx/app_data/state.txt");
	printf("after sending multidisplay file\r\n");
      }//end if multi
      else
	{	
	  SimulationModel.Lock();    
	  
	  display.loadMtl(map, OBJDIR "tween/foo.mtl");
	  
	  SimulationModel.Unlock();    
	}//endelse
      
      
      display.loadMesh(OBJDIR "tween/foo.obj");
      
      //printf("sending ble\r\n");
      //if(blending)
      //  display.loadBle(OBJDIR "tween/foo.ble"); 
      
      
      PhysicalModel.Unlock();
      
    }//end if filep
    else{
      
      fclose(filep);
    }

    // disable blending if we are told to do so
    if(ARGS->blending){
      display.loadBle(OBJDIR "tween/foo.ble"); 
    }
    else {
      display.disableBlending();
    }

    PhysicalModel.Unlock();
    if(blankThisIter!=true)
    {
      display.render();
      //printf("rendering!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
    }
    
    //Renderer 0 must always send an ack so that the sockets don't get too full
    if(blankThisIter!=true)
      renderers[0]->get_ack();    
    
    display.flip();
    MPI_Barrier(MPI_COMM_WORLD);
    
    int retval;
    
    SimulationModel.Lock();
    
    if(blankThisIter==true){}
    else if(ARGS->multidisplay)
      {
	printf("sending multidisplay file\r\n");
	display.sendMultidisplayFile(map,"/home/grfx/app_data/state.txt");
	printf("after sending multidisplay file\r\n");
      }//endelse
    
    else if(!volume_texture_enabled&&!red_green_walls&&!pong&&!pen_demo&&!space_invaders||(score_changed&&pong))    
      {
	
	FILE* filep;
	bool first_time_in_loop=true;
        
	std::ifstream fin("/ramdisk/images/counter.txt");
	assert(fin.is_open());
	int val;
	fin >> val;
	fin.close();
	
	// trying to figure out why it's not projecting
	//if(val!=curr_texture)  {
    int counter=0;
	if(1) {
    if(counter++==5);
    


	  val=curr_texture;
	  std::cout << "sending " << map.getNumTextures() << " textures... ";
	  fflush(stdout);
    unsigned int i;
	  for (i=0; i<map.getNumTextures(); i++)	
    {
#ifdef COMPRESSED_TEX
      display.loadTexture(i, map.getTextureFilename(i).c_str(), renderers);
      //Flush was here
      //display.flush();
#else
      display.updateTexture(i, map.getTextureFilename(i).c_str());
#endif
    }//endfor
    //We want flush here  
    display.flush();
	  //display.loadTexture(i, "/ramdisk/testure.ppm", renderers);
	  
	  std::cout << " done\r\n";
	}//endif
	first_time_in_loop=false;
        

      }//end elseif

    if(fopen("/ramdisk/toggle_color_weights", "r")){
      remove("/ramdisk/toggle_color_weights");
      display.toggleColorWeights();
    }//end if

    SimulationModel.Unlock();

    //Timing stuff
    gettimeofday(&endtime, NULL);
    cnt++;
#if 0
    printf("fps %f count %d\r\n", (float)cnt/(endtime.tv_sec+endtime.tv_usec/
					      1000000.0-starttime.tv_sec-starttime.tv_usec/1000000.0),cnt);
#endif     
  }//end while1
  display.quit();
}












