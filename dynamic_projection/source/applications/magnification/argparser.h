#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>
#include "../../calibration/planar_interpolation_calibration/tiled_display.h"

// This assumes you are running the program from the "source" directory, 
// change as necessary.
#define IMAGE_DIR "applications/magnification/images/"
#define SHADER_DIR "applications/magnification/shaders/"

class ArgParser {

    public:
        ArgParser() { DefaultValues(); }

        ArgParser(int argc, char *argv[]) {
            printf("ARG ONE\n");
            DefaultValues();
            printf("ARG TWO\n");


            for (int i = 1; i < argc; i++) {
                if (std::string(argv[i]) == std::string("--not_full_screen")) {
                    //tiled_display.full_screen = false;
                } 
                else if (std::string(argv[i]) == std::string("--no_lasers")) {
                    lasers_enabled_ = false; //tiled_display.full_screen = false;
                } 
                else if (std::string(argv[i]) == std::string("--image_filename") 
                        || std::string(argv[i]) == std::string("-i")) {
                    i++;
                    assert (i < argc);
                    image_filename_ = std::string(argv[i]);
                    image_filename_ = IMAGE_DIR + image_filename_;
                } 
                else if( std::string(argv[i]) == std::string("--vertex_shader")
                        || std::string(argv[i]) == std::string("-v")){
                    i++;
                    assert(i < argc);
                    vertex_filename_ = std::string(argv[i]);
                    vertex_filename_ = SHADER_DIR + vertex_filename_;

                }
                else if( std::string(argv[i]) == std::string("--fragment_shader")
                        || std::string(argv[i]) == std::string("-f")){
                    i++;
                    assert(i < argc);
                    fragment_filename_ = std::string(argv[i]);
                    fragment_filename_ = SHADER_DIR + fragment_filename_;

                }
                else if (std::string(argv[i]) == std::string("--tiled_display")) {
                    //tiled_display.read(i,argc,argv);
                } 
                else if (std::string(argv[i]) == std::string("--size")) {
                    //tiled_display.full_screen = false;        
                    i++;
                    assert (i < argc);	
                    width_ = atoi(argv[i]);
                    i++;
                    assert (i < argc);	
                    height_ = atoi(argv[i]);
                    //tiled_display.reshape(width,height);
                } 
                else if( std::string(argv[i]) == std::string("--num_cursors")){
                    i++;
                    assert (i < argc);
                    num_cursors_ = atoi(argv[i]);
                }
                else {
                    std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
                    assert (0);
                }
            }

            //tiled_display.print();
        }


        void DefaultValues() {
            image_filename_ = std::string(IMAGE_DIR) + "test.bmp";
            vertex_filename_ = std::string(SHADER_DIR) + "texture.vert";
            fragment_filename_ = std::string(SHADER_DIR) + "texture.frag";

            shader_filepath_ = std::string(SHADER_DIR);

            width_ = 1680;
            height_ = 1050;

            multimice_enabled_ = true;
            lasers_enabled_ = true;

            num_cursors_ = 4;
            
            movement_enabled_ = false;
            magnification_enabled_ = false;
            constant_motion_ = false;
        }

        // ==============
        // Representation
        // all public! (no accessors)

        std::string image_filename_;
        std::string vertex_filename_;
        std::string fragment_filename_;
        std::string shader_filepath_;


        bool movement_enabled_;
        bool multimice_enabled_;
        bool lasers_enabled_;

        int width_;
        int height_;

        int num_cursors_;

        bool magnification_enabled_;
        bool constant_motion_; 

        TiledDisplay tiled_display_;

};


// ========================================================

#endif
