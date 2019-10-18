.PHONY: clean

CC=
CFLAGS=-Wall -std=c++11 -pthread

INC=-I./include
LDFLAGS= -lpthread


all: ntripclient


ntripclient: examples/ntrip_client.o \
	src/ntrip.o
	$(CC)g++ $^ ${LDFLAGS} -o $@


%.o:%.cc
	$(CC)g++ $(CFLAGS) $(INC) $(LDFLAGS) -o $@ -c $<


install:
	$(CC)strip ntripclient 


clean:
	rm -rf src/*.o examples/*.o 
	rm -rf ntripclient

