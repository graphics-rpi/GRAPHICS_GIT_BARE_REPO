// #define GL_GLEXT_PROTOTYPES
// #define GLX_GLXEXT_PROTOTYPES


#define GL_GLEXT_PROTOTYPES
#define GL_API
#define RESOLUTION_HACK

//Remove these 3?
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <mpi.h>


#include "remote_renderer/shader.h"
/*
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <GL/glxext.h>
*/

//#include "LiThreadDispatcher.hxx"
#include "LiWindow.hxx"
#include "SocketReader.h"

#include "glxfont.c"
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
  using namespace std;


#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <unistd.h>
#include <iostream>
#include <fstream>
//#include "remote_render.h"
#include "displayParser.h"
#include "render.h"
//namespace RC{
#include "render_controller.h"
//};
int curr_frame=4;

#include "argparser.h"
ArgParser *ARGS;


RemoteProjector * RemoteProjector::the_instance = NULL;


using std::cout;
using std::endl;
char *outstr;
bool tri=true;
int ranK;
int wid,hei;





Render GLOBAL_r;

//LI::ThreadDispatcher GLOBAL_dispatcher;
LI::XCxn cxn;
 
/*
void display(void)
{
  while(GLOBAL_r.mContinue) 
  {
      if(GLOBAL_dispatcher.getQueueSize() < 5) 
      {
        GLOBAL_dispatcher.prod();
      }
  }
}
*/

// argv[1] is the optional commandline argument for an alternate XServer
int main(int argc, char** argv) {
  
  ARGS = new ArgParser(argc,argv);
      

  //Initialize MPI
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &ranK);
  if(ranK<8)//If it is a remote renderer
  {
      ConfigParser parser(FILE_LOCATION);
      //Variables for the height and width of the two screens
      int w1,w2,h1,h2;
      //ConfigParser parser(FILE_LOCATION);
      int ranKOverTwo=ranK;
      int XServer, XScreen;
      XServer=1;
      XScreen=0;
      XServer=(parser.displays)[ranK].XServer;
      XScreen=(parser.displays)[ranK].XScreen;
      if(ranKOverTwo>2)ranKOverTwo-=3;
      
      ranKOverTwo=(ranKOverTwo+1)/2;

      //printf("XServer %d XScreen %d \r\n",XServer, XScreen);
      //getScreenDimensions(XServer, w1,  h1, w2, h2);

      //Makes all displays the same resolution.
    #ifdef RESOLUTION_HACK
      w1=1024;
      w2=1024;
      h1=768;
      h2=768;   
      //printf("overriding resolution\r\n");
    #endif

      if(XScreen==0)
      {
      	wid=w1;
        hei=h1;
      }
      else
      {
	      wid=w2;
	      hei=h2;
      }


      cxn.mServer=XServer;
	    cxn.mScreen=XScreen;
      
	    //printf("Server %i Screen %i openned\r\n",XServer,XScreen);

	    LI::WindowAttribs attribs(wid, hei);
      outstr=(char*)malloc(50);
      sprintf(outstr,"Machine %i Xserver %i head %i width %i height %i ",(int)ranK/3, cxn.mServer, cxn.mScreen, wid, hei);
      //printf("before %s\r\n",outstr);  
      LI::Window window("Here we go again...", cxn, attribs);
      
//      dispatcher.setInitCallback(boost::bind(&Render::init, &r, boost::ref(window)));
//      dispatcher.setRunCallback(boost::bind(&Render::run, &r, boost::ref(window), _1));

      std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\r\n";
      (*ARGS->output) << "HUH??\r" << std::endl;
      std::cout << "STARTING REMOTE RENDERER\r" << std::endl;

      (*ARGS->output) << "RR A\r" << std::endl;

      GLOBAL_r.init(window);
      (*ARGS->output) << "RR B\r" << std::endl;
      GLOBAL_r.run(window);  
      (*ARGS->output) << "RR C\r" << std::endl;
      //while(r.mContinue) 
      //{
      //    if(dispatcher.getQueueSize() < 5) {
      //        dispatcher.prod();
      //    } else {
      //        sched_yield();
      //    }
     // }

  }
  else //render controller
  {

    //printf("before rc#######################################################################3\r\n");
      RenderController rc;
      //printf("before process command line ########################################################\r\n");
      //processCommandLine(argc, argv, rc);
      //printf("after process c l##################################################################\r\n");
      std::cout << "################################\r\n";
      std::cout << "STARTING RENDER CONTROLLER\r" << std::endl;
      (*ARGS->output) << "HERE??\r" << std::endl;
      rc.run(); 
  }

  MPI_Finalize();
  return 0;
}

//void getScreenDimensions(int XServerNum, int &w1, int &h1, int & w2, int & h2);

/*
void getScreenDimensions(int XServerNum, int &w1, int &h1, int & w2, int & h2)
{
  char line[1024];
  int found=0;
  int int1, int2;
  char fileName[256];


  int retVal=sprintf(fileName,"/var/log/Xorg.%i.log",XServerNum);
  if(retVal<0)
  {
    cout<<"sprintf didn't work";
    exit(-1);
  }

  ifstream XOrgFile(fileName);
  if(XOrgFile.good())
    printf("file opened successfully\r\n");
  else
    printf("not so much\r\n");

  bool first=false;
  bool second=false;
  int cnt=0;

  //This code gets the resolution from the Xorg.x.log files.
  //If the driver is updated this code may break
  while(XOrgFile.getline(line,1024))
  {
    string str(line);
    
    found=str.find("Virtual");
    if(found>-1)
    {
        cout<<"count "<<++cnt<<endl;
        if(first==false)
        {
        	sscanf(line,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %*s %d", &w1, &h1);
          	first=true;
        }
        else if(second==false)
        {
		sscanf(line,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %*s %d", &w2, &h2);
        	second=true;
        }


    }
  }
  XOrgFile.close();
  if(first!=true&&second!=true)
    cout<<"error: both screens were not found "<<XServerNum<<endl;
  else printf("at least one found\r\nx");
   
 
}
*/
