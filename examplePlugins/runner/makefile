# runner build

PLG_PATH = ../../_build_linux/plugins
OBJ_PATH = ../../_intermediate_linux/runner
BUILD_INC_PATH = ../../_build_linux/include
BASE_INC_PATH = ../../include/
BASE_INC_FILES = $(BASE_INC_PATH)/runner.h
CFLAGS = -std=c++11 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BASE_INC_PATH)
CC = g++
COMPILE	= $(CC) $(CFLAGS) -c
LD = $(CC) -shared
OUTPUT = $(PLG_PATH)/runner.so
OBJECTS = $(OBJ_PATH)/runnerImpl.o $(OBJ_PATH)/stdafx.o

all: $(OUTPUT) copy_inc

$(OUTPUT): $(OBJECTS)
	$(LD) -o $(OUTPUT) $(OBJECTS)

$(OBJ_PATH)/runnerImpl.o: runnerImpl.cpp runnerImpl.h
	$(COMPILE) runnerImpl.cpp -o $(OBJ_PATH)/runnerImpl.o

$(OBJ_PATH)/stdafx.o: stdafx.cpp stdafx.h
	$(COMPILE) stdafx.cpp -o $(OBJ_PATH)/stdafx.o

copy_inc:
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

clean:
	rm *.o runner.so
