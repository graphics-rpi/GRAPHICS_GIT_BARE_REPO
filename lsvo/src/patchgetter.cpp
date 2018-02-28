#include "qtApp.hpp"

#include<SimpleScene.hpp>
#include <GLUTDisplay.hpp>
#include "remesher.h"
#include "mesh.h"
#include <iostream>
//#include "test.hpp"

#include "argparser.hpp"
#include "element.h"



struct SharedStuff{
bool done;
vector<float4>* patchVec;
int argc;
char** argv;
 const Mesh* tmp_mesh;
 State* state;
};

void *glutThread(void* ptr)
{

  SharedStuff* stuff = (SharedStuff*) ptr;



  GLUTDisplay::init(stuff->argc, stuff->argv);

  try {
    SimpleScene scene(stuff->tmp_mesh,*(stuff->state), 0, stuff->patchVec, stuff->done);
    GLUTDisplay::run("Window Title", &scene, GLUTDisplay::CDAnimated);
  } catch(Exception& e) {
    printf("wrong one\n");
    sutilReportError(e.getErrorString().c_str());
    //return 1;
  }

}

int main(int argc, char** argv) {
  printf("argc first %d \n ", argc);
  SharedStuff* stuff=new SharedStuff;
  stuff->done=false;
  stuff->patchVec=new vector<float4>;
  stuff->argc=argc;
  stuff->argv=argv;
  stuff->done=false;

  pthread_t thread1;
  State state(argc, argv);
  stuff->state=&state;
  int pthreadReturn;
  std::vector<std::string> command_line_args;
  printf("stuff add %ld \n", stuff);
  ArgParser argparser= ArgParser(argc, argv, command_line_args, state);


//  command_line_args.push_back(std::string((stuff->argv)[0]));
//  command_line_args.push_back(std::string("-i"));
//  command_line_args.push_back(std::string("/home/nasmaj/user_study_3_v2/068/out_2.wall"));

//  command_line_args.push_back(std::string("-offline"));
//  command_line_args.push_back(std::string("-t"));
//  command_line_args.push_back(std::string("3000"));
//  command_line_args.push_back(std::string("-patches"));
//  command_line_args.push_back(std::string("500"));

  stuff->tmp_mesh = remesher_main(command_line_args,true);
  pthreadReturn=pthread_create(&thread1, NULL, glutThread,(void*) stuff);

  const Mesh* mesh=stuff->tmp_mesh;
  int numZones=mesh->numZones();
  
  while(!stuff->done)
  {
    printf(". ");
    sleep(1);
    
  }
   FILE * pFile;

  if(state.outputfile_specified)
  {
     pFile = fopen (state.outputfile.c_str(),"w");
  }
  else pFile=stdout;
  vector<float3> zoneValues(numZones,make_float3(0));
  vector<float> maxLum(numZones,0);
  vector<float> minLum(numZones,9999);
  vector<float4>* patchVec=stuff->patchVec;
  int numPatches=patchVec->size();
  for(int i=0; i<numPatches; i++)
  {
    printf("el %d val: %f %f %f %f \n", i, ((*patchVec)[i]).w,((*patchVec)[i]).x, ((*patchVec)[i]).y, ((*patchVec)[i]).z);
    int zoneNum=mesh->getAssignedZoneForPatch(i);
    const Zone& zone=mesh->getZone(zoneNum);
    float zoneArea=zone.getArea();
    float4 patchValue = (*patchVec)[i];
    zoneValues[zoneNum]=zoneValues[zoneNum]+make_float3(patchValue.x,patchValue.y,patchValue.z)*mesh->getPatchArea(i)/zoneArea;
    float lum=.2989*patchValue.x+0.5870*patchValue.y+0.1140*patchValue.z;
    if(lum<minLum[zoneNum])
      minLum[zoneNum]=lum;
    if(lum>minLum[zoneNum])
      maxLum[zoneNum]=lum;
    if(lum<.001)
    {
      //fprintf(pFile,"near 0 at patch %d \n", i);
      int seed=mesh->getPatch(i).getSeed();
      Element* el=Element::GetElement(seed);
      const MeshMaterial* mmPtr=el->getMaterialPtr();
      //fprintf(pFile,"material %s \n",mmPtr->getName().c_str());
    }
  }
  fprintf(pFile,"%s %d %d %d %d ", state.inputfile.c_str(), state.month, state.day, state.hour, state.minute);  
  for(int i=0;i<numZones; i++)
  {
    float average=0.2989*zoneValues[i].x+ 0.5870*zoneValues[i].y+0.1140*zoneValues[i].z;
    fprintf(pFile,"%s %f ", mesh->getZone(i).getName().c_str(),average);//*"Zone* %s "%f "Value %f %f %f \n",mesh->getZone(i).getName().c_str(),average);//, i, zoneValues[i].x,zoneValues[i].y,zoneValues[i].z);
//    fprintf(pFile,"min %f max %f average %f \n",minLum[i],maxLum[i], average);
    
  }
  fprintf(pFile,"\n");
  if(state.outputfile_specified)
    fclose(pFile);
  
  
  float3 rgbWeights; make_float3(0.2989, 0.5870, 0.1140);

  return 0;
}
