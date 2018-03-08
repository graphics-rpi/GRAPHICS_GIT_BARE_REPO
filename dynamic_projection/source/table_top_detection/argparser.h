#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>
#include <fstream>
#include "../common/MersenneTwister.h"


class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ~ArgParser() {
    if (output != &std::cout)
      delete output;
  }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
  
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == "-input" ||
	  std::string(argv[i]) == "-i") {
	i++;
	assert (i < argc);
	input_file = argv[i];
      } else if (std::string(argv[i]) == "-output" ||
		 std::string(argv[i]) == "-o") {
	i++;
	assert (i < argc);
	output_file = argv[i];
      } else if (std::string(argv[i]) == "-find_army") {
	find_army_terrain = true;
	find_army_soldiers = true;
      } else if (std::string(argv[i]) == "-find_army_terrain") {
	find_army_terrain = true;
      } else if (std::string(argv[i]) == "-find_army_soldiers") {
	find_army_soldiers = true;
      } else if (std::string(argv[i]) == "-find_architectural_design" ||
		 std::string(argv[i]) == "-find_arch") {
	find_architectural_design = true;
      } else if (std::string(argv[i]) == "-debug") {
	write_debug_images = true;
      } else if (std::string(argv[i]) == "-debug_path") {
	i++;
	assert (i < argc);
	debug_path = argv[i];
      } else if (std::string(argv[i]) == "-message_file") {
	i++;
	assert (i < argc);
	message_file = argv[i];
      } else if (std::string(argv[i]) == "-continuous") {
	continuous = true;
      } else if (std::string(argv[i]) == "-num_frames") {
	i++;
	assert(i < argc);
	num_frames = atoi(argv[i]);
	assert (num_frames > 0);
	assert (continuous == true);
      } else if (std::string(argv[i]) == "-quiet") {
	verbose = false;
      } else if (std::string(argv[i]) == "-verbose") {
	verbose = true;
      } else {
	std::cerr << "UNKNOWN COMMAND LINE ARGUMENT: " << argv[i] << std::endl;
	usage(argc,argv); 
	exit(0);
      }
    }
    
    if (input_file == "" ||
	output_file == "" ||
	!(find_army_terrain || find_army_soldiers ||
	  find_architectural_design)) {
      usage(argc,argv); 
    }
    if (verbose) {
      output = &std::cout;
    } else {
      output = new std::ofstream("/dev/null");
    }
  }

  

  void DefaultValues() {
    
    find_army_terrain = false;
    find_army_soldiers = false; 
    find_architectural_design = false; 

    write_debug_images = false;
    continuous = false;
    num_frames = -1;
    debug_path = "./debug_images";

    message_file = "./table_top_detect_message_file";

    verbose = true;
  }

  void usage (int argc, char ** argv) {
    std::cerr << "USAGE: " << argv[0] << std::endl;
    std::cerr << "   -input <input_image_file>" << std::endl;
    std::cerr << "   -output <output_wall_file>" << std::endl;
    std::cerr << "   [ -find_architectural_design ]" << std::endl;
    std::cerr << "   [ -find_army ]" << std::endl;
    std::cerr << "   [ -find_army_terrain ]" << std::endl;
    std::cerr << "   [ -find_soldiers ]" << std::endl;
    std::cerr << "   [ -debug ]" << std::endl;
    std::cerr << "   [ -debug_path <directory_for_debug_images> ]" << std::endl;
    std::cerr << "   [ -message_file <message_filename> ]" << std::endl;
    exit(0);
  }

  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  MTRand mtrand;


  std::string input_file;
  std::string output_file;
  bool find_army_terrain;
  bool find_army_soldiers;
  bool find_architectural_design;


  bool write_debug_images;
  bool continuous;
  int num_frames;
  std::string debug_path;

  bool verbose;
  std::ostream *output;


  std::string message_file;

  void RemoveMessageFile() {
    std::string tmp = "rm -f " + message_file;
    int ret = system (tmp.c_str());
    if (ret != 0) {
      std::cout << "UNUSUAL BEHAVIOR? " << std::endl;
      throw -1;
    }
  }

  void WriteMessageFile(const std::string &s) {

    { /* scope to ensure that the file is closed */
      std::ifstream istr(message_file.c_str());
      if (istr) {
	std::cerr << "ERROR!  MESSAGE FILE ALREADY EXISTS!" << std::endl;
	exit(0);
      }
    } /* scope to ensure that the file is closed */

    std::ofstream ostr(message_file.c_str());
    if (!ostr) {
      std::cerr << "ERROR!  COULD NOT OPEN ERROR MESSAGE FILE" << std::endl;
      exit(0);
    } 
    
    ostr << s;
  }



};


// ========================================================

#endif
