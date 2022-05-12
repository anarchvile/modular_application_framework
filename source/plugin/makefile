# base plugin class build

BUILD_INC_PATH = ../../_build_linux/include
BASE_INC_PATH = ../../include

SRC_INC_FILES = plugin.h plugin.py
BASE_INC_FILES = $(BASE_INC_PATH)/plugin.h $(BASE_INC_PATH)/plugin.py

all: copy_inc

copy_inc:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(BASE_INC_PATH); done
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done
