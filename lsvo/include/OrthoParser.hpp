#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <assert.h>
using std::string;

#ifndef ORTHOPARSER
#define ORTHOPARSER

class OrthoParser
{
  public:
    OrthoParser(string filename)
    {
      bool dimset=false;
      bool nearPointsSet=false;
      bool cameraCenterSet=false;
      bool cameraDirSet=false;
      string line;
      std::ifstream myfile (filename.c_str());
      if (myfile.is_open())
      {
        getline (myfile,line);
        while ( myfile.good() )
        {

          //printf("line: %s \n", line.c_str());
          if(line [0]=='#')
          {
            if(line=="# width height")
            {
                getline (myfile,line);
                sscanf(line.c_str(),"%d %d",&width, &height);
                //printf("width is %i height is %i \n", width, height);
                dimset=true;          
            }
            else if(line=="# camera center")
            {
                getline (myfile,line);
                sscanf(line.c_str(), "%f %f %f", &cameraCenter.x, &cameraCenter.y, &cameraCenter.z);
                //printf("camera center is %f %f %f \n",cameraCenter.x, cameraCenter.y, cameraCenter.z);
                cameraCenterSet=true;
            }   
            else if(line=="# camera direction")
            {
                getline (myfile,line);
                sscanf(line.c_str(), "%f %f %f", &cameraDir.x, &cameraDir.y, &cameraDir.z);
                //printf("camera direction is %f %f %f \n",cameraDir.x, cameraDir.y, cameraDir.z);
                cameraDirSet=true;
            }
            else if(line=="# near plane points")
            {
                getline (myfile,line);
                sscanf(line.c_str(), "%f %f %f", &nearPoint1.x, &nearPoint1.y, &nearPoint1.z);
                //printf("np1 is %f %f %f \n",nearPoint1.x, nearPoint1.y, nearPoint1.z);

                getline (myfile,line);
                sscanf(line.c_str(), "%f %f %f", &nearPoint2.x, &nearPoint2.y, &nearPoint2.z);
                //printf("np2 is %f %f %f \n",nearPoint2.x, nearPoint2.y, nearPoint2.z);

                getline (myfile,line);  
                sscanf(line.c_str(), "%f %f %f", &nearPoint3.x, &nearPoint3.y, &nearPoint3.z);
                //printf("np3 is %f %f %f \n",nearPoint3.x, nearPoint3.y, nearPoint3.z);

                getline (myfile,line);   
                sscanf(line.c_str(), "%f %f %f", &nearPoint4.x, &nearPoint4.y, &nearPoint4.z);
                //printf("np4 is %f %f %f \n",nearPoint4.x, nearPoint4.y, nearPoint4.z);
                nearPointsSet=true;
            }    
            else if(line=="# projection matrix")
            {
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &projMatR1.x, &projMatR1.y, &projMatR1.z, &projMatR1.w);
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &projMatR2.x, &projMatR2.y, &projMatR2.z, &projMatR2.w);
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &projMatR3.x, &projMatR3.y, &projMatR3.z, &projMatR3.w);
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &projMatR4.x, &projMatR4.y, &projMatR4.z, &projMatR4.w);
            }
            else if(line=="# modelview matrix")                 
            {
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &modMatR1.x, &modMatR1.y, &modMatR1.z, &modMatR1.w);
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &modMatR2.x, &modMatR2.y, &modMatR2.z, &modMatR2.w);
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &modMatR3.x, &modMatR3.y, &modMatR3.z, &modMatR3.w);
                    getline (myfile,line);
                    sscanf(line.c_str(), "%f %f %f %f", &modMatR4.x, &modMatR4.y, &modMatR4.z, &modMatR4.w);
            }     
            else if(line=="# distance of near and far plane")
                getline (myfile,line);
                 
            else 
                printf("unrecognized command in line %s \n", line.c_str());
          }
          else
          {
            printf("unrecognized line %s \n", line.c_str());    
            assert(0);
          }
          getline (myfile,line);
        }
        myfile.close();
        
        if(!dimset)
        {
            printf("Dimensions not set! \n");
            assert(0);
        }
        if(!nearPointsSet)
        {
            printf("Near points not set! \n");
            assert(0);
        }
        if(!cameraCenterSet)
        {
            printf("Camera center not set! \n");
            assert(0);
        }
        if(!cameraDirSet)
        {
            printf("Camera direction not set! \n");
            assert(0);
        }
      }
      else
      {
        printf("failed to open file %s \r\n", filename.c_str());
        assert(0);
      }

    }
    
    //Member variables used to generate an orthogonal camera
    int width, height;
    float3 nearPoint1;
    float3 nearPoint2;
    float3 nearPoint3;
    float3 nearPoint4;
    float3 cameraCenter;
    float3 cameraDir;
    float4 projMatR1;
    float4 projMatR2;
    float4 projMatR3;
    float4 projMatR4;
    float4 modMatR1;
    float4 modMatR2;
    float4 modMatR3;
    float4 modMatR4;
     
};

#endif
