# Makefile for c/urg/samples
# $Id$

# Compile options
CARGO = cargo
SRCDIR = ./lib/src
INCLUDEDIR = ./lib/include/cpp
BINDIR = ./bin
CC = gcc
CXX = g++
CFLAGS = -g -O3 -Wall -W $(INCLUDES)
CXXFLAGS = $(CFLAGS)
INCLUDES = -I$(INCLUDEDIR) -I./lib
LDFLAGS =
LDLIBS = -lm $(shell if test `echo $(OS) | grep Windows`; then echo "-lwsock32 -lsetupapi"; else if test `uname -s | grep Darwin`; then echo ""; else echo "-lrt"; fi; fi) -L$(SRCDIR)

# Target
TARGET = get_distance clustring event_detection

all : $(TARGET)

clean :
	$(RM) *.o $(TARGET) *.exe

depend :
	makedepend -Y -- $(INCLUDES) -- $(wildcard *.h *.c)

clustring:
	cd $@/ && $(CARGO) build && $(CARGO) build --release

event_detection:
	cd $@/ && $(CARGO) build && $(CARGO) build --release

.PHONY : all depend clean clustring event_detection

######################################################################
REQUIRE_LIB = $(SRCDIR)/liburg_cpp.a
$(REQUIRE_LIB) : $(wildcard $(SRCDIR)/*.[ch])
	cd $(@D)/ && $(MAKE) $(@F)

get_distance : ./lib/Connection_information.o $(REQUIRE_LIB) 