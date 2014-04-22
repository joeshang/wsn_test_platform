CC = gcc
CPP = g++
RM = rm -rf

CPPUTEST_HOME = /home/joe/app/cpputest-3.5

CPPFLAGS += -c -Wall \
			-I$(CPPUTEST_HOME)/include \
			-I..

LD_LIBRARIES = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt

TARGET = all_tests

OBJECTS += AllTests.o \
		   LoggerTest.o \
		   FtpUploaderTest.o
OBJECTS += ../logger.o \
		   ../ftp_uploader.o

$(TARGET): $(OBJECTS)
	$(CPP) $^ $(LD_LIBRARIES) -o $@

%.o: %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(TARGET)
