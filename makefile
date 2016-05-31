CC=g++
LD=g++

CFLAGS=-std=c++14 -Wall

srcdir=source
objdir=obj

all: LightHub

LightHub: $(objdir)/LightNode.o $(objdir)/main.o
	$(LD) $(CFLAGS) $(objdir)/LightNode.o $(objdir)/main.o -o LightHub

$(objdir)/LightNode.o: $(srcdir)/LightNode.hpp $(srcdir)/LightNode.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/LightNode.cpp -o $(objdir)/LightNode.o

$(objdir)/main.o: $(srcdir)/main.hpp $(srcdir)/main.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/main.cpp -o $(objdir)/main.o
