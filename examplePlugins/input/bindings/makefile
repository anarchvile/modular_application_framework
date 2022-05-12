# input python bindings build

BND_PATH = ../../../_build_linux/plugins/input/bindings
OBJ_PATH = ../../../_intermediate_linux/input/bindings
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BND_PATH)/inputPython.so
OBJECTS = $(OBJ_PATH)/inputBindings.o

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/inputBindings.o: inputBindings.cpp
	mkdir -p $(OBJ_PATH)
	$(COMPILE) inputBindings.cpp -o $(OBJ_PATH)/inputBindings.o

clean:
	rm *.o inputPython.so