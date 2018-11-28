# Tarea 3

Estudiante: Alexander Marín
Curso: Estructuras de Computadoras II


## Ejercicio 1

## Ejercicio 2

Para compilar y correr el programa se debe usar:

	$ cd ejercicio_2
	$ make
	$ mpiexec -n <cantidad de ranks> ./mult_mat.o <dimension de la matriz>

Se puede correr una prueba predefinida con:

	$ make run

Que en realidad ejecuta el programa usando 8 ranks y una matriz de 15x15

	mpiexec -n 8 ./mult_mat.o 15

Existen dos scripts en bash adiconales `test.sh` y `test2.sh`, los cuales se ejecutan con:

	$ make test

y

	$ make test2

Respectivamente, el primero ejecuta el programa usando 2 ranks y varía la cantidad filas de 1 a 10. Mientras que el segundo script varía la cantidad de ranks de 2 a 8, usando 10 filas, se requieren al menos 2 ranks ya que se deben tener como mínimo 1 master y 1 slave.

Para verificar los resultados se puede utilizar el script de python `example.py`

	$ python3 example.py

Dicho script usa numpy para imprimir el resultado de la multiplicación para matrices con tamaño desde 1x1 hasta 10x10.

## Ejercicio 3

Para compilar y correr el programa se debe usar:

	$ cd ejercicio_3
	$ make run

El programa solicita la cantidad de threads a utilizar y luego se presiona ENTER:

	$ make run
	gcc -pthread  calc_pi.c -o calc_pi.o -lm
	./calc_pi.o
	***********************************************
	Digite la cantidad de threads: 
	2
	# threads=2 
	tiempo=1.4140
	pi calculado=	3.1415926535891892
	pi objetivo=	3.1415926535897931
	error=		0.0000000000006040

El programa imprime el tiempo de ejecución y compara el valor de pi obtenido con el valor real (hasta 16 decimales).

También es posible correr una prueba predefinida con:

	$ make test

Esta prueba corre el programa usando 1, 2, 3, 4 y 5 threads.


