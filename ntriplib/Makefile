.PHONY: clean

CC=
CFLAGS=-Wall -std=c++11

INC=-I.
LDFLAGS=


all: ntripcaster ntripclient ntripserver


ntripcaster: main.o \
    caster/ntrip_caster.o \
	util/ntrip_util.o
	$(CC)g++ $^ -o $@


ntripclient: client/ntrip_client.o \
	util/ntrip_util.o
	$(CC)g++ $^ -o $@


ntripserver: server/ntrip_server.o \
	util/ntrip_util.o
	$(CC)g++ $^ -o $@


%.o:%.cc
	$(CC)g++ $(CFLAGS) $(INC) $(LDFLAGS) -o $@ -c $<


install:
	$(CC)strip ntripcaster ntripclient ntripserver


clean:
	rm -rf *.o caster/*.o client/*.o server/*.o util/*.o
	rm -rf ntripcaster ntripserver ntripclient

