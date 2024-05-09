CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated

INC=-I../../include
LIB=-L../../lib -llreactor -lpthread
OBJS = $(addsuffix .o, $(basename $(wildcard *.cc)))

all:
	$(CXX) -o qc_server $(CFLAGS)  qc_server.cc $(INC) $(LIB)
	$(CXX) -o qc_client $(CFLAGS)  qc_client.cc $(INC) $(LIB)

clean:
	rm -f *.o server client
