CC=g++
CFLAGS=-c -Wall -Wextra -g
SOURCES=$(wildcard *.cc)

LIBS=
INCLUDE=-I./

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=libRegExpParser.a

all: $(SOURCES) $(OUTPUT)

%.o:%.cc
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

$(OUTPUT) : $(OBJECTS)
	ar rcs $@ $^

clean:
	rm *.o $(OUTPUT)

