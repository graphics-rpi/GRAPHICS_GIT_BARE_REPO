#ifndef RRCONT
#define RRCONT

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <GL/gl.h>
#include <zlib.h>
#include "displayParser.h"
#include "Vector3.h"
#include "Image.h"
#define PACKET_SIZE 1000000
#define LINE_SIZE 256
#define SOCKET_ERROR        -1
#define HOST_NAME_SIZE      255

#include "../argparser.h"
extern ArgParser *ARGS;

class RRController {
public:
 int hSocket;  
 int rank;

 RRController(int rnk,ConfigParser & parser){
   string strHostName;
   strHostName=(parser.displays)[rnk].host;
   (*ARGS->output) << "Remote renderer number: " << rnk << " display " << strHostName << "\r" << std::endl;
   rank=rnk;
   //printf("rank %d in constructor\r\n",rank);
   /* handle to socket */
   //    struct hostent* pHostInfo;   /* holds info about a machine */
   //    struct sockaddr_in Address;  /* Internet socket address stuct */
   long nHostAddress;
  
   unsigned nReadAmount;
   //    int nHostPort=56758+(rank*10);
  
   curr_index=0;

   //    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

   //    if(hSocket == SOCKET_ERROR)
   //    {
   //       printf("\nCould not make a socket %d\n",nHostPort);
   //return 0;
   //    }

   /* get IP address from name */
   //    pHostInfo=gethostbyname(strHostName.c_str());
   /* copy address into long */
   //   memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

   /* fill address struct */
   //    Address.sin_addr.s_addr=nHostAddress;
   //    Address.sin_port=htons(nHostPort);
   //    Address.sin_family=AF_INET;

   /* connect to host */
   //    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
   //       == SOCKET_ERROR)
   //    {
   //        printf("\nCould not connect to host %s on port %i\n", strHostName.c_str(),nHostPort);
   //    }
   //    else
   //      printf("\nConnected on %d\n",nHostPort);
   //      curr_index=0;
 }

  ~RRController() {
  }

   void  get_ack()
  {

  }
  
  void pong(v3d center, double xdim, double zdim, int score1, int score2){
    char commandline[1024];
    snprintf(commandline, 1024, "PONG %f %f %f %f %f %d %d",
             center.x(), center.y(), center.z(), xdim, zdim, score1, score2);
    send_command(commandline);
  }

  void pen_demo(int pen_detected, v3d center, int penVertical, 
		v3d targetCenter, int targetVertical, double targetRadius){
    char commandline[1024];
    snprintf(commandline, 1024, "PEN_DEMO %d %f %f %f %d %f %f %f %d %f",
             pen_detected, center.x(), center.y(), center.z(), penVertical,
	     targetCenter.x(), targetCenter.y(), targetCenter.z(), targetVertical, targetRadius);
    send_command(commandline);
  }

  void space_invaders(const char* command){
    std::cout << "SENDING COMMAND " << command << std::endl;
    send_command(command);
  }

  void blank(){
    //printf("sending blank\n");
    send_command("BLANK");
    //printf("after sending blank\n");
    flush();
    //printf("after flush\n");

  }

  void render(){
    send_command("RENDER");
  }

  void flip(){
    send_command("FLIP");
    flush();
  }

  void displayImage(const char *filename){
    send_command("DISPLAY IMAGE");
    send_file(filename);
  }

  void loadTexture(int texture_id, const char *filename, char *string, char* data, int* size){
    char commandline[1024];
    snprintf(commandline, 1024, "LOAD TEXTURE %d", texture_id);


      send_command(commandline);
      //flush();
      #ifdef COMPRESSED_TEX
      {
        //printf("sending %s \n", filename);
        if(!ARGS->use_stored_textures)
          send_compressed_texture_z(filename,string,data,size,ARGS->use_stored_textures);    
      }
      #else
      send_file(filename);
      #endif
  
  }

  void updateTexture(int texture_id, const char *filename){
    char commandline[1024];
    FILE * file;
    if(file=fopen(filename,"r"))
    {
    fclose(file);
    snprintf(commandline, 1024, "UPDATE TEXTURE %d", texture_id);
    send_command(commandline);
    send_file(filename);
    }
  
  }

