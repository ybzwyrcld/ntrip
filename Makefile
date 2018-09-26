.PHONY: clean
CC=

all: ntripcaster ntripclient ntripserver

ntripcaster: ntripcaster.o util.o
	$(CC)g++ $^ -o $@ 
	mv $@ ./run/

ntripclient: ntripclient.o util.o
	$(CC)g++ $^ -o $@ 
	mv $@ ./run/

ntripserver: ntripserver.o util.o
	$(CC)g++ $^ -o $@ 
	mv $@ ./run/

clean:
	rm -rf *.o ./run/*
