/*
* file: calc_pi.c
* autor: Alexander Marin 2018
*
* Programa que realiza el calculo de pi usando la biblioteca de pthreads.
*
* Para realizar la aproximación utiliza la integral:
*
* pi = integral de 4/(x^2 + 1) desde 0 a 1
*
* Dicho valor se aproxima mediante la regla del trapecio, la cantidad de
* trapecios se divide entre varias instancias de una funcion que se ejecutan
* de manera paralela, al final de cada función estas realizan el commit
* de sus calculos a la suma total y luego terminan la ejecución. La funcion
* principal espera la terminación de cada uno de estos hilos e imprime el
* resultado final.
*/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

// mutex para conseguir el orden en los commits a la suma total
pthread_mutex_t sum_mutex;

// variable global con el valor de pi obtenido como una suma de trapecios
double sum_series;

// estructura definita para pasarle parametros a la funcion que calcula
// la suma de los trapecios
struct args {
    long n0; // numero del trapecio inicial
    long n1; // numero del trapecio final
    double delta_x; // ancho del trapecio
};


/*
* Funcion a integrar para la aproximacion de pi.
*
* @param[in] x	Preimagen.
* @return double	Imagen.
*/
double f(double x)
{
	return 4./(1+(x*x));
}


/*
* Calcula el area dada por un conjunto de trapecios.
*
* @param[in] args	Estructura predefinida para el paso de parametros.
* @param[out] sum_series	Agrega la suma calculada a la suma total.
*/
void *t_fun(void *input) {
	
	// extrae parametros de la estructura
	long n0 = ((struct args*)input)->n0;
	long n1 = ((struct args*)input)->n1;
	double delta_x = ((struct args*)input)->delta_x;
	
	// variable local para realizar la suma de los trapecios
	double acc = 0;

	// valor inicial de x
	double x = n0*delta_x;
	
	// suma de trapecios
	for(int j = n0; j<n1; j++ )
	{
		x = j*delta_x;
		acc += ( (f(x)+f(x+delta_x))/2. ) * delta_x;
		x += delta_x;
	}
	
	// usa mutex para hacer commit de la suma
	pthread_mutex_lock(&sum_mutex);
	sum_series += acc;
	pthread_mutex_unlock(&sum_mutex);
	
	// termina thread
	pthread_exit(0);
	}


int main()
{
	// necesario para medir el tiempo de ejecucion
	struct timespec start, finish;

	// tiempo de ejecucion
	double elapsed;

	// cantidad de bloques(trapecios)
	double n_block = 1.;

	// delta x (ancho de trapecio)
	double d = 1./100000000.;

	// cantidad de threads
	int num_threads = 1;

	// inicializa suma de trapecios (valor de pi)
	sum_series = 0.;

	// lee la cantidad de threads de stdin
	printf("***********************************************\n");
	printf("Digite la cantidad de threads: \n");
	scanf("%d", &num_threads);

	// array con threads
	pthread_t *threads_arr;

	// hace espacio en memoria para la cantidad de hilos solicitados
	threads_arr = malloc(sizeof(pthread_t) * num_threads);

	// calcula cantidad de trapecios
	n_block = (1./d);

	// inicio timer
	clock_gettime(CLOCK_MONOTONIC, &start);

	// inicia la ejecucion de los hilos
	for(int k=0; k<num_threads; k++)
	{
		// parametros para el hilo a ejecutar
		struct args *a = (struct args *)malloc(sizeof(struct args));
		a->n0 = (long)round((double)k*n_block/(double)num_threads);
		a->n1 = (long)round((double)(k+1)*n_block/(double)num_threads);
		a->delta_x = d;
		// inicia ejecución del hilo
		pthread_create(&threads_arr[k], NULL, t_fun, (void *)a);
	}

	// almacena valor de retorno de cada hilo
	int code_t;

	// espera por la finalizacion del los hilos
	for(int k=0; k<num_threads; k++)
	{
		pthread_join(threads_arr[k], (void **)&code_t);
		/*
		if((code_t)==0)
			printf("Success: thread %d\n", k);
		else
			printf("Error: thread %d\n", k);
		*/
	}

	// fin de timer
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	// imprime resultados
	printf("# threads=%d \ntiempo=%.4f\n",
		num_threads,
		elapsed);

	printf("pi calculado=\t%.16f\n"
		"pi objetivo=\t%.16f\n"
		"error=\t\t%.16f\n",
		sum_series, M_PI, fabs(sum_series-M_PI));

	return 0;
}
