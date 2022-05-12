# event build

PLG_PATH = ../../_build_linux/plugins
BUILD_INC_PATH = ../../_build_linux/include
BASE_INC_PATH = ../../include

SRC_INC_FILES = event.h
BASE_INC_FILES = $(BASE_INC_PATH)/event.h

all: copy_inc folders build_bindings

folders:
	mkdir -p bindings
	mkdir -p $(PLG_PATH)/event
	mkdir -p $(PLG_PATH)/event/bindings

copy_inc:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(BASE_INC_PATH); done
	for u in $(BASE_INC_FILES); do echo $$u; cp -f $$u $(BUILD_INC_PATH); done

build_bindings:
	+$(MAKE) -C bindings