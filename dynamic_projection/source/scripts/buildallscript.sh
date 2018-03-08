#!/bin/bash

pushd ../../..
make -C remesher/build/ && 
make -C dynamic_projection/source/common/ && 
make -C lsvo/build && 
make -C dynamic_projection/source/tabletop/build/ && 
make -C dynamic_projection/build/ 
popd

