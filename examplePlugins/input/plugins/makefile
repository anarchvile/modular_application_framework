# input plugins build

BIN_PATH = ../../../_build_linux/plugins/input/bin
OBJ_PATH = ../../../_intermediate_linux/input
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include

SRC_INC_FILES = input.h
BASE_INC_FILES = $(BASE_INC_PATH)/input.h

# TODO: Need to link x11 libs to input
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH) -I/usr/include
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BIN_PATH)/input.so
OBJECTS = $(OBJ_PATH)/inputImpl.o $(OBJ_PATH)/stdafx.o

all: copy_inc $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) `pkg-config --cflags --libs x11` -o $(OUTPUT) $(OBJECTS)
	
$(OBJ_PATH)/inputImpl.o: inputImpl.cpp inputImpl.h
	$(COMPILE) inputImpl.cpp `pkg-config --cflags --libs x11` -o $(OBJ_PATH)/inputImpl.o

$(OBJ_PATH)/stdafx.o: stdafx.cpp stdafx.h
	$(COMPILE) stdafx.cpp `pkg-config --cflags --libs x11` -o $(OBJ_PATH)/stdafx.o

copy_inc:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(BASE_INC_PATH); done
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

clean:
	rm *.o input.so
