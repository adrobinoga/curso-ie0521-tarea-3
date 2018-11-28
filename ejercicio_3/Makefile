calc_pi.o: calc_pi.c
	gcc -pthread  calc_pi.c -o calc_pi.o -lm
	
run: calc_pi.o
	./calc_pi.o

test: calc_pi.o
	./test.sh
