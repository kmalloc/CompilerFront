CC=g++
CFLAGS=-c -Wall -Wextra -g
SOURCES=$(wildcard *.cc) $(wildcard ../*.cc)

ifneq (${back_reference},0)
REG_DEFINE += -DSUPPORT_REG_EXP_BACK_REFERENCE
endif

LIBS=
#DEPS=RegExpParser
DEPS_PATH=../
INCLUDE=-I./ -I../ -I../../
LINKPATH=-L../
LINKLIBS=$(foreach i,$(DEPS),-l${i}) -lgtest_main -lgtest -lpthread

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=testRegExp

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

