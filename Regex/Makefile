CC=g++
CFLAGS=-c -Wall -Wextra -g
SOURCES=$(wildcard *.cc)

ifneq (${back_reference},0)
REG_DEFINE += -DSUPPORT_REG_EXP_BACK_REFERENCE
else
REG_DEFINE +=
endif

LIBS=
INCLUDE=-I./
INCLUDE+=-I../

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=libRegExpParser.a

all: $(SOURCES) $(OUTPUT)

%.o:%.cc
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@ ${REG_DEFINE}

$(OUTPUT) : $(OBJECTS)
	ar rcs $@ $^

clean:
	-rm *.o $(OUTPUT)

