TARGET = lars_dns_test1
CXX = g++
CFLAGS = -g -O2 -Wall -Wno-deprecated

BASE = ../../base
BASE_H = $(BASE)/include

LARS_REACTOR = ../../Lars_reactor
LARS_REACTOR_H = $(LARS_REACTOR)/include $(LARS_REACTOR)/include/lars_reactor
LARS_REACTOR_LIB = $(LARS_REACTOR)/lib  -llars_reactor

PROTO = $(BASE)/proto
PROTO_H = $(BASE)/proto

MYSQL = /usr/include/mysql-connector-c
MYSQL_H = /usr/include/mysql-connector-c/include
MYSQL_LIB = /usr/include/mysql-connector-c/lib/libmysqlclient.a


OTHER_LIB = -lpthread -ldl -lprotobuf
SRC = ../src
INC = -I../include -I$(BASE_H) -I$(LARS_REACTOR_H) -I$(MYSQL_H) -I$(PROTO_H)
LIB = $(MYSQL_LIB) -L$(LARS_REACTOR_LIB) $(OTHER_LIB)

# 注意:这里只是将相应的.cc后缀变为.o后缀,并没有实质进行编译,类似于字符串转换
#OBJS = $(addsuffix .o, $(basename $(wildcard $(SRC)/*.cc)))
OBJS = lars_dns_test1.o

OBJS += $(PROTO)/lars.pb.o

# check : 
# 	@echo $(OBJS)

# -o 将多个目标文件链接成一个可执行文件
$(TARGET) : $(OBJS)
	$(CXX) -o $@  $(CFLAGS) $(OBJS) $(LIB) 

# -c 只编译不链接
# -o output 生成可执行文件
%.o : %.cc	
	$(CXX) -c -o $@ $< $(CFLAGS) $(INC)

.PHONY: clean

clean:
	-rm -f src/*.o $(TARGET)
