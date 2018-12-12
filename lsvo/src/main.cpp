#include "qtApp.hpp"
#include<SimpleScene.hpp>
#include "GLUTDisplay.hpp"
#include "remesher.h"
#include "mesh.h"
#include <iostream>
//#include "test.hpp"
#ifdef _WIN32
#include <process.h>
#endif

#include "argparser.hpp"
using namespace std;
//void callRemesher() {}

#ifdef _WIN32
void qtThread(void* ptr)
#else
void *qtThread(void* ptr)
#endif
{
 State& state= *((State*)ptr);

  QTApp app(state.argc,state.argv, state);
  return 0;

}

int main(int argc, char** argv) {
  State state(argc,argv);
  state.argc=argc;
  state.argv=argv;


  std::vector<std::string> command_line_args;
    LSVOArgParser argparser= LSVOArgParser(argc, argv, command_line_args, state);

  cout<<"args"<<endl;
  for(unsigned int i=0; i<command_line_args.size();i++)
	printf("arg %d : %s\n",i,command_line_args[i].c_str());


  const Mesh* tmp_mesh = remesher_main(command_line_args,true);
  cout<<"Num vertices: "<< tmp_mesh->numVertices() << endl;
  cout<<"Num tris: "<< tmp_mesh->numTriangles() << endl;
  if(state.screenSpecified)
    state.screen=1;
  if(state.useQT==true)
  {

#ifdef _WIN32
      _beginthread(qtThread, 0, &state);
#else
      pthread_t thread1;
      int pthreadReturn;
      pthreadReturn=pthread_create(&thread1, NULL, qtThread, &state);
#endif
  //QTApp app(argc, argv, state);
    while(state.changed==1)
    {
        sleep(.1);
    }
  }


  GLUTDisplay::init(argc, argv);

  try {
    bool done;
    printf("before scene constructor\n");
    SimpleScene scene(tmp_mesh,state, 0,done);
    printf("after scene constructor\n");
    GLUTDisplay::run("Window Title", &scene, GLUTDisplay::CDAnimated);
  } catch(Exception& e) {
    sutilReportError(e.getErrorString().c_str());
		printf("error1\n");
    return 1;
  }

  return 0;
}

//  command_line_args.push_back(std::string(argv[0]));
//  command_line_args.push_back(std::string("-i"));

//command_line_args.push_back(std::string("/home/nasmaj/GIT_CHECKOUT/remesher/examples/user_study_3_wallfiles/groundtruth.wall"));
//  command_line_args.push_back(std::string("-offline"));
//  command_line_args.push_back(std::string("-t"));
//  command_line_args.push_back(std::string("3000"));
//  command_line_args.push_back(std::string("-patches"));
//  command_line_args.push_back(std::string("500"));
