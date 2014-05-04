CC=g++
CFLAGS=-c -Wall -Wextra -g
SOURCES=$(wildcard *.cc)

LIBS=
DEPS=RegExpParser
DEPS_PATH=../
INCLUDE=-I./ -I../
LINKPATH=-L../
LINKLIBS=-lpthread -lgtest_main $(foreach i,$(DEPS),-l${i})

GTEST_LIB = -L$(GTEST_DIR)/lib/.libs
GTEST_HEADERS = -I$(GTEST_DIR)/include/gtest
GTEST_HEADERS += -I$(GTEST_DIR)/include/gtest/internal
GTEST_HEADERS += -I$(GTEST_DIR)/include

OBJECTS=$(SOURCES:.cc=.o)

OUTPUT=testRegExp

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDE) $(GTEST_HEADERS) $< -o $@

$(OUTPUT) : $(OBJECTS) $(foreach i,$(DEPS),$(DEPS_PATH)lib${i}.a)
	$(CC) -o $@ $^ $(LINKLIBS) $(GTEST_LIB) $(LINKPATH)
	./$(OUTPUT)

clean:
	rm *.o $(OUTPUT)

