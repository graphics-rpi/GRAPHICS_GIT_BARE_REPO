
mpicxx main.cpp shader.cpp  -O3 --std=c++0x -I../../common/ -L../../common/  -L/usr/lib64/ -lboost_thread-mt -lboost_regex -DCONTRAPTION -lX11 -lGL -lGLU -lGLEW  -lXmu -lXi -lXrandr -lglut -o mpi_renderer -DCOMPRESSED_TEX -lz -DWRITE_FILE -DMPI_DEFINED

# -I/usr/local/include/boost-1_39 
#scp mpi_renderer torch:/home/grfx/GIT_CHECKOUT/dynamic_projection/source/tabletop/remote_renderer
#scp mpi_renderer chainsaw:/home/grfx/GIT_CHECKOUT/dynamic_projection/source/tabletop/remote_renderer




