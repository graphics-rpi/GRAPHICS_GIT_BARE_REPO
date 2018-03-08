#include <cassert>
#include <string>
#include <vector>
#include <sstream>

#define MAX_BUFFER_SIZE 1024

// =============================================================================

std::vector<std::string> split_command_into_vector(const std::string &command) {
  std::string s;
  std::vector<std::string> command_vector;
  std::stringstream ss(command);
  while (ss >> s) {
    command_vector.push_back(s);
  }
  return command_vector;
}

std::string remove_path(std::string prog_name) {
  int pos = prog_name.find_last_of('/');
  if (pos == -1) return prog_name;
  assert (pos > 0 && pos < (int)prog_name.size());
  return  prog_name.substr(pos+1,prog_name.size()-pos-1);
}

void split_command(const std::string &command, std::string &directory, char *&path, int &num_args, char **&argv) { 
 
  std::cout << "split_command '" << command << "'" << std::endl;

  // first separate the command at the spaces
  std::vector<std::string> command_vector = split_command_into_vector(command);
  num_args = command_vector.size();
  assert (num_args >= 2);

  directory = command_vector[0];
  // path is first entry
  path = new char[command_vector[1].size()+1];
  strcpy(path,command_vector[1].c_str());
  
  // create the argv vector
  argv = new char*[command_vector.size()+1-1];
  // note: remove path from first entry (the program name)
  std::string prog_name = remove_path(command_vector[1]);
  //int pos = prog_name.find_last_of('/');
  //assert (pos > 0 && pos < (int)prog_name.size());
  //prog_name = prog_name.substr(pos+1,prog_name.size()-pos-1);

  argv[0] = new char[prog_name.size()+1];
  strcpy(argv[0],prog_name.c_str());
  // the rest of the arguments
  for (unsigned int i = 2; i < command_vector.size(); i++) {
    argv[i-1] = new char[command_vector[i].size()+1];
    strcpy(argv[i-1],command_vector[i].c_str());
  }


  // must terminate in NULL
  argv[num_args-1] = (char*)0;
}

// =============================================================================
// run the command in a separate process, return the process id

pid_t commandify(std::string command, bool is_master) {
  
  // remove trailing newline 
  if (command[command.size()-1] == '\n')
    command = command.substr(0,command.size()-1); 


  // quit the program 
  if (command == "exit" || command == "quit" || command == "q") {
    std::cout << "RECEIVED EXIT" << std::endl;
    return 0;
  }        

  std::string master = "master";
  if (!is_master) master = "not_master";
  
  // add tiling info to the commandline
  std::stringstream ss;
  ss << command << " " << "-tiled_display " 
     << master << " "
     << FULLSCREEN_WIDTH << " " 
     << FULLSCREEN_HEIGHT << " " 
     << MY_WIDTH << " " 
     << MY_HEIGHT << " " 
     << MY_LEFT << " " 
     << MY_BOTTOM; 

  // fork the current process, child will run the command
  pid_t childPID = vfork();
  if (childPID >= 0) {
    if (childPID == 0) {
      //child

      // prepare command line to call exec
      std::vector<std::string> command_vector;
      char* path = NULL;
      char** argv = NULL;
      int num_args = -1;
      std::string directory;
      split_command(ss.str(),directory,path,num_args,argv);

      // launch the process

      //std::cout << "HERE" << std::endl;
      if (chdir(directory.c_str()) != 0) //"/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/synenv/visualizations/MapView") != 0) 
	{ std::cout << "ERROR changing directory" << directory << std::endl; exit(0); }

      execv(path,argv); 


      
      //execl("/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/synenv/visualizations/MapView/bin/mapview", "mapview", "--fullscreen", (char*)0);
      //execl("/bin/ls", "ls", "-1", (char*)0);
      std::cout << "HERE2" << std::endl;

      // cleanup (this won't get called if thread was successful!)
      delete path;
      for (int i = 0; i < num_args; i++) {
	delete [] argv[i];
      }
      delete [] argv;

      std::cout << "ERROR TRYING TO RUN COMMAND: '" << ss.str() << "'" << std:: endl;
      exit(0);

    } else {
      // parent
    }
  } else {
    std::cout << "ERROR: FORK FAILED" << std::endl; exit(0);
  }

  return childPID;
}
