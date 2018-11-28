#!/bin/bash
for i in {1..10}
do
	echo "########################################"
	echo "PRUEBA, m=$i, ranks=2"
	mpiexec -n 2 ./mult_mat.o $i
	echo ""
done


