CC=clang++
CFLAGS=-c -Wall -Wextra -g -std=c++11
SOURCES=$(wildcard *.cc)

REG_DEFINE =

LIBS=
DEPS=ink
DEPS_PATH=../
INCLUDE=-I./ -I../ -I../..
LINKPATH=-L../
LINKLIBS=$(foreach i,$(DEPS),-l${i}) -lgtest_main -lgtest -lpthread

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=testInk

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDE) $(REG_DEFINE) $< -o $@

$(OUTPUT) : $(OBJECTS) $(foreach i,$(DEPS),$(DEPS_PATH)lib${i}.a)
	$(CC) -o $@ $^ $(GTEST_LIB) $(LINKPATH) $(LINKLIBS)
	cp ./$(OUTPUT) ./$(OUTPUT).bak
	-./$(OUTPUT)
	-rm -f ./$(OUTPUT).bak

clean:
	-rm ../*.o
	-rm *.o $(OUTPUT)
	-rm -f $(OUTPUT).bak

