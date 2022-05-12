# input build

CONFIG_FILE = start_option.cfg
PLG_PATH = ../../_build_linux/plugins

all: folders copy_config build_subdirs

folders:
	mkdir -p $(PLG_PATH)/input
	mkdir -p $(PLG_PATH)/input/bin
	mkdir -p $(PLG_PATH)/input/bindings

build_subdirs:
	+$(MAKE) -C plugins
	+$(MAKE) -C bindings

copy_config:
	cp -f $(CONFIG_FILE) $(PLG_PATH)/input
