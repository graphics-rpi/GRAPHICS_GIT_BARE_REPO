/* C Example */
#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main (int argc, char ** argv)

{
  int rank, size;

  MPI_Init (&argc, &argv);	/* starts MPI */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
  char name[10];
  gethostname(name, 10);
  printf( "Hello world from process %d of %d from %s\n", rank, size, name );
  MPI_Barrier(MPI_COMM_WORLD);
  printf( "after barrier\n");
  if(rank==8)//server
  {
    char inname[10];
    MPI_Status status;
    
    for(int i=0; i<8; i++)
      MPI_Send(name,10,MPI_BYTE,i,3,MPI_COMM_WORLD);
    for(int i=0; i<8; i++)
    {
      MPI_Recv(inname, 10, MPI_BYTE,i,4, MPI_COMM_WORLD, &status);
      printf("%s rank received from %s rank %d\n",name , inname, rank);
    }
  }
  else //client
  {
    char inname[10];
    MPI_Status status;
    int count;
    MPI_Recv(inname, 10, MPI_BYTE,8,3, MPI_COMM_WORLD, &status);
    printf("%s rank %d received %s \n", name, rank, inname);

    MPI_Send(name,10,MPI_BYTE,8,4,MPI_COMM_WORLD);
  }
  MPI_Finalize();
  return 0;
}
