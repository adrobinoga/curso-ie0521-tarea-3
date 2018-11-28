mult_mat.o: mult_mat.c
	mpicc mult_mat.c -o mult_mat.o

run: mult_mat.o
	mpiexec -n 8 ./mult_mat.o 15
	
test: mult_mat.o
	./test.sh

test2: mult_mat.o
	./test2.sh
