# runner python bindings build

BND_PATH = ../../../_build_linux/plugins/runner/bindings
OBJ_PATH = ../../../_intermediate_linux/runner/bindings
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BND_PATH)/runnerPython.so
OBJECTS = $(OBJ_PATH)/runnerBindings.o

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/runnerBindings.o: runnerBindings.cpp
	mkdir -p $(OBJ_PATH)
	$(COMPILE) runnerBindings.cpp -o $(OBJ_PATH)/runnerBindings.o

clean:
	rm *.o runnerPython.so