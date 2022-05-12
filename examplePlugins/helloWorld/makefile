# helloWorld build

PLG_PATH = ../../_build_linux/plugins

all: folders build_subdirs

folders:
	mkdir -p $(PLG_PATH)/helloWorld
	mkdir -p $(PLG_PATH)/helloWorld/bin
	mkdir -p $(PLG_PATH)/helloWorld/bindings
	mkdir -p $(PLG_PATH)/helloWorld/scripts

build_subdirs:
	+$(MAKE) -C plugins
	+$(MAKE) -C bindings
	+$(MAKE) -C scripts
