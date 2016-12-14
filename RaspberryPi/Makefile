CC = gcc
CXX = g++
CFLAGS = -g -O3 -Wall -W -std=c11
CXXFLAGS = -g -O3 -Wall -W -std=c++11
LDFLAGS =
LDLIBS = -lm -L/usr/local/lib/ -lpaho-mqtt3c

# Target
TARGET = get_sensor_from_serial detect subscribe publish

.PHONY : all clean
all : $(TARGET)

clean :
	$(RM) *.o $(TARGET) *.exe
