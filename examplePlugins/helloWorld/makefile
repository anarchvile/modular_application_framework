# helloWorld build

PLG_PATH = ../../_build_linux/plugins
OBJ_PATH = ../../_intermediate_linux/helloWorld
BUILD_INC_PATH = ../../_build_linux/include
BASE_INC_PATH = ../../include/
BASE_INC_FILES = $(BASE_INC_PATH)/helloWorld.h
CFLAGS = -std=c++11 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BASE_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(PLG_PATH)/helloWorld.so
OBJECTS = $(OBJ_PATH)/helloWorldImpl.o $(OBJ_PATH)/stdafx.o

all: $(OUTPUT) copy_inc

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/helloWorldImpl.o: helloWorldImpl.cpp helloWorldImpl.h
	$(COMPILE) helloWorldImpl.cpp -o $(OBJ_PATH)/helloWorldImpl.o

$(OBJ_PATH)/stdafx.o: stdafx.cpp stdafx.h
	$(COMPILE) stdafx.cpp -o $(OBJ_PATH)/stdafx.o

copy_inc:
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

clean:
	rm *.o helloWorld.so
