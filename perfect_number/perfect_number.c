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

int main ( int argc, char *argv[] )
{
  int id;
  int p;
  int ierr;
  int n;
  int n_factor;
  int n_hi;
  int n_lo;
  dtype *prime_vec;
  dtype *prime_vec_all;
  char *primes_part;
  int prime_num;
  int prime_num_all;
  int * prime_num_list;
  int *buffer_size;
  int *dis_size;
  int * cur_prime_num;
  double wtime;
  if (argc==2)
    n = atoi(argv[1]);
  else n=100000000;

  ierr = MPI_Init ( &argc, &argv );

  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &p );
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &id );

  MPI_Barrier(MPI_COMM_WORLD);
  if (id==0){
    printf("N:%d \n", n);
  }

  //primes_part = malloc((n+1)*sizeof(int));
  //primes = malloc((n+1)*sizeof(int));
  
  prime_vec = prime_number ( n, id, p, primes_part, &prime_num);
  if(id==0){
    prime_num_list = malloc(p*sizeof(int));
  }
  printf("1 prime num:%d", prime_num);
  fflush(stdout);
  MPI_Gather (&prime_num, 1, MPI_INT, prime_num_list, p, MPI_INT, 0, MPI_COMM_WORLD);
  printf("id:%d, prime num:%d", id, prime_num);
  fflush(stdout);
  
  if(id==0){
    for(int i=0;i<p;i++) printf("|%d,", prime_num_list[i-1]);
    fflush(stdout);
    cur_prime_num = malloc(p*sizeof(int));
    buffer_size = malloc(p*sizeof(int));
    dis_size = malloc(p*sizeof(int));
    cur_prime_num[0] = 0;
    for(int i=1;i<p;i++) {
      cur_prime_num[i] = cur_prime_num[i-1] + prime_num_list[i-1];
      buffer_size[i] = prime_num_list[i] * sizeof(int);
      dis_size[i] = cur_prime_num[i] *sizeof(int);
    }
    prime_num_all = cur_prime_num[p-1] + prime_num_list[p-1];
    prime_vec_all = malloc(prime_num * sizeof(dtype));
    printf("Prime Num is: %d",prime_num);
  }
   MPI_Barrier(MPI_COMM_WORLD);
   printf("ID:%d come here \n", id);
   fflush(stdout);
    //Gather prime vector
  ierr = MPI_Gatherv (prime_vec, prime_num, mpitype, prime_vec_all,  buffer_size,  dis_size, mpitype, 0,  MPI_COMM_WORLD );

  ierr = MPI_Finalize ( );
  return 0;

}


dtype * prime_number ( int n, int id, int p, char* prime_list, int *prime_num )
{
  //if(id>=1) return;
  int i;
  int j;
  int prime;
  int total;
  int begin = BLOCK_LOW(id, p, n);
  int end = BLOCK_HIGH(id, p, n);
  int num = BLOCK_SIZE(id, p, n);
  printf("id:%d, begin:%d, end%d, size:%d\n", id, begin, end, num);
  int collect_num;
  int sieve_begin;
  total = 0;

  int sieve_num = (int) sqrt(n);
  int sub_sieve_num = (int) sqrt(sieve_num);
  collect_num = sieve_begin > num ? sieve_begin : num;
  prime_list = malloc(collect_num*sizeof(char));
  prime_list[0] = 0;
  prime_list[1] = 0;
  for(int i=2;i<collect_num; i++) prime_list[i]=1;  //质数为1
  for(int i=4;i<collect_num; i+=2) prime_list[i]=0;  //合数为0
  for(int i=5; i<sieve_num; i+=2){   //只check奇数
    for(int j=3; j<i && j<sub_sieve_num; j+=2)
        if(i%j==0){
            prime_list[i]=0; break;  //合数置为0
        }
  }
  //for(int i=0;i<sieve_num;i++) printf("%d,%d/", i, prime_list[i]);

  int k=0 ;
  for ( i = 3; i <=sieve_num;)
  { 
    for(i; i<=sieve_num;){
      i++;
      if(prime_list[i]==1) break;
    }
    sieve_begin = (begin-1) / i * i + i;
    //printf("sieve begin:%d", sieve_num);
    if(i>sieve_num) break;
    //printf("i:%d,", i);
    for(int m=sieve_begin-begin; m<end; m+=i) prime_list[m]=0;
    //printf("k:%d, i:%d, s:%d\t",k, i, sieve_num);
    k+=1;
  }

  //printf("n:%d, id:%d, p:%d\n",n, id, p);
  dtype *prime_vec;
  for(int i=0;i<num;i++) {
    if(prime_list[i]==1) (*prime_num)+=1;
  }
  prime_vec = malloc((*prime_num) * sizeof(int));
  j=0;
  for(int i=0;i<num;i++) {
    if(prime_list[i]==1){
      prime_vec[j] = i + begin;
    }
  }
  //printf("id:%d, p num: %d\n", id, prime_num);

  return prime_vec;
}