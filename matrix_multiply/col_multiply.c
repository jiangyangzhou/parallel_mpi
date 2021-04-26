# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>

# include "MY_MPI.h"

typedef  double  dtype;
#define mpitype MPI_DOUBLE

int main ( int argc, char *argv[] );

int main ( int argc, char *argv[] )
{
  int id;
  int p;
  int ierr;
  int m;
  int n;
  char *file;
  dtype *storage;  //sub matrix
  dtype **a; //matrix 
  dtype *b;   //vector
  dtype *sub_ans;
  dtype *ans;
  int nprime;
  int cols;
  int col_begin;
  int col_end;
  double wtime;
  _Bool verbose;

  if (argc<3)
    printf("Not enough file input!");
  else if (argc==4) 
    verbose = (_Bool)atoi(argv[3]);
  else if (argc==3) 
    verbose = 0;

  ierr = MPI_Init( &argc, &argv);
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );
  if ( id == 0 ){
    wtime = MPI_Wtime ( );
  }
  read_col_striped_matrix (argv[1], (void *) &a,  (void*) &storage,
                    mpitype, &m, &n, MPI_COMM_WORLD);
  cols = BLOCK_SIZE(id, p, n);
  if (verbose)
    print_col_striped_matrix((void *)a, mpitype, m, n, MPI_COMM_WORLD);

  read_replicated_vector(argv[2], (void *)&b, mpitype, &nprime, MPI_COMM_WORLD);

  if (verbose)
    print_replicated_vector(b, mpitype, nprime, MPI_COMM_WORLD);
  
  //
  printf("ID:%d, and storage:%f， cols:%d. \n", id, storage[49], cols);
  col_begin = BLOCK_LOW(id, p, n);
  col_end = BLOCK_HIGH(id, p, n);
  sub_ans = malloc (m * sizeof(dtype));
  memset(sub_ans, 0, sizeof(dtype));

  for(int i=0; i<m;i++){
    for(int j=0; j<cols; j++)
      sub_ans[i] += storage[i*cols+j] * b[col_begin+j];
  }
  if (id == 0)
    ans = malloc(n*sizeof(dtype));
  MPI_Reduce(sub_ans, ans, m, mpitype, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if (id == 0){
      printf("Results:\n");
    for(int i=0;i<m; i++)
      printf("%f,", ans[i]);
  }
  printf("\n");
  free(sub_ans);
  if (id == 0)
    free(ans);
  free(storage);
  free(b);
  MPI_Barrier(MPI_COMM_WORLD);
  if ( id == 0 ) {
    printf("Time: %14f s\n", MPI_Wtime() - wtime);
  }
  ierr = MPI_Finalize();
  if ( id == 0 ) 
  {
    printf ( "\n");         
    printf ( "PRIME_MPI - Master process:\n");         
    printf ( "  Normal end of execution.\n"); 
    printf ( "\n" );
  }
  return 0;
}