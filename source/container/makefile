# container build

BIN_PATH = ../../_build_linux/bin
OBJ_PATH = ../../_intermediate_linux/container
BUILD_INC_PATH = ../../_build_linux/include
BASE_INC_PATH = ../../include

SRC_INC_FILES = container.h
BASE_INC_FILES = $(BASE_INC_PATH)/container.h

CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BIN_PATH)/container.so
OBJECTS = $(OBJ_PATH)/containerImpl.o $(OBJ_PATH)/stdafx.o

all: copy_inc $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/containerImpl.o: containerImpl.cpp containerImpl.h
	$(COMPILE) containerImpl.cpp -o $(OBJ_PATH)/containerImpl.o

$(OBJ_PATH)/stdafx.o: stdafx.cpp stdafx.h
	$(COMPILE) stdafx.cpp -o $(OBJ_PATH)/stdafx.o

copy_inc:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(BASE_INC_PATH); done
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

clean:
	rm *.o container.so
