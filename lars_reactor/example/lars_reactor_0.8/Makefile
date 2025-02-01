CXX=g++
CFLAGS=-g -O2 -Wall -fPIC -Wno-deprecated

INC=-I../../include/lars_reactor -I../../include
LIB=-L../../lib -llars_reactor -lpthread
OBJS = $(addsuffix .o, $(basename $(wildcard *.cc)))

all:
	$(CXX) -o server $(CFLAGS)  server.cc $(INC) $(LIB) -fcoroutines
	$(CXX) -o client $(CFLAGS)  client.cc $(INC) $(LIB) -fcoroutines

clean:
	rm -f *.o server client
