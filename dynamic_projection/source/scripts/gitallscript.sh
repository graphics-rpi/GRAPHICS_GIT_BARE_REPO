#!/bin/bash

./buildallscript.sh

git pull
git push
ssh torch    "cd GIT_CHECKOUT ; git pull && make -C dynamic_projection/source/tabletop/build/"
ssh chainsaw "cd GIT_CHECKOUT ; git pull && make -C dynamic_projection/source/tabletop/build/"