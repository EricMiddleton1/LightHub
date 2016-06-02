CC=g++
LD=g++

CFLAGS=-std=c++14 -Wall

srcdir=source
objdir=obj

all: LightHub

LightHub: $(objdir)/Exception.o $(objdir)/Color.o $(objdir)/LightStrip.o $(objdir)/Packet.o $(objdir)/LightNode.o $(objdir)/main.o
	$(LD) $(CFLAGS) $(objdir)/Exception.o $(objdir)/Color.o $(objdir)/LightStrip.o $(objdir)/Packet.o $(objdir)/LightNode.o $(objdir)/main.o -lboost_system -lpthread -o LightHub

$(objdir)/Exception.o: $(srcdir)/Exception.hpp $(srcdir)/Exception.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/Exception.cpp -o $(objdir)/Exception.o

$(objdir)/Color.o: $(srcdir)/Color.hpp $(srcdir)/Color.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/Color.cpp -o $(objdir)/Color.o

$(objdir)/LightStrip.o: $(srcdir)/LightStrip.hpp $(srcdir)/LightStrip.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/LightStrip.cpp -o $(objdir)/LightStrip.o

$(objdir)/Packet.o: $(srcdir)/Packet.hpp $(srcdir)/Packet.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/Packet.cpp -o $(objdir)/Packet.o

$(objdir)/LightNode.o: $(srcdir)/LightNode.hpp $(srcdir)/LightNode.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/LightNode.cpp -o $(objdir)/LightNode.o

$(objdir)/main.o: $(srcdir)/main.hpp $(srcdir)/main.cpp
	$(CC) $(CFLAGS) -c $(srcdir)/main.cpp -o $(objdir)/main.o

clean:
	rm $(objdir)/*.o
