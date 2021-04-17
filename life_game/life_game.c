# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>

int main ( int argc, char *argv[] );
void life_evaluate(int **cell_board, int **temp_board, int x, int y);
void life_game_one ( int m,int n, int *row_num, int *row_begin, int id, int p,  int *** split_cells, int *** split_temp );
void print_board (int ** temp_board, int m, int n);
void copy_board (int m, int n, int id, int p, int ** cell_board, int ** temp_board);
char *getfileall(char *fname);
int** get_csv(char*fname, int* m, int *n);
/******************************************************************************/
int main ( int argc, char *argv[] )
{
  int id;
  int ierr;
  int m = 0;
  int n = 0;
  int num_j;
  int num_k;
  int p;
  int **cell_board;
  int **temp_board;
  double wtime;
  if(argc==3){
    num_j = atoi(argv[1]);
    num_k = atoi(argv[2]);
  }
  else printf("Please give iterations j and print frequency k.");

  char * buff;
  cell_board = get_csv("board.csv", &m, &n);

  printf("m:%d, n:%d\n", m, n);fflush(stdout);
  for(int i=0;i<m; i++){
    for(int j=0; j<n; j++){
      printf("%d", cell_board[i][j]);fflush(stdout);}
    printf("\n");
  }fflush(stdout);
  
  /*
  temp_board = (int**)malloc(sizeof(int*)*(m+2));
  for(int i=0; i<m+2; i++){
    temp_board[i] = malloc(sizeof(int)*(n+2));
  }
  for(int i=0;i<=m+1; i++){
    for(int j=0; j<=n+1; j++){
      temp_board[i][j] = cell_board[i][j];
    }
  }
  */

  /*
    Initialize MPI.
  */
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

  if ( id == 0 )
  {
    printf ( "\n" );
    printf ( "PRIME_MPI\n" );
    printf ( "  C/MPI version\n" );
    printf ( "\n" );
    printf ( "  An MPI program to perfrom life game.\n" );
    printf ( "  The number of processes is %d\n", p );

  }

  if ( id == 0 )
  {
    wtime = MPI_Wtime ( );
  }

  int *** split_cells ;
  split_cells = (int ***) malloc(sizeof(int**)*(p));
  int *** split_temp = (int ***) malloc(sizeof(int**)*p);
  
  int row = m / p;
  int rem = m % p;
  int r = 0;
  int *row_num;
  int *row_begin;
  row_num = malloc(sizeof(int)*p);
  row_begin = malloc(sizeof(int)*p);
  if(id==0){
    for(int i=0;i<p;i++){
      if(id<rem){
        row_num[i] = row+1;
        r += row+1;
      }
      else {row_num[i] = row+1; r += row;}
      split_cells[i] = &cell_board[r];
      ierr = MPI_Send(&split_cells[i], row_num[i]*(n+2), MPI_INT, i, 0, MPI_COMM_WORLD);
      //ierr = MPI_send(&split_temp[i], row_num[i]*(n+2), MPI_INT, i, MPI_COMM_WORLD);
    }
  }
  else{
    MPI_Recv(&split_cells[id], row_num[id]*(n+2), MPI_INT, 0, 0, MPI_COMM_WORLD,
              MPI_STATUS_IGNORE);
  }
  for(int i=0;i<=m+1; i++){
    for(int j=0; j<=n+1; j++){
      split_temp[id][i+1][j] = split_cells[id][i][j];
    }
  }
  int * buff_num = malloc(sizeof(int)*p);
  for(int i=0;i<p;i++) buff_num[i] = row_num[i]*(n+2);
  for(int j=0; j<num_j; j++){
    if (j%1 == 0 && id==0) 
      print_board(cell_board, m, n);
    life_game_one (m, n, row_num, row_begin, id, p, split_cells, split_temp);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gatherv(split_cells, buff_num[id], MPI_INT, split_cells, buff_num, buff_num, MPI_INT,0, MPI_COMM_WORLD);
    for(int i=0;i<=m+1; i++){
    for(int j=0; j<=n+1; j++){
      temp_board[i][j] = cell_board[i][j];
    }
  }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  

  if ( id == 0 )
  {
    wtime = MPI_Wtime ( ) - wtime;
    printf ( "  %8d  %8d  %14f\n", m, n, wtime );
  }

  /*
    Terminate MPI.
  */
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


// This function read csv file and initialize cell board
int** get_csv(char* fname, int* m, int* n)
{
  FILE *fp;
  if ((fp=fopen(fname,"r"))==NULL){
      printf("打开文件%s错误\n",fname);
      return NULL;
  }
  //int m, n;
  //m = *m_p;
  //n = *n_p;
  char ch=fgetc(fp);
  while(ch!=EOF){
      if(ch=='\n') (*m)+=1;
      if(ch==',' && (*m)==0) (*n)+=1;
      ch=fgetc(fp);
  }
  rewind(fp);
  int **matrix = (int**)malloc(sizeof(int*)*((*m)+2)); 
  for(int i=0; i<(*m)+2; i++){
    matrix[i] = malloc(sizeof(int)*((*n)+2));
    for(int j=0; j<(*n)+2; j++)
    matrix[i][j]=0;
  }
  char txt[1000];
  char *p=txt;
  char *num, *head; //
  num = malloc(10*sizeof(char));
  head = num;
  int i=0;
  int j=-1;
  while((fgets(txt,1000,fp))!=NULL){//循环读取1000字节,如果没有数据则退出循环
    p = txt;
    while((*p)!='\0' && p!=NULL){
      //printf("%s", p);
      *num = (*p);
      //printf("First%s||%s......\t",num,head);
      while((*p)!='\n' && (*p)!=',' && (*p)!='\0'){
        num+=1;
        p++;
        *num = *p;
      } 
      j++;
      *(num) ='\0';
      matrix[i+1][j+1] = atoi(head);
      //printf("ma[i][j]:%d\n",matrix[i][j] );
      num=head;
      p++;
      if(*p=='\n'){i+=1; j=-1;break;}
    }
  }
  //  printf("%d,\n", matrix[0][0]);fflush(stdout);
  for(int i=0; i<8; i++){
    for(int j=0; j<8; j++){
      printf("%d,", matrix[i][j]);fflush(stdout);}
    printf("\n");
  }
  return matrix;
}

void print_board(int **cell_board, int m, int n){
  for(int i=0; i<=m+1; i++){
    for(int j=0; j<=n+1; j++){
      printf("%d ", cell_board[i][j]);
    }
    printf("\n");
  }
  printf("\n");

}
/******************************************************************************/
//Ok we need to write a life game 
//this is a function to evaluate the cell should be live or dead 
//so Give cells board, and its coordinate (x,y), it check neighbors and give the decision 
// 3 neibors: live | 2 neibors no change | <=1 neibors, dead
//Let's do it !
void life_evaluate(int **cell_board, int **temp_board, int x, int y){
  int counts=0;
  for(int i=-1; i<=1; i++){
    for(int j=-1; j<=1; j++){
      if( !(i==0 && j==0))
        counts+=temp_board[x+i][y+j];
    }
  }
  if(counts==3) cell_board[x][y] = 1;
  else if (counts<2) cell_board[x][y] = 0;
  else if(counts>3) cell_board[x][y] = 0;
}

//Let's paralellize the life game
// Considering divide with data
void life_game_one (int m,int n, int* row_num,  int * row_begin, int id, int p, int*** split_cells, int *** split_temp)
{
  int x;
  int y;
  for(int k=0; k<m*row_num[id]; k+=1){
    x = k/m+1;
    y = k%n;
    if(x<=m && y<=n)
      life_evaluate(split_cells[id], split_temp[id], x, y);
  }
  //printf("id:%d, p:%d Done!\n", id,p); fflush(stdout);
}

void copy_board (int m, int n, int id, int p, int ** cell_board, int ** temp_board)
{
  int x;
  int y;
  for(int k=id; k<m*n; k+=p){
    x = k/m+1;
    y = k%n+1;
    temp_board[x][y] = cell_board[x][y];
  }
  
}

/*n*****************************************************************************/
