/*
 * The Game of Life
 *
 * a cell is born, if it has exactly three neighbours 
 * a cell dies of loneliness, if it has less than two neighbours 
 * a cell dies of overcrowding, if it has more than three neighbours 
 * a cell survives to the next generation, if it does not die of loneliness 
 * or overcrowding 
 *
 * In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
 * The game plays a number of steps (given by the input), printing to the screen each time.  'x' printed
 * means on, space means off.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

// descomente para depurar o código
//#define DEBUG_MODE 1

typedef unsigned char cell_t; 

// Alloca um tabuleiro quadrado de tamanho 'size'
cell_t *allocate_board(int sizex, int sizey) 
{
	cell_t *board = (cell_t *) calloc(sizex*sizey, sizeof(cell_t));
	return board;
}

void free_board(cell_t *board) 
{
	free(board);
}

// retorna o número de células 'vivas' adjacentes à célula (i,j)
int adjacent_to(cell_t *board, int sizex, int sizey, int i, int j) 
{
	int	k, l, count=0;

  // não precisamos mais destas checagens devido as ghost rows
/* 
	int sk = (i>0) ? i-1 : i;
	int ek = (i+1 < sizey) ? i+1 : i;
*/
	int sl = (j>0) ? j-1 : j;
  int el = (j+1 < sizex) ? j+1 : j;


	for (k=i-1; k<=i+1; k++)
		for (l=sl; l<=el; l++)
			count+=board[k*sizex+l];
	count-=board[i*sizex+j];


	return count;
}

// percore as células e aplica as regras do jogo
void play(cell_t *board, cell_t *newboard, int sizex, int sizey) 
{
	int	i, j, a;
	/* para cada célula do tabuleiro original (board), constroi outro com as regras
   * aplicadas (newboard) */
	for (i=0; i<sizey; i++)
		for (j=0; j<sizex; j++) {
			a = adjacent_to(board, sizex, sizey, i, j);  // checa quantas células vivas
                                            // estão próximas da atual
      // regras
			if (a == 2) newboard[i*sizex+j] = board[i*sizex+j];
			if (a == 3) newboard[i*sizex+j] = 1;
			if (a < 2) newboard[i*sizex+j] = 0;
			if (a > 3) newboard[i*sizex+j] = 0;
		}
}

/* imprime o tabuleiro na saída de erro */
void print(cell_t *board, int sizex, int sizey) 
{
	int	i, j;
	/* para cada linha */
	for (j=0; j<sizey; j++) {
		/* imprime o estado de cada coluna... */
		for (i=0; i<sizex; i++) 
			fprintf(stderr, "%c", board[j*sizex+i] ? 'x' : ' ');
		fprintf(stderr, "\n");
	}
}

/* lê arquivo com o tabuleiro inicial */
void read_file(char *file, cell_t **board, int *sizex, int *sizey) 
{
	int	i, j;

	FILE    *f;
	f = fopen(file, "r");
  if (f == NULL) 
  {
    perror(file);
    exit(-1);
  }
  
  fprintf(stdout, "Lendo arquivo... ");

	fscanf(f,"%d", sizex);
	while (fgetc(f) != '\n') /* no-op */;
	fscanf(f,"%d", sizey);
	while (fgetc(f) != '\n') /* no-op */;
  
  fprintf(stdout, " board of size %dx%d\n", *sizex, *sizey);
	
  *board = allocate_board(*sizex,*sizey);

// lê tabuleiro
	char	*s = (char *) malloc(*sizex+10);
	char c;
	for (j=0; j<*sizey; j++) {
		/* lê string */
		fgets (s, *sizex+10,f);
		/* copia string para o tabuleiro */
		for (i=0; i<*sizex; i++)
		{
			(*board)[j* *sizex+i] = s[i] == 'x';
		}
	}

  free(s);
  fprintf(stdout, "ok\n");
  fclose(f);
}

