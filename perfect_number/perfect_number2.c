# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include "MY_MPI.h"

typedef  int  dtype;
#define mpitype MPI_DOUBLE

int main ( int argc, char *argv[] );
dtype * prime_number ( int n, int id, int p, char* prime_list, int *prime_num );
char judge_prime(long long n);
int * manager(int n);

void worker(int root);

int main ( int argc, char *argv[] )
{
  int id;
  int p;
  int ierr;
  int n;
  long long perfect_num;
  double wtime;
  if (argc==2)
    n = atoi(argv[1]);
  else n=8;

  ierr = MPI_Init ( &argc, &argv );

  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );

  MPI_Barrier(MPI_COMM_WORLD);
  if (id==0){
    wtime = MPI_Wtime();
    printf("N:%d \n", n);
  }

  int * ans;
  if (id==0){
    ans = manager(n);
  }
  else{
    worker(0);
  }
  if (id==0){
    printf("Ans of n:");
    for (int i=0; i<n; i++) printf("%d ", ans[i]);
    printf("\n");
    printf("Find %d perfect number:\n", n);
    for(int i=0; i<n; i++){
      perfect_num = ((long long)1<<(long long)ans[i])*((long long)1<<((long long)ans[i]-1));
      printf("%lld \n", perfect_num);
    }
    printf("Time: %f\n", MPI_Wtime() - wtime);
  }

  ierr = MPI_Finalize ( );
  return 0;

}
//manager 分配 i, worker : 判断 2^i-1是素数吗
int * manager(int n){
  int id;
  int p;
  int src;
  int tag;
  MPI_Status status;
  MPI_Request pending;
  MPI_Comm_rank (MPI_COMM_WORLD, &id);
  MPI_Comm_size (MPI_COMM_WORLD, &p);
  int flag;
  int send_id = 2;
  int done=0;
  int *ans;
  ans = malloc(sizeof(long) * 8);
  while(1){
    MPI_Recv(&flag, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    src = status.MPI_SOURCE;
    tag = status.MPI_TAG;
    long raw_num = (1<<tag) -1;
    if (flag==1 && tag>1){
      printf("manager %d recv from worker %d, 2^%d-1(%ld) prime? :%d\n", id, src, tag, raw_num, flag);
      ans[done] = tag;
      done += 1;
      if(done>=n) break;
    }
    MPI_Send(&send_id, 1, MPI_INT, src, send_id, MPI_COMM_WORLD);
    send_id +=1;
  }
  for(int i=1;i<p; i++)
    MPI_Send(NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD  );
  return ans;
}

void worker(int root){
  int id;
  int n;
  int recv_len;
  long long num;
  int is_prime;
  MPI_Request pending;
  MPI_Status status;
  MPI_Comm_rank (MPI_COMM_WORLD, &id);
  //tell manager, it's ready
  MPI_Send(&id, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
  while(1){
    MPI_Probe(root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, mpitype, &recv_len);
    if(!recv_len) break;
    MPI_Recv(&n, 1, MPI_INT, root, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    num = (1 << n) - 1;
    is_prime = (int)judge_prime(num);
    MPI_Send(&is_prime, 1, MPI_INT, root, status.MPI_TAG, MPI_COMM_WORLD);
  }
}

char judge_prime(long long n){
  long sieve = (long) sqrt((double)n);
  //printf("sieve is:%ld, %lld\n", sieve, n);
  if(n%2==0) return 0;
  else{
    char is_prime = 1;
    for(int i=3;i<=sieve; i+=2){
      if (n%i==0)
        is_prime=0;
    }
    return is_prime;
  }
}
