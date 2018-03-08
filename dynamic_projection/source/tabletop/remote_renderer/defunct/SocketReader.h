#ifndef SOCKET_READER_H_
#define SOCKET_READER_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>
#include <errno.h>

#include <mpi.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         300
#define QUEUE_SIZE          5

class SocketReader{
public:
int readcount;
int hSocket;
SocketReader(int port)
{

    int hServerSocket;  /* handle to socket */
    int rank;
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;
    nHostPort=port;


    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
    }
 
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("!!!!Could not connect to host!!!!\n");
        exit(0);
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);

    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        //return 0;
    }

    printf("Waiting for a connection\n");
    /* get the connected socket */
    hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

    printf("Socket %d opened on port %d\n",hSocket, port);
 
	  printf("\nGot a connection");
     
    max=read(hSocket,curr_line,256);
    readcount=1;

    if(max<0)
      printf("problem reading first line\n");
    index=0;

}

int send_ack()
{
    char* ack= (char*)"reply";
    int rank;
 MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  if(rank==0)
    assert(write(hSocket,ack,1)!=0);
}

int send_ack_to_all()
{
    char* ack= (char*)"reply";
    int rank;
 MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   printf("sending1 ack to %i \n",rank);
    assert(write(hSocket,ack,1)!=0);
   printf("sending2 ack to %i \n",rank);
}

int Jgets(char * line , int N)  //(like fgets)
{
 //  printf("in jgets\n");

    int i=0;
    //if it is left at a newline char
    //assert(curr_line[index]!='\n');
    while(curr_line[index]=='\n')
    {
      index++;  //advance one character
      if(index==256)//If our buffer is empty, put more in the buffer
      {
        index=0;

        while((max=read(hSocket,curr_line,256))==0)printf("loop1");

        if(max>0)
        {
               readcount++;
        }
        assert( max>=0);
      }
    }

    //run until we've reached our quota or got to a new line
    while(i<N&&curr_line[index]!='\n')
    {
      //copy from our buffer to the line
      if(index<max)
      line[i++]=curr_line[index++];


      if(index>=max)//If our buffer is empty, put more in the buffer
      {

        index=0;
        while((max=read(hSocket,curr_line,256))==0);
        if(max>0)
        {
            readcount++;
        }
        assert(max>=0);
      }
    }

    //add a newline and null char at the end of the line
    line[i]='\n';
    line[i+1]='\0';

    //Test if there was an error
    if(max!=-1);//warning semicolon here


    if(max==-1)
     {
      printf("max is -1 %d\n",errno);
      return -1;
     }
    if(i==0)
      return NULL;
    else
    {
      return 1;
    }
}//copies n bytes (or until new line)
    
//Simply reads a data file            
int Jread(char * ptr, int size, int count)
{
   int num_tries;
   int i=0;
    while(curr_line[index]=='\n'){
      index++;
    }
    while(i<size*count){
      
      if(max>0&&index<max){
        ptr[i++]=curr_line[index++];
      }
      if(index>=max){//If our buffer is empty, put more in the buffer
        index=0;
        num_tries=0;
        while((max=read(hSocket,curr_line,256))==0){
           num_tries++;
           if(num_tries>1000)
               printf("Have tried receiving 1000 times\n");
        }//wait for more data NOTE:semi-colon
        if(max>0){
           readcount++;
        }
        assert(max>=0);
      }
    }
    return (int) i;
}//returns desired number of bytes
//moves current char to address provided
//index++
//returns the number of objects of size size to be read.
int Jgetc()
{

    int i=0;
    int retVal;
   
    retVal=curr_line[index++];
    if(index==max)//If our buffer is empty, put more in the buffer
    {
      index=0;
      while((max=read(hSocket,curr_line,256))==0);//warning semicolon here
      if(max>0)
      {
         readcount++;
      }
      assert(max>=0);
    }
    

    if(max==-1)
    {
       printf("getc errno %i\n",errno);
       return -1;
    }

    return retVal;
}//returns one char

void println()
{
   //printf("line: %s /n",curr_line);
}

bool eof()
{
   if(max<=index)
   {
       max=read(hSocket,curr_line,256);
   }
   if(max<=index)
    return true;
   else
    return false;
}

void closef()
{
   close(hSocket);
}

private:
char curr_line[256+1]; 
int index;
int max; //(if reading to 256 isn't successful, then has the current end of file)
};

#endif
