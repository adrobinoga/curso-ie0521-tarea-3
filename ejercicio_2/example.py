#!/usr/bin/python3.5

import numpy as np

for n in range(16):
    print("########################################################")
    print("m=%d" % n)
    
    a = np.arange(0,n)
    b = np.fromfunction(lambda x,y: x+y,(n,n))
    
    print("Vector entrada")
    print(a)
    print("Matriz de entrada")
    print(b)
    print("Salida")
    print((a@b).reshape((-1, 1)))
