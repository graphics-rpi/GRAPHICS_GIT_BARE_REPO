#!/bin/bash                                                                     

mpirun --mca btl_tcp_if_include eth0 --hostfile ~/hosts -np 1 mpi_renderer
