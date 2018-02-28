ON UBUNTU 12.04
NEED TO GET A NEWER g++ (to use c++11)

http://mortenvp.com/installing-a-newer-gccg-on-ubuntu-12-04-lts/

RUN THESE COMMANDS:

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-5 g++-5

THEN HOPEFULLY

g++-5 -v

says you've got g++5.4 or something :)
