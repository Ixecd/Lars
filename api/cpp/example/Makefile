
CXX = g++
CFLAGS = -g -O2 -Wall -Wno-deprecated -fcoroutines

BASE = ../../../base
BASE_H = $(BASE)/include

LARS_REACTOR = ../../../lars_reactor
LARS_REACTOR_H = $(LARS_REACTOR)/include
LARS_REACTOR_LIB = $(LARS_REACTOR)/lib  -llars_reactor

PROTO = $(BASE)/proto
PROTO_H = $(BASE)/proto

all :
	$(CXX) $(CFLAGS) -o example example.cpp -I../ -L../lib -llars_client -I$(LARS_REACTOR_H) -L$(LARS_REACTOR_LIB) -llars_reactor -lpthread -ldl -lprotobuf -I$(PROTO_H) -L$(BASE)/lib -lbase -fcoroutines

	$(CXX) $(CFLAGS) -o report report.cc -I../ -L../lib -llars_client -I$(LARS_REACTOR_H) -L$(LARS_REACTOR_LIB) -llars_reactor -lpthread -ldl -lprotobuf -I$(PROTO_H) -L$(BASE)/lib -lbase -fcoroutines


.PHONY: clean 

clean:
	-rm ./example ./qps ./report