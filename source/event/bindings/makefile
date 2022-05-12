#eventBindings build

BND_PATH = ../../../_build_linux/plugins/event/bindings
OBJ_PATH = ../../../_intermediate_linux/event/bindings
BUILD_INC_PATH = ../../../_build_linux/include
BASE_INC_PATH = ../../../include
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(BND_PATH)/eventPython.so
OBJECTS = $(OBJ_PATH)/eventBindings.o

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/eventBindings.o: eventBindings.cpp
	mkdir -p $(OBJ_PATH)
	$(COMPILE) eventBindings.cpp -o $(OBJ_PATH)/eventBindings.o

clean:
	rm *.o eventPython.so