  void enableVolumetricTexture(){
    send_command("ENABLE VOLUME_TEXTURE");
  }

  void disableVolumetricTexture(){
    send_command("DISABLE VOLUME_TEXTURE");
  }

  void load3DTexture(int rows, int cols, int first_slice, 
                     int last_slice, const char *file_template){
    char commandline[1024];
    snprintf(commandline, 1024, 
             "LOAD 3DTEXTURE %d %d %d %d %s", 
             rows, cols, first_slice, last_slice, file_template);
    send_command(commandline);
  }

  void setGLCam(const char *filename){

    send_command("GLCAM");

    send_file(filename);

  }

  void addGLCam(const char *filename){

    send_command("ADD GLCAM");

    send_file(filename);

  }


  void setBlendingID(int blending_id){
    char commandline[1024];
    snprintf(commandline, 1024, "BLENDING ID %d", blending_id);
    send_command(commandline);
  }    

  void loadTextureMapMap(const char *filename){
    send_command("BEGIN TEXTURE_MAP_MAP");
    send_file(filename);
    send_command("END TEXTURE_MAP_MAP");
  }

  void loadMesh(const char *filename){
    send_command("BEGIN OBJ_FILE");
    send_file(filename);
    send_command("END OBJ_FILE");
    
  }

  void loadBle(const char *filename){
    send_command("BEGIN BLE_FILE");
    send_file(filename);
    send_command("END BLE_FILE");
  }

  void disableBlending(){
    send_command("DISABLE BLENDING");
  }

  void toggleColorWeights(){
    send_command("TOGGLE COLOR WEIGHTS");
  }

  void parseMtl(TextureMapMap &map,const char *filename){
    FILE *fp = fopen(filename, "r");
    assert(fp!=NULL);
    char name[LINE_SIZE];
    char data[LINE_SIZE];
    char textline[LINE_SIZE];
    int val;
    while (!feof(fp)){
      fgets(textline, LINE_SIZE, fp);
      if (1 == sscanf(textline, "newmtl %1024s ", name));
      else if(1 == sscanf(textline, "map_Ka  %s", data)){
	//printf("found material in mtl : %s \n", name);
        val=map.getTextureID(name);
        if(val==-1)
          map.addToMap(name,data);
      }  
    }
    fclose(fp);
  }




  void loadMtl(TextureMapMap &map, const char *filename){

    send_command("BEGIN MTL_FILE");
    send_file(filename);
    send_command("END MTL_FILE");
    parseMtl( map, filename);

  }


  void sendPuzzleFile(const char *filename){
    char string_to_send[256];
    int rgb=0;

    sprintf(string_to_send,"SWITCH TO PUZZLE %d",rgb);

    send_command(string_to_send);
    send_file(filename);
    send_command("END PUZZLE_FILE");

  }

  void sendMultidisplayFile(const char *filename){
    char string_to_send[256];
    int rgb=0;

    sprintf(string_to_send,"SWITCH TO MULTIDISPLAY %d",rgb);

    send_command(string_to_send);
    send_file(filename);
    send_command("END MULTIDISPLAY");
    

  }

  void flush(){
    int i=0;
    if(curr_index>0)
    {  
      //printf("flushing with mpi\n");
      assert(i<LINE_SIZE);
      {
         #ifdef USE_MPI
         assert(MPI_SUCCESS==MPI_Send(command_string,curr_index,MPI_BYTE,rank,3,MPI_COMM_WORLD));
         #else
         assert(write(hSocket,command_string,curr_index)>0);
         #endif
          
         curr_index=0;
      }
    }
  }


void get_compressed_texture(const char *filename,char * string_to_send, char * file_to_send,  int* compressed_size){
  

  Image<sRGB> image(filename);
  assert(GL_NO_ERROR==glGetError());

  int level = 1; /* 1-9 (fastest - smallest) */
  int max_buffer_size = compressBound(image.getRows() * image.getCols() * 3);
  unsigned long comp_size= max_buffer_size;
  unsigned long uncompressed_size = image.getRows() * image.getCols() * 3;
  (*compressed_size) = max_buffer_size;
  int ret_val = compress2((Bytef*)file_to_send, &comp_size, 
                          (Bytef*)image.getData(), uncompressed_size,
                          level);
  *compressed_size=comp_size;
  assert (Z_OK == ret_val);
  sprintf(string_to_send, "%i %i %i", image.getRows(), image.getCols(), *compressed_size);
}

void cleanup_tex(char * string_to_send, char * file_to_send, int* compressed_size)
{
  delete compressed_size;
  delete [] string_to_send;
}

void quit(){
  send_command("QUIT");
}

private:
  char ack[5];
  char command_string[LINE_SIZE];
  char interum[LINE_SIZE+4];
  int curr_index;
 

