#ifndef REMOTEDISPLAY_H
#define REMOTEDISPLAY_H

#include <string>

#include "GigEVisionCamera.hpp"
#include "TextureMapMap.hpp"
#include "RRController.h"



class RemoteDisplay{
 public:
 void parsePuzzle(TextureMapMap &map,const char *filename){
    FILE *fp = fopen(filename, "r");
    assert(fp!=NULL);
    char puzzle_string[LINE_SIZE];
    char full_filename[LINE_SIZE];
    char textline[LINE_SIZE];
    int val;
    while (!feof(fp)){
      //sprintf(full_filename)
      fgets(textline, LINE_SIZE, fp);
      assert (1 == sscanf(textline, "%1024s ", puzzle_string));

      for(int i=0;i<=4;i++){
        val=map.getTextureID(walls[i]);
        if(val==-1){
          map.addToPuzzleMap(walls[i],full_filename);
        }
      }

      
    }
    fclose(fp);
  }

 void parseMultidisplayFile(TextureMapMap &map,const char *filename){
    FILE *fp = fopen(filename, "r");
    assert(fp!=NULL);
    char wall_name[LINE_SIZE];
    char file_name[LINE_SIZE];
    char textline[LINE_SIZE];
    int val;
    while (!feof(fp)){

      fgets(textline, LINE_SIZE, fp);
      assert (2 == sscanf(textline, "%1024s %1024s", wall_name, file_name));
      printf("Got %s %s\n",wall_name,file_name);
      for(int i=0;i<=4;i++){
        val=map.getTextureID(wall_name);
        if(val==-1){
          map.addToMultiMap(wall_name,file_name);
        }
      }
    }
    fclose(fp);
  }

  void addDisplay(RRController *r){
    displays.push_back(r);
  }

  void pong(v3d center, double xdim, double zdim, int score1, int score2){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->pong(center, xdim, zdim, score1, score2);
    }
  }
  
  
  void pen_demo(int pen_detected, v3d center, int penVertical, 
		v3d targetCenter, int targetVertical, double targetRadius){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->pen_demo(pen_detected, center, penVertical, targetCenter, targetVertical, targetRadius);
    }
  }
  

  void space_invaders(const char* command){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->space_invaders(command);
    }
  }

  void displayImage(const char *filename){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->displayImage(filename);
    }
  }

  void displayImages(const char **filenames){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->displayImage(filenames[i]);
    }
  }

  void loadTextureMapMap(const char *filename){

    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->loadTextureMapMap(filename);
    }

  }

  void loadBle(const char *filename){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->loadBle(filename);
    }
  }

  void disableBlending(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->disableBlending();
    }
  }

  void toggleColorWeights(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->toggleColorWeights();
    }
  }

  void enableVolumetricTexture(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->enableVolumetricTexture();
    }
  }

  void disableVolumetricTexture(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->disableVolumetricTexture();
    }
  }

  void loadMtl(TextureMapMap &map, const char *filename){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->loadMtl(map, filename);
    }
  }

  void loadMesh(const char *filename){
    for (unsigned i=0; i<displays.size(); i++){
      //printf("sending mesh to %i\n",i);
      displays[i]->loadMesh(filename);
    }
  }


  void loadTexture(int texture_id, const char *filename, RRController** renderers)
  {
    //printf("sending texture to all displays\n");
    
    //FILE * filep;


    // VERBOSE
    //printf("opening texture file : %s\r\n", filename);
    //sleep(1);
    //if(filep=fopen(filename,"r"))
      {

	      int* size=new int;
      	char * string=new char[LINE_SIZE];
      
     
      for (unsigned i=0; i<displays.size(); i++)
      {

        //printf("sending texture %s to display %i \r\n",filename, i);
        displays[i]->loadTexture(texture_id, filename,string,file,size);
      }

      displays[0]->cleanup_tex(string, file, size);
    }
  }

  void parseMtl(TextureMapMap &map,const char *filename)
  {  
    for (unsigned i=0; i<displays.size(); i++){
        displays[i]->parseMtl(map,filename);
    }
  }
  void updateTexture(int texture_id, const char *filename){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->updateTexture(texture_id, filename);
    }
  }

 void load3DTexture(int rows, int cols, int first_slice, int last_slice, 
                     const char *file_template){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->load3DTexture(rows, cols, first_slice,
                                 last_slice, file_template);
    }    
  }

  void render(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->render();
    }
  }

  void flip(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->flip();
    }
  }

  void sendPuzzleFile(const char *filename){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->sendPuzzleFile(filename);
    }
  }

  void sendMultidisplayFile(TextureMapMap & map, const char *filename){
   
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->sendMultidisplayFile(filename);
    }

    parseMultidisplayFile(map, filename);

  }

  void flush(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->flush();
    }
  }

  void quit(){
    for (unsigned i=0; i<displays.size(); i++){
      displays[i]->quit();
    }
  }

  void take_image(long shutter, const std::string &out_filename)
  {
    static bool camera_initialized=0;
    static GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER16>
        camera;

    int gain = 0;
    int red_bal = 148;
    int blue_bal = 195;
    
    
    int irows = 1456;
    int icols = 1936;
    int row_offset = 0;
    int col_offset = 0;
    camera.SetExposure(shutter);
    camera.SetGain(gain);
    camera.SetWhiteBalance(red_bal, blue_bal);  // red, blue
    camera.SetBinning(1, 1);
    camera.SetROI(row_offset, col_offset, irows, icols);
    if(!camera_initialized) {
      camera.InitContinuousCapture();
    }
    Image<Vector3<uint16_t> > image;
    sleep(1);
    for (int i=0; i<3; i++){
      image = camera.GetNextFrame();
    }

    Image<sRGB> out(image.getRows(), image.getCols());
    for (int row=0; row<out.getRows(); row++){
      for (int col=0; col<out.getCols(); col++){
        out(row, col) = sRGB(image(row, col)/uint16_t(16));
      }
    }
    std::cout << "saving new image " << out_filename << "\r" << std::endl;
    out.write(out_filename.c_str());
    camera_initialized=1;

  }

  void blank_and_take_image(bool really_take_image)
  {
    std::cout << "telling projectors to blank...\r" << std::endl;
    for (unsigned i=0; i<displays.size(); i++)
    {
      displays[i]->blank();
    }

    //Make sure all of them have actually blanked
    MPI_Barrier(MPI_COMM_WORLD);
    if(really_take_image)
    {
      std::cout << "take photos...\r" << std::endl;
      take_image(25000,"/ramdisk/out_lights_on.ppm");    
      take_image(300000,"/ramdisk/out_lights_off.ppm");
    }
    else
    {
      std::cout << "not taking photos...\r" << std::endl;
    }

    // The projectors should come back on after the second barrier
    MPI_Barrier(MPI_COMM_WORLD);
    
  }

 private:
  std::vector<RRController *> displays;
  char file[1024*1024*4];
  char* walls[5];
};

#endif
