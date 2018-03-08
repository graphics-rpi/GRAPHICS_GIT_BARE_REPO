cd ~/Checkout/rendering_test_with_sockets
cvs up -d
#mpicxx LiExample.cpp shader.cpp  -O3 -I/usr/local/include/boost-1_39 --std=c++0x -Icommon/ -Lcommon/  -lboost_thread -lboost_regex -DCONTRAPTION -lX11 -lGL -lGLU -lGLEW  -lXmu -lXi -lXrandr -lglut -lSDL -o mpi_renderer -DCOMPRESSED_TEX -lz -DWRITE_FILE
cvs commit
ssh torch ~/Checkout/rendering_test_with_sockets/Make.sh
sleep 5
ssh chainsaw ~/Checkout/rendering_test_with_sockets/Make.sh

#scp mpi_renderer torch:~/Checkout/rendering_test_with_sockets/
#scp mpi_renderer chainsaw:~/Checkout/rendering_test_with_sockets/




