CC = g++

TARGET = main

CFLAGS = -g -Wall

LIB = -lboost_program_options

SRC = src/main.cpp

all:
	$(CC) $(CFLAGS) $(SRC) -o ./build/$(TARGET) $(LIB)
	echo HOLA MUNDO

clean:
	rm -rf ./build/$(TARGET)