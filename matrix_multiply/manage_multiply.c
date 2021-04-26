# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>

# include "MY_MPI.h"

typedef  double  dtype;
#define mpitype MPI_DOUBLE
#define EMPTY_MSG 3

int main ( int argc, char *argv[] );
void worker(int root, int m, int n, dtype *b);
dtype * manager(FILE *infileptr,  int m, int n, dtype *b);
void print_vector(dtype *v, int n){
  for(int i=0; i<n; i++)
    printf("%8f ", v[i]);
  printf("\n");
}

int main ( int argc, char *argv[] )
{
  int id;
  int p;
  int ierr;
  int m;
  int n;
  dtype *ans;
  dtype *b; 
  FILE * infileptr;
  int nprime;
 int datum_size = sizeof (dtype);

  double wtime;
  _Bool verbose;

  if (argc<3){
    printf("Not enough file input!");
    return -1;
  }
  else if (argc==4) 
    verbose = (_Bool)atoi(argv[3]);
  else if (argc==3) 
    verbose = 0;

  ierr = MPI_Init(&argc, &argv);
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );

  MPI_Barrier(MPI_COMM_WORLD);
  // Read Matrix m & n. 
  if ( id == 0 ){
    wtime = MPI_Wtime ( );
    infileptr = fopen (argv[1], "r");
    if (infileptr == NULL) n = 0;
    else {
        fread (&m, sizeof(int), 1, infileptr);
        fread (&n, sizeof(int), 1, infileptr);
        printf("read m,n");
        fflush(stdout);
    }
  }
  printf("m:%d, n:%d", m, n);
  fflush(stdout);
  MPI_Bcast (&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  printf("m:%d, n:%d", m, n);
  fflush(stdout);
  b = malloc(n*datum_size);

  // Read vector
  if (id == 0){
    FILE *infileptr2 = fopen (argv[2], "r");
    if (infileptr2 == NULL) n = 0;
    else {
        fread (&n, sizeof(int), 1, infileptr2);
        fread (b, datum_size, n, infileptr2);
        fclose (infileptr2);
    }
    //print_vector(b, n);
  }

  MPI_Bcast (b, n, mpitype,  0, MPI_COMM_WORLD);
  printf("Begin manager and worker....\n");
  if (id ==0){
    ans = manager(infileptr,  m, n, b);
  }else{
      worker(0, m, n, b);
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  if ( id == 0 ) {
    printf("\nTime: %14f s\n", MPI_Wtime() - wtime);
    printf("Results:\n");
    print_vector(ans, m);
    fflush(stdout);
    free(ans);
  }else{
    //printf("ID %d fiinsh as well.\n", id);
  }
  free(b);
  //printf("ID:%d come here\n", id);
  ierr = MPI_Finalize();
  //printf("\nierr is %d", ierr);
  if ( id == 0 ) 
  {
    printf ( "\n");         
    printf ( "PRIME_MPI - Master process:\n");         
    printf ( "  Normal end of execution.\n"); 
    printf ( "\n" );
  }
  return 0;
}

dtype * manager(FILE *infileptr, int m , int n, dtype *b){
    int k;
    int src;
    int tag;
    int done = 0;
    int id;
    int p;
    int send_id = 0;
    int datum_size = sizeof (dtype);
    MPI_Status status;
    MPI_Request pending;
    dtype *buffer;
    dtype *ans;
    dtype sub_ans;
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    buffer = malloc(n * sizeof(dtype));
    ans = malloc(m * sizeof(dtype));
    // k = (*n)/p/10 > 1 ? (*n)/p/10 : 1;
    printf("\nDoes here ok");
    fflush(stdout);

    while(done < m){
      //printf("Done:%d\n", done);
      MPI_Irecv(&sub_ans, 1, mpitype, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &pending);
      if (send_id < m){  //send row of matrix
        fread (buffer, datum_size, n, infileptr);
        MPI_Wait(&pending, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        MPI_Send(buffer, n, mpitype, src, send_id+1, MPI_COMM_WORLD);
        send_id +=1;
        //printf("send ...send id:%d, done:%d\n", send_id, done);
      }
      else{   // Send stop message
        MPI_Wait(&pending, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        MPI_Send(NULL, 0, mpitype, src, 0, MPI_COMM_WORLD);
      }
      if (tag>0){  // Gather result
        ans[tag-1] = sub_ans;
        done+=1;
      }   
    } 
    free(buffer);
    fclose(infileptr);
    return ans;
}

void worker(int root, int m, int n, dtype *b)
{
  int recv_len;
  dtype *buffer;
  int id;
  int p;
  dtype ans = 0;
  MPI_Request pending;
  MPI_Status status;

  MPI_Comm_rank (MPI_COMM_WORLD, &id);

  buffer = malloc(n *sizeof(dtype));
  memset(buffer, 0, n * sizeof(dtype));


  // Tell manager, this worker is prepared 
  MPI_Send(buffer, 1, mpitype, root, 0, MPI_COMM_WORLD);
  //printf("here send ok\n");
  fflush(stdout);
  while(1){
      MPI_Probe(root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, mpitype, &recv_len);
      if(!recv_len) break;
      MPI_Recv(buffer, n, mpitype, root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      //printf("worker recv \n ");
      ans = 0.0;
      for (int i=0; i<n; i++){
          ans += buffer[i] * b[i];
      }
      MPI_Send(&ans, 1, mpitype, root, status.MPI_TAG, MPI_COMM_WORLD);
  }
  //printf("worker finish! id:%d \n", id);
  free(buffer);

}
