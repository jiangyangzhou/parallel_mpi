# include <math.h>
//# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>

// # include "MY_MPI.h"
#include <values.h>
void print_root(int **root, int low, int high);
void alloc_matrix(void *** a, int m, int n, int size);
int main ( int argc, char *argv[] );

int main ( int argc, char *argv[] )
{
  int id;
  int p;
  float bestcost;
  int bestroot;
  int high;
  int i,j;
  int low;
  int n;
  int r;
  float rcost;
  int **root;
  float **cost;
  float *prob; 
  n = atoi(argv[1]);

  prob = (float *) malloc(n * sizeof(float));
  for (int i=0; i<n; i++){
    prob[i] = atof(argv[i+2]);
  }
  alloc_matrix((void ***) &cost, n+1, n+1, sizeof(float));
  alloc_matrix((void ***) &root, n+1, n+1, sizeof(int));
  for (low=n;low>=0; low--){
    cost[low][low] = 0,0;
    root[low][low] = low; 
    for (high=low+1; high<=n;high++){
      bestcost = MAXFLOAT;
      for (r=low; r<high; r++){
        rcost = cost[low][r] + cost[r+1][high];
        for(j=low; j<high; j++) rcost += prob[j];
        if(rcost<bestcost){
          bestcost = rcost;
          bestroot = r;
        }
        cost[low][high] = bestcost;
        root[low][high] = bestroot;
      }
    }
  }
  print_root(root, 0, n-1);
}

//

void print_root(int **root, int low, int high){
  printf("Root of tree spanning %d-%d is %d\n", 
              low, high, root[low][high+1]);
  if (low < root[low][high+1]-1)
    print_root(root, low, root[low][high+1]-1);
  if (root[low][high+1] < high -1)
    print_root(root, root[low][high+1]+1, high);
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
