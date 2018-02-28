#include "remesher.h"
#define GL_GLEXT_PROTOTYPES
int main(int argc, char *argv[]) {

    
  std::vector<std::string> command_line_args;
  for (int i = 0; i < argc; i++) {
    command_line_args.push_back(std::string(argv[i]));
  }

  // std::cout << "=====================================" << std::endl;
  //std::cout << "REMESHER begin!" << std::endl;
  remesher_main(command_line_args,true);
  //std::cout << "REMESHER finished!" << std::endl;
  //std::cout << "=====================================" << std::endl;
  //remesher_main(command_line_args,false);
}

