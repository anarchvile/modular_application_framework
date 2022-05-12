# pluginManager build

LIB_PATH = ../../_build_linux/lib
OBJ_PATH = ../../_intermediate_linux/pluginManager
BUILD_INC_PATH = ../../_build_linux/include
BASE_INC_PATH = ../../include

SRC_INC_FILES = common.h pluginManager.h
BASE_INC_FILES = $(BASE_INC_PATH)/common.h $(BASE_INC_PATH)/pluginManager.h

CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(LIB_PATH)/pluginManager.a
OBJECTS = $(OBJ_PATH)/common.o $(OBJ_PATH)/pluginManager.o $(OBJ_PATH)/stdafx.o

all: copy_inc $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/common.o: common.cpp $(BASE_INC_PATH)/common.h
	$(COMPILE) common.cpp -o $(OBJ_PATH)/common.o

$(OBJ_PATH)/pluginManager.o: pluginManager.cpp $(BASE_INC_PATH)/pluginManager.h
	$(COMPILE) pluginManager.cpp -o $(OBJ_PATH)/pluginManager.o

$(OBJ_PATH)/stdafx.o: stdafx.cpp stdafx.h
	$(COMPILE) stdafx.cpp -o $(OBJ_PATH)/stdafx.o

copy_inc:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(BASE_INC_PATH); done
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

clean:
	rm *.o pluginManager.a