int main(int argc, char *argv[])
{
  struct timeval start, stop;
	
  /* Linha de comando */
  if (argc <=2) {
    fprintf(stdout, "Entrada inválida: use %s <arquivo> <passos>\n", argv[0]);
    exit(-1);
  }
  
  int sizex, sizey, steps = strtol(argv[2], NULL, 10);
  cell_t *tabuleiro;
  

  // Estamos assumindo que o tabuleiro é dividido perfeitamente entre os nodos
  MPI_Init(&argc, &argv);
  
  int comm_sz, my_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 


  // Apenas rank 0 lê arquivo com o tabuleiro
  if (my_rank == 0) {
    read_file(argv[1], &tabuleiro, &sizex, &sizey);
    sizey = sizey/comm_sz;
  }

  // rank 0 informa as dimensões do tabuleiro para os demais nodos
  MPI_Bcast(&sizex, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&sizey, 1, MPI_INT, 0, MPI_COMM_WORLD);



  // Antes de fazer o scatter, aloca área que vai receber a fatia do tabuleiro
  // para o nodo correspondente
  // usamos uma linha a mais (ghost cells) acima e abaixo
  cell_t *prev = allocate_board(sizex, sizey+2);
	cell_t *tmp;
	int i,j;
  
  gettimeofday(&start, NULL);


  // Note que o buffer que recebe os dados já pula a primeira linha (ghost cells)
  MPI_Scatter(tabuleiro, sizex*sizey, MPI_UNSIGNED_CHAR, 
              prev+sizex, sizex*sizey, MPI_UNSIGNED_CHAR,
              0, MPI_COMM_WORLD);
     

  // todos os nodos alocam memória para cópia do tabuleiro
  cell_t *next = allocate_board(sizex, sizey+2);


  // laço principal do jogo
	for (i=0; i<steps; i++) {

    // antes de jogar a rodada atual, é necessário comunicar as bordas
    // note que precisamos de 2 nodos no mínimo para esse código funcionar

    MPI_Status status;

    // rank 0 comunica apenas com o sucessor (rank 1)
    if (my_rank == 0) {
#ifdef DEBUG_MODE
      printf("Rank %d enviando %d elementos\n", my_rank, sizex);
      printf("Conteúdo: ");
      for (int i=0; i<sizex; i++)
        printf("%c", *(prev+(sizex*sizey)+i) ? 'x' : ' ');
      printf("\n");
#endif
      // envia última linha para rank 1
      MPI_Send(prev+(sizex*sizey), sizex, MPI_UNSIGNED_CHAR, my_rank+1, 0, MPI_COMM_WORLD);
  
      // recebe primeira linha do rank 1 (vai ser a última 'ghost row' nossa)
      MPI_Recv(prev+(sizex*sizey)+sizex, sizex, MPI_UNSIGNED_CHAR, my_rank+1, 0, MPI_COMM_WORLD, &status);
      
#ifdef DEBUG_MODE
      int count;
      MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);
      printf("Rank %d received: %d uchars do rank %d\n", my_rank, count, status.MPI_SOURCE);
      printf("Mensagem: ");
      for (int i=0; i<sizex; i++)
        printf("%c", *(prev+(sizex*sizey)+sizex+i) ? 'x' : ' ');
      printf("\n");
#endif     
    }
    // último rank comunica apenas com o antecessor
    else if (my_rank == comm_sz-1) {
#ifdef DEBUG_MODE
      printf("Rank %d enviando %d elementos\n", my_rank, sizex);
      printf("Conteúdo: ");
      for (int i=0; i<sizex; i++)
        printf("%c", *(prev+sizex+i) ? 'x' : ' ');
      printf("\n");
#endif      
      // envia primeira linha para rank anterior
      MPI_Send(prev+sizex, sizex, MPI_UNSIGNED_CHAR, my_rank-1, 0, MPI_COMM_WORLD);

      // recebe última linha do rank anterior (primeira ghost row nossa)
      MPI_Recv(prev, sizex, MPI_UNSIGNED_CHAR, my_rank-1, 0, MPI_COMM_WORLD, &status);

#ifdef DEBUG_MODE
      int count;
      MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);
      printf("Rank %d received: %d uchars do rank %d\n", my_rank, count, status.MPI_SOURCE);
      printf("Mensagem: ");
      for (int i=0; i<sizex; i++)
        printf("%c", *(prev+i) ? 'x' : ' ');
      printf("\n");
#endif
    }
    // todos os outros nodos comunicam com antecessor e sucessor
    else {
      // envia primeira linha para rank anterior
      MPI_Send(prev+sizex, sizex, MPI_UNSIGNED_CHAR, my_rank-1, 0, MPI_COMM_WORLD);
      // envia última linha para rank sucessor
      MPI_Send(prev+(sizex*sizey), sizex, MPI_UNSIGNED_CHAR, my_rank+1, 0, MPI_COMM_WORLD);

      // recebe última linha do rank anterior (primeira ghost row nossa)
      MPI_Recv(prev, sizex, MPI_UNSIGNED_CHAR, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // recebe primeira linha do rank sucessor ((última ghost row nossa)
      MPI_Recv(prev+(sizex*sizey)+sizex, sizex, MPI_UNSIGNED_CHAR, my_rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // lê o tabuleiro 'prev' e escreve no 'next'
		play(prev+sizex,next+sizex,sizex,sizey);

    // permuta os tabuleiros
		tmp = next;
		next = prev;
		prev = tmp;
	}
  
  if (my_rank == 0)
    tabuleiro = allocate_board(sizex,sizey*comm_sz);

  MPI_Gather(prev+sizex, sizex*sizey, MPI_UNSIGNED_CHAR, 
              tabuleiro, sizex*sizey, MPI_UNSIGNED_CHAR,
              0, MPI_COMM_WORLD);

  gettimeofday(&stop, NULL); 

  if (my_rank == 0) {
  	print(tabuleiro,sizex,sizey*comm_sz);
    free(tabuleiro);
	
    double t = (((double)(stop.tv_sec)*1000.0  + (double)(stop.tv_usec / 1000.0)) - \
                   ((double)(start.tv_sec)*1000.0 + (double)(start.tv_usec / 1000.0)));

    fprintf(stdout, "Tempo decorrido = %g ms\n", t);
  }
  

  free_board(prev);
	free_board(next);
  
  MPI_Finalize();
}
