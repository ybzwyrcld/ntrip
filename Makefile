CC=g++
CFLAGS=-Wall -std=c++11

TARGET = NTRIPCaster

all: $(TARGET)
	mv $(TARGET) ./run


SRCS = main.cpp \
	caster/ntrip_caster.cpp \
	util/ntrip_util.cpp 


OBJS = $(SRCS:.cpp=.o)
$(TARGET):$(OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
	$(CC) -o $@ $^

 
INC = -I./include

clean:
	rm -rf ./run/$(TARGET) $(OBJS)

%.o:%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<
