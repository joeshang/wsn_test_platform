CC = gcc
CPP = g++
RM = rm -rf

CPPUTEST_HOME = /home/joe/app/cpputest-3.5

CPPFLAGS += -c \
			-I$(CPPUTEST_HOME)/include \
			-I..

LD_LIBRARIES = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt

TARGET = all_tests

OBJECTS += AllTests.o \
		   LoggerTest.o
OBJECTS += ../logger.o

$(TARGET): $(OBJECTS)
	$(CPP) $^ $(LD_LIBRARIES) -o $@

%.o: %.[cpp/c]
	$(CPP) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(TARGET)
