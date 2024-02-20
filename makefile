CC = g++

TARGET_SEQ = sequential
TARGET_PAR = parallel
TARGET_MAIN = main

CFLAGS = -g -Wall

LIB = -lboost_program_options

SRC_SEQ = src/sequential.cpp
SRC_PAR = src/parallel.cpp
SRC = src/main.cpp

all:
	$(CC) $(CFLAGS) $(SRC) -o ./build/$(TARGET_MAIN) $(LIB)
	$(CC) $(CFLAGS) $(SRC_SEQ) -o ./build/$(TARGET_SEQ) $(LIB)
	$(CC) $(CFLAGS) $(SRC_PAR) -o ./build/$(TARGET_PAR) $(LIB)

clean:
	rm -rf ./build/$(TARGET_SEQ)
	rm -rf ./build/$(TARGET_PAR)