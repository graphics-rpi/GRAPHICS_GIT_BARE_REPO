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
CC              = g++ -g -O3 -Wall -pedantic -fPIC -D__LINUX__ -std=c++0x
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), FREEBSD)
# FreeBSD
CC              = g++ -g -O3 -Wall -pedantic -D__FREEBSD__
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), Darwin)
# Mac OS X
CC              = g++ -g -Wall -pedantic 
LIBS            = -framework GLUT -framework OpenGL
else
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
# Cygwin with native Windows libraries
CC              = g++ -g -O3 -Wall -pedantic 
INCLUDE_PATH    = -I/usr/include/w32api -I/usr/include/opengl
LIB_PATH        = -L/usr/lib/w32api
LIBS            = -lm -lopengl32 -lglu32 -lglut32 
else
$(error "Unknown OS: " $(UNAME))
endif
endif
endif
endif

# ===============================================================

SRCS	= main.cpp ../../../../remesher/src/matrix.cpp munkres.cpp tracker.cpp planar_calibration.cpp \
	../../applications/paint/text.cpp \
	../../ir_tracking/ir_data_point.cpp \
	../../../../remesher/src/utils.cpp
OBJS	= $(SRCS:.cpp=.o)
EXE	= planar_interpolation_calibration
EXE2	= socket_server
EXE3	= socket_client

# ===============================================================
# targets

.PHONY: all depend clean

all: depend $(EXE) $(EXE2) $(EXE3)

depend:
	$(CC) $(INCLUDE_PATH) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak $(OBJS)  $(EXE) $(EXE).exe $(EXE2) $(EXE3) Makefile.depend

# ===============================================================
# compilation rules

$(EXE): Makefile $(OBJS)
	$(CC) $(INCLUDE_PATH) -o $@ $(OBJS) $(LIB_PATH) $(LIBS) 

$(EXE2): $(EXE2).cpp socket_helper.h
	$(CC) $(EXE2).cpp -o $(EXE2)

$(EXE3): $(EXE3).cpp socket_helper.h
	$(CC) $(EXE3).cpp -o $(EXE3)

.cpp.o: Makefile
	$(CC) $(INCLUDE_PATH) $< -c -o $@

# ===============================================================

-include Makefile.depend
