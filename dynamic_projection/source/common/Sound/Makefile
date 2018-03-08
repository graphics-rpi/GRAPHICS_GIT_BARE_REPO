# CSCI-4530/6530 Advanced Computer Graphics
# Simple Makefile for Homework 0
# for g++ on unix, osx, or cygwin platforms

SHELL = /bin/bash

# ===============================================================

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
UNAME := UNIX
else 
ifeq ($(UNAME), FreeBSD)
UNAME := UNIX
endif
endif

ifeq ($(UNAME), UNIX)
# Linux or FreeBSD
CC              = g++ -g -O3 -Wall -pedantic -lalut -w
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), Darwin)
# Mac OS X
CC		= g++ -g -Wall -pedantic
LIBS		= -framework GLUT -framework OpenGL
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

# ===============================================================

SRCS	= Main.cpp Source.cpp Listener.cpp SoundHandler.cpp
OBJS	= $(SRCS:.cpp=.o)
EXE	= soundtest

# ===============================================================
# targets

.PHONY: all depend clean

default: all

all: depend $(EXE) 

depend:
	$(CC) $(INCLUDE_PATH) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak *.o  $(EXE) $(EXE).exe Makefile.depend

# ===============================================================
# compilation rules

$(EXE): Makefile $(OBJS)
	$(CC) $(INCLUDE_PATH) -o $@ $(OBJS) $(LIB_PATH) $(LIBS) 

.cpp.o: Makefile
	$(CC) $(INCLUDE_PATH) $< -c -o $@

# ===============================================================

-include Makefile.depend
