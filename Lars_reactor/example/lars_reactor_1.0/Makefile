CXX=g++
CFLAGS=-g -O2 -fPIC -Wall -Wno-deprecated

INC=-I../../include
LIB=-L../../lib -llreactor -lpthread
OBJS = $(addsuffix .o, $(basename $(wildcard *.cc)))

all :
	$(CXX) -o server $(CFLAGS) server.cc $(INC) $(LIB)
	$(CXX) -o client $(CFLAGS) client.cc $(INC) $(LIB)

clean :
	-rm -rf *.o server client

