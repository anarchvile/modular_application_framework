# goodbyeWorld plugins build

BIN_PATH = ../../../_build_linux/plugins/goodbyeWorld/bin
OBJ_PATH = ../../../_intermediate_linux/goodbyeWorld
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include

SRC_INC_FILES = goodbyeWorld.h
BASE_INC_FILES = $(BASE_INC_PATH)/goodbyeWorld.h

CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BIN_PATH)/goodbyeWorld.so
OBJECTS = $(OBJ_PATH)/goodbyeWorldImpl.o $(OBJ_PATH)/stdafx.o

all: copy_inc $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/goodbyeWorldImpl.o: goodbyeWorldImpl.cpp goodbyeWorldImpl.h
	$(COMPILE) goodbyeWorldImpl.cpp -o $(OBJ_PATH)/goodbyeWorldImpl.o

$(OBJ_PATH)/stdafx.o: stdafx.cpp stdafx.h
	$(COMPILE) stdafx.cpp -o $(OBJ_PATH)/stdafx.o

copy_inc:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(BASE_INC_PATH); done
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

clean:
	rm *.o goodbyeWorld.so
