/*
* file: mult_mat.c
* autor: Alexander Marin 2018
*
* Programa que calcula la multiplicación de un vector y una matriz cuadrada
* usando MPI, se define un master que le asigna tareas a los esclavos,
* el maestro se encarga de recolectar los resultados de los esclavos y
* agregarlos al resultado final para su despliegue. La dimensión de
* de la matriz/vector viene dada por el primer argumento del programa,
* el cual debe ser un entero mayor o igual que 1:
*
* Ej con matriz de 15x15:
*
* 	mpiexec -n 8 ./mult_mat.o 15
*
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SUPRESS_PRINTS 	0	// se usa para evitar imprimir resultados
#define ID_MASTER 	0	// identifica el rank master

// tags a utilizar en los mensajes de MPI
enum tags {
	TAG_SLAVE_REQ,	// usano por los esclavos para solicitar una tarea
	TAG_SLAVE_DONE, // usado por los esclavos para indicar tarea terminada
	TAG_TERMINATE,	// usado por el maestro para terminar los demas ranks
	TAG_TASK	// usado por el maestro para asignar una tarea
	};
	

/*
* Imprime matriz en stdin, con dimesiones rows x cols.
*
* @param[in]	mat	Array bidimensional.
* @param[in]	rows	Cantidad de filas.
* @param[in]	cols	Cantidad de columnas.
*/
void print_mat(int **mat, int rows, int cols)
{
	for(int i=0; i<rows; i++)
	{
		printf("[");
		for(int j=0; j<cols; j++)
		{
			printf(" %d ", mat[i][j]);
		}
		printf("]\n");
	}
}


int main(int argc, char *argv[])
{
	// mpi basics
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// necesario para medir el tiempo de ejecucion
	struct timespec start, finish;

	// tiempo de ejecucion
	double elapsed;
	
	int m = 3;	// dimensiones de matriz (mxm)
	int i = 0;	// contador para filas
	int j = 0;	// contador para columnas

	// extrae valor de los argumentos
	m = atoi(argv[1]);
	
	// creacion/inicializacion de matrices
	// los vectores se crean como matrices para mayor facilidad
	
	// crea el vector de entrada
	int **vec = (int **)malloc(1 * sizeof(int *)); 
	vec[0] = (int *)malloc(m * sizeof(int)); 

	// inicializa los elementos del vector
	for (i=0; i<m; i++)
		vec[0][i] = i;
	
	// crea la matriz de entrada
	int **mat = (int **)malloc(m * sizeof(int *)); 
	for (i=0; i<m; i++) 
		mat[i] = (int *)malloc(m * sizeof(int)); 

	// inicializa las entradas de la matriz
	for (i=0; i<m; i++)
		for (j=0; j<m; j++)
			mat[i][j] = i+j;

	// crea la matriz resultado (vector mx1)
	int **result = (int **)malloc(m * sizeof(int *)); 
	for (i=0; i<m; i++) 
		result[i] = (int *)malloc(1 * sizeof(int)); 

	// inicializa las entradas de la matriz
	for (i=0; i<m; i++)
		result[i][0] = -1;

	// contador de filas asignadas
	int curr_row=0;
	
	// contador de operaciones realizadas (multiplicaciones)
	int done_rows=0;
	
	// tag del mensaje recibido
	int tag_msg = 0;
	
	// cantidad de ranks disponibles
	int slave_ranks = size-1;
	
	// buffers para intercambiar datos
	int task_output[2];
	int buf[2];
	
	// setup master
	if (rank==0)
	{
		// inicio timer
		clock_gettime(CLOCK_MONOTONIC, &start);
		
		while(1)
		{
			// revisa por mensajes
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
			// se asigna tarea
			if(status.MPI_TAG == TAG_SLAVE_REQ)
			{
				MPI_Recv(buf, 2, MPI_INT, status.MPI_SOURCE, TAG_SLAVE_REQ, MPI_COMM_WORLD, &status);
				
				
				if(curr_row>=m)
				{
					// si no hay mas trabajo enviar mensaje de terminacion
					MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, TAG_TERMINATE, MPI_COMM_WORLD);
					slave_ranks--;
				}
				else
				{
					// asignar fila
					MPI_Send(&curr_row, 1, MPI_INT, status.MPI_SOURCE, TAG_TASK, MPI_COMM_WORLD);
					curr_row++;
				}
				
			}
			else{
				// se recibe resultado de tarea
				if(status.MPI_TAG == TAG_SLAVE_DONE)
				{
					MPI_Recv(task_output, 2, MPI_INT, status.MPI_SOURCE, TAG_SLAVE_DONE, MPI_COMM_WORLD, &status);
					// se guarda resultado
					result[task_output[1]][0] = task_output[0];
					done_rows++;
					if(done_rows>=m)
						break;
				}
			}
			
			
		}
		
		// terminacion de ranks esclavos
		while (slave_ranks)
		{
			MPI_Recv(buf, 2, MPI_INT, MPI_ANY_SOURCE, TAG_SLAVE_REQ, MPI_COMM_WORLD, &status);
			MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, TAG_TERMINATE, MPI_COMM_WORLD);
			slave_ranks--;
		}
	}
	else // esclavos
	{
		int task_input=0;
		
		while(1)
		{
			// esclavo sin tarea asignada envia pedido de tarea (TAG_REG)
			MPI_Send(NULL, 0, MPI_INT, ID_MASTER, TAG_SLAVE_REQ, MPI_COMM_WORLD);
			
			// recibe respuesta terminacion o tarea
			// revisa por mensajes
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			
			if (status.MPI_TAG==TAG_TASK)
			{
				// realiza tarea
				MPI_Recv(&task_input, 1, MPI_INT, ID_MASTER, TAG_TASK, MPI_COMM_WORLD, &status);
				task_output[0] = 0;
				task_output[1] = task_input;
				for (j=0; j<m; j++)
				{
					task_output[0] += vec[0][j]*mat[j][task_input];
				}
				//sleep(1);
				MPI_Send(&task_output, 2, MPI_INT, ID_MASTER, TAG_SLAVE_DONE, MPI_COMM_WORLD);
			}
			else
			{
				if (status.MPI_TAG==TAG_TERMINATE)
				{
					// terminacion del ranks
					MPI_Recv(buf, 2, MPI_INT, ID_MASTER, TAG_TERMINATE, MPI_COMM_WORLD, &status);
					break;
				}
			}			
		}
	}

	if (rank == 0)
	{
		// fin de timer
		clock_gettime(CLOCK_MONOTONIC, &finish);
		elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
		
		// imprime resultados
		printf("************************************\n");
		if(!SUPRESS_PRINTS)
		{
		printf("Vector de entrada\n");
		print_mat(vec, 1, m);
		printf("\nMatriz de entrada\n");
		print_mat(mat, m, m);
		printf("\nResultado\n");
		print_mat(result, m, 1);
		printf("\n");
		}
		printf("Tiempo de ejecucion: %.4f\n", elapsed); 
	}
	MPI_Finalize();
	return 0;
}
