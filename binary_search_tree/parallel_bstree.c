# include <math.h>
//# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>
# include <mpi.h>

// # include "MY_MPI.h"
#include <values.h>
void print_root(int *root, int low, int high, int n);
void alloc_matrix(void *** a, int m, int n, int size);
int main ( int argc, char *argv[] );
void print_diag_matrix(int *root, int n);
void print_diag_matrix_f(float *root, int n);
int index_d(int i, int j, int n){
  return (2*n+3+i-j)*(j-i) / 2 + i;
}

#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) \
                     (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
// index: cordinate[i,j] to diag index
// #define index_d(i,j,n) (2*n+3+i-j)*(j-i) / 2 + i 
//(n+1+n+1-(j-i))*(j-i)   (0,2)

int main ( int argc, char *argv[] )
{
  int id;
  int p;
  float bestcost;
  int bestroot;
  int low;
  int high;
  int i,j;
  int n;
  int r;
  float rcost;
  int *root;
  float *cost;
  float *prob; 
  double wtime;
  int begin; 
  int end;
  int num;
  int ierr;
  FILE * infileptr;
  printf("argc%d", argc);
  fflush(stdout);

  if (argc > 2){
    n = atoi(argv[1]);
    if (argc != 2+n) printf("Please give %d+2 probs\n", n);
    prob = (float *) malloc(n * sizeof(float));
    printf("Get n=%d, and prob = ", n);
    for (int i=0; i<n; i++){
      prob[i] = atof(argv[i+2]);
      printf("%.3f, ", prob[i]);
    }
    printf("\n");
  }
  
  ierr = MPI_Init(&argc, &argv);
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );
  if(argc==2){
    if(id==0){
      printf("open %s", argv[1]);
      infileptr = fopen (argv[1], "r");
      if (infileptr == NULL) printf("No valid file provides。。。\n");
      fread(&n, sizeof(int), 1, infileptr);
    } 
    MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    prob = (float *) malloc(n * sizeof(float));
    if(id==0){
      fread (prob, sizeof(float), n, infileptr);
      printf("Get n=%d, and prob = ", n);
      for (int i=0; i<n; i++){
        printf("%.3f, ", prob[i]);
      }
      printf("\n");
    }
    MPI_Bcast (prob, n, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }
  else if(argc==1) printf("at least one arg, with file input or cmd!\n");
  int counts = (n+2)*(n+1) / 2;

  if ( id == 0 ){
    wtime = MPI_Wtime ( );
  }
  root = malloc(counts * sizeof(int));
  cost = malloc(counts * sizeof(float));

  for(int i=0; i<n+1; i++){
    root[i] = i;
    cost[i] = 0;
  }
  for(int i=0; i<n; i++){
    cost[i+n+1] = prob[i];
    //root[i+n+1] = i;
  }
  MPI_Barrier(MPI_COMM_WORLD);
  int cur = n+1;
  int k;
  int *dis;
  int *dis2;
  int *num_list;
  dis = malloc(sizeof(int)*p);
  dis2 = malloc(sizeof(int) * p);
  num_list = malloc(sizeof(int) * p); 
  for(int i=1; i<n+1; i++){
    begin = BLOCK_LOW(id, p, n+1-i);
    end = BLOCK_HIGH(id, p, n+1-i);
    num = BLOCK_SIZE(id, p, n+1-i);

    for(int j=begin; j<=end; j++){
      low = j;
      high = low+i;
      bestcost = MAXFLOAT;
      for (r=low; r<high; r++){
        rcost = cost[index_d(low,r,n)] + cost[index_d(r+1, high, n)];
        for(k=low;k<high;k++) rcost += prob[k];
        if(rcost<bestcost){
          bestcost = rcost;
          bestroot = r;
        }
      }
      cost[index_d(low, high, n)] = bestcost;
      root[index_d(low, high, n)] = bestroot;
    }

    for(int j=0;j<p; j++){
      num_list[j] = BLOCK_SIZE(j, p, n+1-i);
      dis[j] = BLOCK_LOW(j, p, n+1-i) ;//* sizeof(float);  
      dis2[j] = BLOCK_LOW(j, p, n+1-i) ;//* sizeof(int); 
    }

    MPI_Allgatherv(&cost[cur+begin], num, MPI_FLOAT, &cost[cur], num_list, dis, MPI_FLOAT, MPI_COMM_WORLD);
    MPI_Allgatherv(&root[cur+begin], num, MPI_INT, &root[cur], num_list, dis2, MPI_INT, MPI_COMM_WORLD);
    cur += n+1-i;

  }
  //MPI_Gatherv(&root[cur], num, MPI_FLOAT, &root[cur], num_list, dis2, MPI_INT, 0, MPI_COMM_WORLD);
  if(id==0){
    //print_diag_matrix(root, n);
    //print_diag_matrix_f(cost, n);
    //fflush(stdout);
    print_root(root, 0, n-1, n);
  }
  if ( id == 0 ) {
    printf("Time: %14f s\n", MPI_Wtime() - wtime);
  }
  ierr = MPI_Finalize ( );
  free(root);
  free(cost);
}

//
void print_diag_matrix(int *root, int n){
  printf("(2,2):%d\n",index_d(2,2,6));
  for(int i=0; i<n+1; i++){
    for(int j=0;j<n+1;j+=1){
      if (j>=i)
        printf("%2d,", root[index_d(i,j,n)]);
      else printf("%2d,",0);
    }  printf("\n");
  }
}
void print_diag_matrix_f(float *root, int n){
  printf("(2,2):%d\n",index_d(2,2,6));
  for(int i=0; i<n+1; i++){
    for(int j=0;j<n+1;j+=1){
      if (j>=i)
        printf(" %.1f,", root[index_d(i,j,n)]);
      else printf("%4d,",0);
    }  printf("\n");
  }
}

void print_root(int *root, int low, int high, int n){
  printf("Root of tree spanning %d-%d is %d\n", 
              low, high, root[index_d(low,high+1,n)]);
  if (low < root[index_d(low,high+1,n)]-1)
    print_root(root, low, root[index_d(low,high+1,n)]-1, n);
  if (root[index_d(low,high+1,n)] < high -1)
    print_root(root, root[index_d(low,high+1,n)]+1, high, n);
}

void alloc_matrix(void *** a, int m, int n, int size){
  int i;
  void *storage;
  storage = (void *) malloc (m* n* size);
  *a = (void **) malloc(m * sizeof(void *));
  for(i=0; i<m; i++){
    (*a)[i] = storage + i*n*size;
  }
}
 