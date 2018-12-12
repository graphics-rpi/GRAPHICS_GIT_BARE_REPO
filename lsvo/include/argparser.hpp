#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stackdump.h"
#include "wallfile_parser.hpp"
#include <vector>

#ifndef MPI_FLAG
#ifdef __APPLE__
//#include <glui.h>
#else
#include <GL/glui.h>
#endif
#else
#include "mpifake.h"
#endif
#include "state.hpp"



class LSVOArgParser {

public:

  LSVOArgParser() { DefaultValues(); }

  LSVOArgParser(int argc, char *argv[], std::vector<std::string>& remeshparams, State& state) {
    DefaultValues();
    bool remeshDone=false;

    for (int i = 0; i < argc; i++)
    {
      if(!remeshDone)
      {
         if (!strcmp(argv[i],"-remeshend"))
            remeshDone=true;
         else if(!strcmp(argv[i],"-i"))
         {
            remeshparams.push_back(std::string(argv[i]));
            printf("remesh param %s \n", argv[i++]);
            remeshparams.push_back(std::string(argv[i]));
            state.inputfile=std::string(argv[i]);
         }
         else
         {
            remeshparams.push_back(std::string(argv[i]));
            printf("remesh param %s \n", argv[i]);
         }
      }
      else
      {
        printf("non-remesh param %s \n", argv[i]);
        if (!strcmp(argv[i],"-t")) {
        i++;
        state.hour=atoi(argv[i++]);
        state.minute=atoi(argv[i]);
//        materials_file = std::string(argv[i]);
       printf("time %d:%d\n", state.hour, state.minute);
       }
        if (!strcmp(argv[i],"-date")) {
        i++;
        state.month=atoi(argv[i++]);
        state.day=atoi(argv[i]);
//        materials_file = std::string(argv[i]);
       printf("date %d/%d\n", state.month, state.day);
       }
       else if (!strcmp(argv[i],"-noqt"))
            state.useQT=false;
       else if (!strcmp(argv[i],"-ortho"))
       {
          i++;
          state.useOrthoCamera=true;
          state.orthoFiles.push_back(std::string(argv[i]));
       }
       else if (!strcmp(argv[i],"-dumpOrthos")) {
        i++;
        state.useOrthoCamera=true;
        int numImages=atoi(argv[i]);
        for(int j=0; j< numImages; j++)
        {
          state.orthoFiles.push_back(std::string(argv[++i]));
          //sleep(1000);
        }

       }
      else if (!strcmp(argv[i],"-dumpPeople")) {
        i++;
//        state.dumpPeople=true;
        WallfileParser wp;
        state.people=wp.getPersonVector(argv[i]);
        //if (state.people.size()>0)
          state.dumpPeople=true;
        assert(state.useOrthoCamera&&"Must be using ortho cameras to dump people");
       }
       else if (!strcmp(argv[i],"-exp")) {
        i++;
        state.exposurePercent=atoi(argv[i]);
        printf("Exposure %d \n", state.exposurePercent);
       }
       else if (!strcmp(argv[i],"-viewAngle")) {
        i++;
        state.viewAngle=atoi(argv[i]);
        printf("View angle %d \n", state.viewAngle);
       }
        else if (!strcmp(argv[i],"-o")) {
        i++;
        state.outputfile_specified=true;
        state.outputfile=std::string(argv[i]);
        printf("output file %s \n", state.outputfile.c_str());
       }
       else if (!strcmp(argv[i],"-dumpImage")) {
        i++;
        state.dumpImage=true;

        state.imageFile=std::string(argv[i]);

       }
        else if (!strcmp(argv[i],"-dumpPatches")) {
        i++;
        state.dumpPatches=true;
        state.patchesFile=std::string(argv[i]);

       }
        else if (!strcmp(argv[i],"-dumpTris")) {
        i++;
        state.dumpTris=true;
        state.trisFile=std::string(argv[i]);

       }
        else if (!strcmp(argv[i],"-greyscale")) {
        state.greyscale=1;

       }
       else if (!strcmp(argv[i],"-screen")) {
        i++;
        state.screen=1;
        state.screenSpecified=true;
        state.screenFile=std::string(argv[i]);
        printf("screem file %s \n", state.screenFile.c_str());
       }
				else if (!strcmp(argv[i],"-weather")) {

					i++; //next

					if( !strcmp(argv[i], "CLEAR" )){
						state.skyType=0;
						printf("Skytype CLEAR\n");
					}

					if( !strcmp(argv[i], "TURBID" )){
						state.skyType=1;\
						printf("Skytype TURBID\n");
					}

					if( !strcmp(argv[i], "INTERMEDIATE" )){
						state.skyType=2;
						printf("Skytype INTERMEDIATE\n");
					}

					if( !strcmp(argv[i], "OVERCAST" )){
						state.skyType=3;
						printf("Skytype OVERCAST\n");
					}

				}

       else if (!strcmp(argv[i],"-toodim")) {
        ++i;
        state.toodim=atof(argv[i]);
       }
       else if (!strcmp(argv[i],"-toobright")) {
        ++i;
        state.toobright=atof(argv[i]);
       }
        else if (!strcmp(argv[i],"-moments")) {
        state.moment=true;
        }
        else if(!strcmp(argv[i],"-coordinate")){
          // Expect the latitude and longitude
          ++i; state.latitude=atof(argv[i]);
          ++i; state.longitude=atof(argv[i]);
          printf("latitude and longitude (%f,%f)\n", state.latitude, state.longitude);
        }
      }
      // filenames
//      if (!strcmp(argv[i],"-input")) {
//	i++; assert_stackdump (i < argc);
//	load_file = std::string(argv[i]);
//      } else if (!strcmp(argv[i],"-materials")) {
//        i++; assert_stackdump (i < argc);
//        materials_file = std::string(argv[i]);
//      } else if (!strcmp(argv[i],"-output")) {
//	i++; assert_stackdump (i < argc);
//	save_file = std::string(argv[i]);
    }

  }


  void DefaultValues() {


  }





};


#endif
