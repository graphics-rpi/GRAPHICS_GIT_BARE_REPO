#include "InteractionLogger.h"


std::ofstream InteractionLogger::ostr;

void InteractionLogger::writeMessage(std::string msg){
  char buf[200];
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buf,100,"%H:%M:%S ",timeinfo);
  ostr<<buf<<msg<<'\n';
  ostr.flush();
}

void InteractionLogger::writeMessage(char *msg){
  char buf[200];
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buf,100,"%H:%M:%S ",timeinfo);
  ostr<<buf<<msg<<'\n';
  ostr.flush();
}

void InteractionLogger::setup(){
  char buf[200];
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buf,100,"./InteractionLogger/Session-%m.%d.%y-%H.%M.log",timeinfo);
  ostr.open(buf);

  assert(ostr.good());

  strftime(buf,100,"Program started %c",timeinfo);
  ostr<<buf<<'\n';
  ostr.flush();
}
