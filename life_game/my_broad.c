# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>

int main ( int argc, char *argv[] );
void my_bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator);
int main ( int argc, char *argv[] )
{
  int id;
  int p;
  int ierr;
  int m = 0;
  int n = 0;
  int num_j;
  int num_k;

   ierr = MPI_Init ( &argc, &argv );

  if ( ierr != 0 )
  {
    printf ( "\n" );
    printf ( "LIFE GAME - Fatal error!\n" );
    printf ( "  MPI_Init returns nonzero IERR.\n" );
    exit ( 1 );
  }
  //Get the number of processes.
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
  /*
    Determine this processes's rank.
  */

  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );
  int *data;
  if(id==0){
    data = malloc(sizeof(int)*1000);
    for(int i=0;i<1000;i++) data[i] = i;
  }

  my_bcast(data, 1000, MPI_INT, 0, MPI_COMM_WORLD);
  printf("id:%d, data[%d] is:", id, id, data[id]);
}

void my_bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator) {
  int id;
  MPI_Comm_rank(communicator, &id);
  int print_board;
  MPI_Comm_size(communicator, &p);

  if (id == root) {
    // If we are the root process, send our data to everyone
    int i;
    for(i=1; i<p && i<=2; i++)
        MPI_Send(data, count, datatype, i, 0, communicator);
  } else {
    // If we are a receiver process, receive the data from the root
    int new_id = id;
    if(id==0) new_id = root;
    int re_id = new_id / 2;
    if(re_id==0) re_id = root;
    MPI_Recv(data, count, datatype, re_id, 0, communicator,
             MPI_STATUS_IGNORE);
    if(id*2<p)
        MPI_Send(data, count, datatype, id*2, 0, communicator);
    if(id*2+1<p)
        MPI_Send(data, count, datatype, id*2+1, 0, communicator);
  }
}