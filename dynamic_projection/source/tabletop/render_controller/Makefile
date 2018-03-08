EXECUTABLES =  render_controller
all:  $(EXECUTABLES)

CFLAGS= --std=c++0x -O3 -I../../common/  -lX11 -lGL -lGLU -DCOMPRESSED_TEX

render_controller: render_controller.cpp TextureMapMap.hpp sockets.h 
	g++ $(CFLAGS)  -g -o $@ $< -lGL -lGLU -lglut -lz 
#-lpthread 
#	sudo chown render_controller
#	sudo chmod o+x render_controller
#	sudo chmod o+s render_controller

clean:
	rm -rf core *.o $(EXECUTABLES)
