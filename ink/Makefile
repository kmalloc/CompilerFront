CC=clang++
CFLAGS=-c -Wall -Wextra -g -Werror -std=c++11
SOURCES=$(wildcard *.cc)

LIBS=
INCLUDE=-I./

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=libink.a

all: $(SOURCES) $(OUTPUT)

%.o:%.cc
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ ${REG_DEFINE}

depend:.depend

.depend:$(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

include .depend

$(OUTPUT) : $(OBJECTS)
	ar rcs $@ $^

clean:
	-rm *.o $(OUTPUT) .depend

