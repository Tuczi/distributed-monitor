#!/bin/bash
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:lib /usr/lib64/openmpi/bin/mpirun -np 4 main
