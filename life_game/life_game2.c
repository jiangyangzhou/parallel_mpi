# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
#include <string.h>

int main ( int argc, char *argv[] );
void life_evaluate(int *cell_board, int *temp_board, int n, int x, int y);
void life_game_one ( int m,int n, int row_num, int row_begin, int id, int p, int* sub_cells, int * sub_temp );
void print_board_l (int * temp_board, int m, int n);

void sysn_temp_board(int id, int p, int n, int* sub_cells, int * sub_temp, int row_num);
void copy_board (int m, int n, int id, int p, int * cell_board, int * temp_board);
char *getfileall(char *fname);
int* get_csv(char*fname, int* m, int *n);
void to_csv(char* fname, int * cell_board, int m, int n);
/******************************************************************************/
int main ( int argc, char *argv[] )
{
  int id;
  int p;

  int ierr;
  int m = 0;
  int n = 0;
  int num_j;
  int num_k;
  int *cell_board;
  //int *temp_board;
  double wtime;
  double wtime2;
  double wtime3;
  if(argc==3){
    num_j = atoi(argv[1]);
    num_k = atoi(argv[2]);
  }
  else printf("Please give iterations j and print frequency k.");

  cell_board = get_csv("board.csv", &m, &n);


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
  MPI_Barrier(MPI_COMM_WORLD);
  if ( id == 0 )
  {
    printf ( "\n" );
    printf ( "PRIME_MPI\n" );
    printf ( "  C/MPI version\n" );
    printf ( "\n" );
    printf ( "  An MPI program to perfrom life game.\n" );
    printf ( "  The number of processes is %d\n", p );

  }

  int row_num;
  int row_begin;
  int *row_num_list;
  //int *row_begin_list;

  if ( id == 0 )
  {
    wtime = MPI_Wtime ( );
    int row = m / p;
    printf("m:%d,n%d, p:%d, row:%d", m, n, p, row);
    int rem = m % p;
    int r = 0;  

    row_num_list = malloc(sizeof(int)*p);
    //row_begin_list = malloc(sizeof(int)*p);
    for(int i=0;i<p;i++){
      //row_begin_list[i] = r;
      if(id<rem){
          row_num_list[i] = row+1;
          r +=row+1;
        }
      else {row_num_list[i] = row; r += row;}
    if(i!=0){
      ierr = MPI_Send(&row_num_list[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      //ierr = MPI_Send(&row_begin_list[i], 1, MPI_INT,i, 0, MPI_COMM_WORLD);
    }
    row_num = row_num_list[0];
    //row_begin = 0
    }
  }
  else{
    ierr = MPI_Recv(&row_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //ierr = MPI_Recv(&row_begin, 1, MPI_INT, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

    //MPI_Bcast()
  int *sub_cells = malloc(sizeof(int)*row_num*(n+2));
  int *sub_temp = malloc(sizeof(int)*(row_num+2)*(n+2));

  if(id==0){
    int r = 1;  
    for(int i=0; i<p; i++){
      if(i!=0){
        ierr = MPI_Send(&cell_board[r*(n+2)], row_num_list[i]*(n+2), MPI_INT, i, 0, MPI_COMM_WORLD);
        //ierr = MPI_Send(split_temp[i], (row_num[i]+2)*(n+2), MPI_INT, i,0, MPI_COMM_WORLD);
      }
      else{
        for(int k=0;k<row_num;k++){
          for(int j=0;j<n+2;j++)
            sub_cells[k*(n+2)+j] = cell_board[(k+1)*(n+2)+j];
        }
      }
      r+=row_num_list[i];
    }
  }
  else{
    ierr = MPI_Recv(sub_cells, row_num*(n+2), MPI_INT, 0, 0, MPI_COMM_WORLD,
              MPI_STATUS_IGNORE);
  }


  sysn_temp_board(id, p, n, sub_cells, sub_temp, row_num );
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);
  int * buff_num;
  int * dis_num;
  if(id==0){
    buff_num = malloc(sizeof(int)*p);
    dis_num = malloc(sizeof(int)*p);
    int r=0;
    for(int i=0;i<p;i++){
      buff_num[i] = row_num_list[i]*(n+2);
      dis_num[i] = r;
      r+=row_num_list[i]*(n+2);
    }
    //printf("xx0print s, %d, %d, %d,\n", dis_num[1],buff_num[1], row_num_list[1]);
  }

  for(int j=0; j<num_j; j++){
    if (j%num_k == num_k-1){
      ierr = MPI_Gatherv(sub_cells, row_num*(n+2), MPI_INT, &cell_board[n+2], buff_num, dis_num, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Barrier(MPI_COMM_WORLD);
      if(id==0){
        print_board_l(cell_board, m, n);
        fflush(stdout);
      }
    }
    life_game_one (m, n, row_num, row_begin, id, p, sub_cells, sub_temp);
    MPI_Barrier(MPI_COMM_WORLD);
    sysn_temp_board(id, p, n, sub_cells, sub_temp, row_num );
    MPI_Barrier(MPI_COMM_WORLD);
  }
  ierr = MPI_Gatherv(sub_cells, row_num*(n+2), MPI_INT, &cell_board[n+2], buff_num, dis_num, MPI_INT, 0, MPI_COMM_WORLD);
  wtime2 = MPI_Wtime ( ) - wtime;
  if(id==0)
    to_csv("life.out1", cell_board, m, n);
  
  MPI_Barrier(MPI_COMM_WORLD);
  if ( id == 0 )
  {
    wtime3 = MPI_Wtime ( ) - wtime;
    printf("\nTotal time:%f, %f\n", wtime2, wtime3);
  }
  /*
    Terminate MPI.
  */

 free(sub_cells);
 free(sub_temp);
ierr = MPI_Finalize();
free(cell_board);

//printf("Finalize ok\n");

  return 0;
}


// This function read csv file and initialize cell board
int* get_csv(char* fname, int* m, int* n)
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
  int *matrix = (int*)malloc(sizeof(int)*((*m)+2)*(*n+2)); 
  for(int i=0; i<(*m)+2; i++){
    for(int j=0; j<(*n)+2; j++)
      matrix[i*((*n)+2)+j]=0;
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
      matrix[(i+1)*(*n+2)+j+1] = atoi(head);
      //printf("ma[i][j]:%d\n",matrix[i][j] );
      num=head;
      p++;
      if(*p=='\n'){i+=1; j=-1;break;}
    }
  }
  //  printf("%d,\n", matrix[0][0]);fflush(stdout);

  return matrix;
}


void to_csv(char* fname, int * cell_board, int m, int n){
    FILE *fp;
  if ((fp=fopen(fname,"w"))==NULL){
      printf("打开文件%s错误\n",fname);
      return;
  }
  printf("n:%d",n);
  for(int i=0;i<m;i++){
    for(int j=0;j<n-1;j++){
      fprintf(fp, "%d,", cell_board[(i+1)*(n+2)+j+1]);
    }
    fprintf(fp, "%d\n",cell_board[(i+1)*(n+2)+n]);
  }
}

void print_board_l(int *cell_board, int m, int n){
  for(int i=0; i<=m+1; i++){
    for(int j=0; j<=n+1; j++){
      printf("%d", cell_board[i*(n+2)+j]);
    }
    printf("\n");
  }
  printf("\n");

}

void sysn_temp_board(int id, int p, int n, int* sub_cells, int * sub_temp, int row_num){
  for(int i=0;i<row_num; i++){
    for(int j=0; j<=n+1; j++){
      sub_temp[(i+1)*(n+2)+j] = sub_cells[i*(n+2)+j];    
    }
  }
  for(int j=0; j<=n+1; j++){
      sub_temp[j] = 0;  
      sub_temp[(row_num+1)*(n+2)+j] = 0;    
    }
  if(id!=p-1)
    MPI_Send(&sub_cells[(row_num-1)*(n+2)], (n+2), MPI_INT, id+1, 0, MPI_COMM_WORLD);
  if(id!=0){
    MPI_Recv(&sub_temp[0], (n+2), MPI_INT, id-1, 0, MPI_COMM_WORLD,
              MPI_STATUS_IGNORE);
    MPI_Send(&sub_cells[0], (n+2), MPI_INT, id-1, 0, MPI_COMM_WORLD);
  }
  if(id!=p-1)
    MPI_Recv(&sub_temp[(row_num+1)*(n+2)], (n+2), MPI_INT, id+1, 0, MPI_COMM_WORLD,
              MPI_STATUS_IGNORE);
}
/******************************************************************************/
//Ok we need to write a life game 
//this is a function to evaluate the cell should be live or dead 
//so Give cells board, and its coordinate (x,y), it check neighbors and give the decision 
// 3 neibors: live | 2 neibors no change | <=1 neibors, dead
//Let's do it !
void life_evaluate(int *cell_board, int *temp_board,int n, int x, int y){
  int counts=0;
  for(int i=-1; i<=1; i++){
    for(int j=-1; j<=1; j++){
      if( !(i==0 && j==0))
        counts+=temp_board[(x+1+i)*(n+2)+y+j];
    }
  }
  if(counts==3) cell_board[x*(n+2)+y] = 1;
  else if (counts<2) cell_board[x*(n+2)+y] = 0;
  else if(counts>3) cell_board[x*(n+2)+y] = 0;
}
//Let's paralellize the life game
// Considering divide with data
void life_game_one (int m,int n, int row_num, int row_begin, int id, int p, int* sub_cells, int* sub_temp)
{
  int x;
  int y;
  for(int k=0; k<n*row_num; k+=1){
    x = k/n;
    y = k%n+1;
    if(x<=m && y<=n)
      life_evaluate(sub_cells, sub_temp, n, x, y);
  }

}

void copy_board (int m, int n, int id, int p, int * cell_board, int * temp_board)
{
  int x;
  int y;
  for(int k=id; k<m*n; k+=p){
    x = k/m+1;
    y = k%n+1;
    temp_board[x*(n+2)+y] = cell_board[x*(n+2)+y];
  }
}

/*n*****************************************************************************/
