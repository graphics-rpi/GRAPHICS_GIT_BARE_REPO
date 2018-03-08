SHELL = /bin/bash

# ===============================================================

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
UNAME := LINUX
else 
ifeq ($(UNAME), FreeBSD)
UNAME := FREEBSD
endif
endif

ifeq ($(UNAME), LINUX)
# Linux
#CC              = g++ -g -O3 -Wall -pedantic -D__LINUX__ -std=c++0x -DMYNDEBUG -DNDEBUG
#CC              = g++ -g -O3 -Wall -pedantic -D__LINUX__ -std=c++0x -DNDEBUG
CC              = g++ -g -O3 -Wall -pedantic -D__LINUX__ -std=c++0x 
INCLUDE_PATH    = -I/usr/X11R6/include -I/usr/local/include -I../common/
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib -L../common/
LIBS            = -lm -lGL -lGLU -lglut -lcommon -lrt -lpthread
else
ifeq ($(UNAME), FREEBSD)
# FreeBSD
CC              = g++ -g -O3 -Wall -pedantic -D__FREEBSD__
INCLUDE_PATH    = -I/usr/X11R6/include -I/usr/local/include -I../common/
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), Darwin)
# Mac OS X
# the flag  __APPLE__  is defined!
CC              = g++ -g -Wall -pedantic
INCLUDE_PATH    = -I../common/
LIBS            = -framework GLUT -framework OpenGL
else
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
# Cygwin with native Windows libraries
CC              = g++ -g -O3 -Wall -pedantic -std=c++0x -D_WIN32
INCLUDE_PATH    = -I/usr/include/w32api -I/usr/include/opengl -I../common/
LIB_PATH        = -L/usr/lib/w32api
LIBS            = -lm -lopengl32 -lglu32 -lglut32 
else
$(error "Unknown OS: " $(UNAME))
endif
endif
endif
endif

# ===============================================================

SRCS	= led_pixel_finder.cpp viewer.cpp ir_data_point.cpp
#SRCS	= example.cpp
OBJS	= $(SRCS:.cpp=.o)
EXE	= led_pixel_finder

# ===============================================================
# targets

.PHONY: all depend clean trail_merger

all: depend $(EXE) trail_merger

depend:
	$(CC) $(INCLUDE_PATH) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak $(OBJS) $(EXE) $(EXE).exe Makefile.depend

trail_merger:
	g++ -o trail_merger trail_merger.cpp





#EXECUTABLES = led_pixel_finder
#
#TRACKING = led_pixel_finder live_led_finder live_matcher live_tracker tracking_display
#all:  $(EXECUTABLES)
#tracking: $(TRACKING)
##CFLAGS= --std=c++0x -O3 -finline-limit=200000 -I../common/ -L../common/
#CFLAGS= -O3 -finline-limit=200000 -I../common/ -L../common/
##CFLAGS= -g --std=c++0x -O0 -I../../common/ -L../../common/
#
#led_pixel_finder: led_pixel_finder.cpp FindLeds.hpp#
#	g++ $(CFLAGS) -o $@ $< -lGL -lGLU -lglut -lcommon -lrt -lpthread
#
#clean:#
#	rm -rf core *.o $(EXECUTABLES)


# ===============================================================
# compilation rules

$(EXE): Makefile $(OBJS)
	$(CC) $(INCLUDE_PATH) -o $@ $(OBJS) $(LIB_PATH) $(LIBS) 

.cpp.o: Makefile
	$(CC) $(INCLUDE_PATH) $< -c -o $@

# ===============================================================

-include Makefile.depend

