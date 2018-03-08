#ifndef INTERACTIONLOGGER_H_
#define INTERACTIONLOGGER_H_

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cassert>

class InteractionLogger{

  //message functions
  public:
    static void writeMessage(std::string msg);
    static void writeMessage(char *msg);
    static void writeWarning(std::string msg);
    static void writeError(std::string msg);

  //modifier functions
  public:
    static void setup();
  private:
    static std::ofstream ostr;

};

#endif
