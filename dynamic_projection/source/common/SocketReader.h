#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <cstdlib>

#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <mpi.h>


#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1000000
#define QUEUE_SIZE          5



#ifndef SocketReaderINCLUDED
#define SocketReaderINCLUDED
class SocketReader{
 public:
  int readcount;
  int hSocket;

  int getMessage(int socket, char* buf, int len)
  {
#ifdef USE_MPI
    MPI_Status status;
    int count;
    //printf("mpi receiving\n");
    MPI_Recv(buf, len, MPI_BYTE,8,3, MPI_COMM_WORLD, &status);
    //printf("done mpi receiving\n");
    MPI_Get_count(& status, MPI_BYTE, &count);
    return count;
#else
    while((max=read(socket,buf,len))==0)printf("loop1");
    return max;
#endif
  }

  SocketReader(int port, bool verbose_) // = true)
  {

    verbose = verbose_;

    //    int hServerSocket;  /* handle to socket */
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
 
    if (verbose) 
    {
      printf("be4 getMessage\r\n");
    }
    max=getMessage(hSocket,curr_line,BUFFER_SIZE);
    if (verbose) 
    {
      printf("after getMessage\r\n");
    }

    if(max<0)
    {
      if (verbose) {
        printf("problem reading first line\r\n");
      }
      else
      {
        if (verbose) 
        {
          printf("read first line on port %d \r\n", port);
        }
      }
    }
    index=0;

  }


  int Jgets(char * line , int N)  //(like fgets)
  {

    int i=0;
    //if it is left at a newline char
    //assert(curr_line[index]!='\n');
    while(curr_line[index]=='\n')
      {
	index++;  //advance one character
	if(index==BUFFER_SIZE)//If our buffer is empty, put more in the buffer
	  {
	    index=0;

	    max=getMessage(hSocket,curr_line,BUFFER_SIZE);

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
	    //printf("before max is : %d \n");
	    index=0;
	    max=getMessage(hSocket,curr_line,BUFFER_SIZE);
	    if(max>0)
	      {
		      readcount++;
	      }
	    if(max<0)
	      { 
		if (verbose) {
		  printf("Error max is : %d \r\n");
		}
		assert(0);

	      }
	  }
      }

    //add a newline and null char at the end of the line
    line[i]='\n';
    line[i+1]='\0';

    //Test if there was an error
    if(max!=-1);//warning semicolon here


    if(max==-1)
      {
	if (verbose) {
	  printf("max is -1 %d\r\n",errno);
	}
	return -1;
      }
    if(i==0)
      return 0; //NULL;
    else
      {
	return 1;
      }
  }//copies n bytes (or until new line)
    
  //Simply reads a data file            
  int Jread(char * ptr, int size, int count)
  {
#ifdef USE_MPI
    assert(0&&"Can't use Jread with MPI");
#else
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
        while((max=read(hSocket,curr_line,BUFFER_SIZE))==0){
	  num_tries++;
	  if(num_tries>1000)
	    printf("Have tried receiving 1000 times\r\n");
        }//wait for more data NOTE:semi-colon
        if(max>0){
	  readcount++;
        }
        assert(max>=0);
      }
    }
    return (int) i;
#endif
  }//returns desired number of bytes
  //moves current char to address provided
  //index++
  //returns the number of objects of size size to be read.
  int Jgetc()
  {
    //    assert(0&&"defunct");


    int i=0;
    int retVal;
   
    retVal=curr_line[index++];
    if(index==max)//If our buffer is empty, put more in the buffer
      {
	index=0;
	//      while((max=read(hSocket,curr_line,BUFFER_SIZE))==0);//warning semicolon here
        max=getMessage(hSocket, curr_line, BUFFER_SIZE);
	if(max>0)
	  {
	    readcount++;
	  }
	assert(max>=0);
      }
    

    if(max==-1)
      {
	if (verbose) {
	  printf("getc errno %i\r\n",errno);
	}
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
	    max=getMessage(hSocket,curr_line,BUFFER_SIZE);
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
    char curr_line[BUFFER_SIZE+1]; 
    int index;
    int max; //(if reading to BUFFER_SIZE isn't successful, then has the current end of file)
    bool verbose;
};



#endif
