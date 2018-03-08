#ifndef RENDER_H
#define RENDER_H
#include "remote_render.h"



#include "../argparser.h"
extern ArgParser *ARGS;

struct Render {
  inline Render()
    : mContinue(true) {}

  inline void init(const LI::Window& w) 
  {
    w.bindContext();
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0, 0, 0, 0);
    char font_name[256];
    char* default_font =(char *) "fixed";
    strcpy(font_name, default_font);
  }

  inline void run(const LI::Window& w) 
  {

    std::cout << "In Render::Run()\r" << std::endl;

    int ranK;
    glClear(GL_COLOR_BUFFER_BIT);

    MPI_Comm_rank ( MPI_COMM_WORLD, &ranK );
    int swit=ranK/4;
    
//    MPI_Barrier(MPI_COMM_WORLD);
//    w.getAttributes().mWidth
    int num=55758+ranK*10;
    (*ARGS->output) << "about to open socket reader " << ranK << "\r" << std::endl;
    SocketReader sr(56758+ranK*10,ARGS->verbose);
    (*ARGS->output) << "before rp \r" << std::endl;
    RemoteProjector *projector = RemoteProjector::Instance(    w.getAttributes().mWidth, 
        w.getAttributes().mHeight);//wid,hei);
    (*ARGS->output) << "After rp \r" << std::endl;
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    (*ARGS->output) << "before pcs\r" << std::endl;
    projector->ParseCommandStream(sr,w);

    w.flushDisplay();
    sr.closef();
  }

  bool mContinue;
};

#endif