  void send_command(const char *command){

    int i=0;
    snprintf(interum,LINE_SIZE, "%s\n\0", command);

    while(interum[i]!='\0')
    {

      assert(i<LINE_SIZE);
      command_string[curr_index++]=interum[i++];

      if(curr_index==LINE_SIZE)
      {

         curr_index=0;
         #ifdef USE_MPI
         assert(MPI_SUCCESS== MPI_Send(command_string,LINE_SIZE,MPI_BYTE,rank,3,MPI_COMM_WORLD));
         #else
         assert(write(hSocket,command_string,LINE_SIZE)>0);
         #endif

      }

    }

  }



void send_compressed_texture_z(const char *filename, char * string_to_send, char* file_to_send, const int* compressed_size, bool use_stored_textures){
  
  /*
   * send the texture here: 
   *  height
   *  width
   *  bytes
   * <data>
   */
   int x=0;
   int test;
   int write_amt;
   int retval;

   send_command(string_to_send);

   #ifdef USE_MPI
   assert(MPI_SUCCESS==MPI_Send(command_string,curr_index,MPI_BYTE,rank,3,MPI_COMM_WORLD));
   #else
   retval=write(hSocket,command_string,curr_index);
   assert(retval>=0);
   #endif

   int readInt;
   curr_index=0;
    
   //hack to use stored textures (still need to flush)
   if(use_stored_textures)
   {
      return;
   }

   #ifdef USE_MPI
   //printf("sending with MPI\n");
   assert(MPI_SUCCESS==MPI_Send(file_to_send, *compressed_size, MPI_BYTE, rank, 2, MPI_COMM_WORLD));
   #else
   while(x<*compressed_size){

      write_amt=(*compressed_size-x)<PACKET_SIZE?*compressed_size-x:PACKET_SIZE;  
      test=write(hSocket,  file_to_send+x,write_amt);
      assert(test==write_amt);
      x+=test;

    }
   #endif

   //printf("done sending %s \n", filename);

}

  void send_file(const char *filename){
    #ifdef USE_MPI
    assert(MPI_SUCCESS==MPI_Send(command_string,curr_index,MPI_BYTE,rank,3,MPI_COMM_WORLD));
    #else
    assert(write(hSocket,command_string,curr_index)>0);
    #endif

    curr_index=0;
    FILE *fp ;
    int failcount=0;

    while( (fp= fopen(filename, "rb"))==NULL){

      printf("failing to open %s\n",filename);
      if(failcount++>10)
      {
        assert(0&&"FAILED TO OPEN FILE");
      }
      //  break;
     }
  
     assert(fp!=NULL);
   
     if(NULL == fp){
       fprintf(stderr, "unable to open %s\n", filename);
       exit(-1);
     }
     else
     { 
       //fprintf(stderr, "able to open %s\n", filename);
     }
     int len, test, retVal;
     char buf[PACKET_SIZE];
     int loopcount=0;

     while(1) {

      //Reads the file bit by bit (not literally)
      len = fread(buf, 1, PACKET_SIZE, fp);
      assert(len>=0);

      if (len>0) { 
        #ifdef USE_MPI
        assert(MPI_SUCCESS==MPI_Send(buf,len,MPI_BYTE,rank,3,MPI_COMM_WORLD));
        #else
        test=write(hSocket,  buf,len);
        assert(test==len);
        #endif

      } else {
        break;
      }
    }

    fclose(fp);

  }

};

#endif
