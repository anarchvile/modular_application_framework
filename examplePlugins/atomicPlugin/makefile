# atomicPlugin build

PLG_PATH = ../../_build_linux/plugins

all: folders build_subdirs

folders:
	mkdir -p $(PLG_PATH)/atomicPlugin
	mkdir -p $(PLG_PATH)/atomicPlugin/bin

build_subdirs:
	+$(MAKE) -C plugins
