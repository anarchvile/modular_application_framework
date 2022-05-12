# goodbyeWorld build

PLG_PATH = ../../_build_linux/plugins

all: folders build_subdirs

folders:
	mkdir -p $(PLG_PATH)/goodbyeWorld
	mkdir -p $(PLG_PATH)/goodbyeWorld/bin
	mkdir -p $(PLG_PATH)/goodbyeWorld/bindings

build_subdirs:
	+$(MAKE) -C plugins
	+$(MAKE) -C bindings
