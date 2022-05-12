# goodbyeWorld python bindings build

BND_PATH = ../../../_build_linux/plugins/goodbyeWorld/bindings
OBJ_PATH = ../../../_intermediate_linux/goodbyeWorld/bindings
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BND_PATH)/goodbyeWorldPython.so
OBJECTS = $(OBJ_PATH)/goodbyeWorldBindings.o

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/goodbyeWorldBindings.o: goodbyeWorldBindings.cpp
	mkdir -p $(OBJ_PATH)
	$(COMPILE) goodbyeWorldBindings.cpp -o $(OBJ_PATH)/goodbyeWorldBindings.o

clean:
	rm *.o goodbyeWorldPython.so