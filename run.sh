#!/bin/bash
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:lib mpirun -np 4 test_p
