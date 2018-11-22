CC=g++
CFLAGS=-Wall -std=c++11

CASTER_TARGET = NTRIPCaster
CLIENT_TARGET = NTRIPClient
SERVER_TARGET = NTRIPServer
TARGET = $(CASTER_TARGET) $(CLIENT_TARGET) $(SERVER_TARGET)

all: $(TARGET)
	mv $(TARGET) ./run


CASTER_SRCS = caster/ntrip_caster.cpp \
	util/ntrip_util.cpp 

CLIENT_SRCS = client/ntrip_client.cpp \
	util/ntrip_util.cpp 

SERVER_SRCS = server/ntrip_server.cpp \
	util/ntrip_util.cpp 


CASTER_OBJS = $(CASTER_SRCS:.cpp=.o)
$(CASTER_TARGET):$(CASTER_OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
	$(CC) -o $@ $^


CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
$(CLIENT_TARGET):$(CLIENT_OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
	$(CC) -o $@ $^


SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
$(SERVER_TARGET):$(SERVER_OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
	$(CC) -o $@ $^
   
 
INC = -I./include

OBJS = $(CASTER_OBJS) $(CLIENT_OBJS) $(SERVER_OBJS)

clean:
	rm -rf ./run/* $(OBJS)

%.o:%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<
