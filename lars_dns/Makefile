TARGET = bin/lars_dns
CXX = g++
CFLAGS = -g -O2 -Wall -Wno-deprecated

BASE = ../base
BASE_H = $(BASE)/include
LARS_REACTOR = ../Lars_reactor
LARS_REACTOR_H = $(LARS_REACTOR)/include
LARS_REACTOR_LIB = $(LARS_REACTOR)/lib  -llreactor

MYSQL = $(BASE)/mysql-connector-c
MYSQL_H = $(MYSQL)/include
MYSQL_LIB = $(MYSQL)/lib/libmysqlclient.a


OTHER_LIB = -lpthread -ldl
SRC = ./src
INC = -I./include -I$(BASE_H) -I$(LARS_REACTOR_H) -I$(MYSQL_H)
LIB = $(MYSQL_LIB) -L$(LARS_REACTOR_LIB) $(OTHER_LIB)

# 注意:这里只是将相应的.cc后缀变为.o后缀,并没有实质进行编译,类似于字符串转换
OBJS = $(addsuffix .o, $(basename $(wildcard $(SRC)/*.cc)))

# check : 
# 	@echo $(OBJS)

# -o 将多个目标文件链接成一个可执行文件
$(TARGET) : $(OBJS)
	mkdir -p bin
	$(CXX) -o $@  $(CFLAGS) $(OBJS) $(LIB) 

# -c 只编译不链接
# -o output 生成可执行文件
%.o : %.cc	
	$(CXX) -c -o $@ $< $(CFLAGS) $(INC)

.PHONY: clean

clean:
	-rm -f src/*.o $(TARGET)
