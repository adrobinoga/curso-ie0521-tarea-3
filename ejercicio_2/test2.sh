#!/bin/bash
for i in {2..8}
do
	echo "########################################"
	echo "PRUEBA, m=10, ranks=$i, slaves=$[i-1]"
	mpiexec -n $i ./mult_mat.o 10
	echo ""
done
