# helloWorld python bindings build

BND_PATH = ../../../_build_linux/plugins/helloWorld/bindings
OBJ_PATH = ../../../_intermediate_linux/helloWorld/bindings
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BND_PATH)/helloWorldPython.so
OBJECTS = $(OBJ_PATH)/helloWorldBindings.o

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/helloWorldBindings.o: helloWorldBindings.cpp
	mkdir -p $(OBJ_PATH)
	$(COMPILE) helloWorldBindings.cpp -o $(OBJ_PATH)/helloWorldBindings.o

clean:
	rm *.o helloWorldPython.so