.PHONY: clean

CC=
CFLAGS=-Wall -std=c++11 -pthread

INC=-I./include


all: ntrip_client_exam ntrip_caster_exam ntrip_server_exam


ntrip_caster_exam: examples/ntrip_caster_exam.o \
	src/ntrip_caster.o \
	src/ntrip_util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@

ntrip_client_exam: examples/ntrip_client_exam.o \
	src/ntrip_client.o \
	src/ntrip_util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@

ntrip_server_exam: examples/ntrip_server_exam.o \
	src/ntrip_server.o \
	src/ntrip_util.o
	$(CC)g++ $^ ${LDFLAGS} -o $@

%.o:%.cc
	$(CC)g++ $(CFLAGS) $(INC) $(LDFLAGS) -o $@ -c $<

install:
	$(CC)strip ntrip_caster_exam ntrip_client_exam ntrip_server_exam


clean:
	rm -rf src/*.o examples/*.o 
	rm -rf ntrip_caster_exam ntrip_client_exam ntrip_server_exam

