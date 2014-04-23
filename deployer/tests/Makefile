CC = gcc
CPP = g++
RM = rm -rf

CPPUTEST_HOME = /home/joe/app/cpputest-3.5

CPPFLAGS += -c -Wall -D_DEBUG \
			-I$(CPPUTEST_HOME)/include \
			-I../lib/libtelnet/ \
			-I../

LD_LIBRARIES = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt

TARGET = all_tests

OBJECTS += AllTests.o \
		   LoggerTest.o \
		   FtpUploaderTest.o \
		   TelnetCommanderTest.o
OBJECTS += ../logger.o \
		   ../ftp_uploader.o \
		   ../telnet_commander.o \
		   ../lib/libtelnet/libtelnet.o

$(TARGET): $(OBJECTS)
	$(CPP) $^ $(LD_LIBRARIES) -o $@

%.o: %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(TARGET)
