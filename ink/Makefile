CC=g++
CFLAGS=-c -Wall -Wextra -g -Werror
SOURCES=$(wildcard *.cc)

REG_DEFINE=

LIBS=
INCLUDE=-I./
INCLUDE+=-I/usr/local/include/boost/

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=libink.a

all: $(SOURCES) $(OUTPUT)

%.o:%.cc
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ ${REG_DEFINE}

$(OUTPUT) : $(OBJECTS)
	ar rcs $@ $^

clean:
	-rm *.o $(OUTPUT)

