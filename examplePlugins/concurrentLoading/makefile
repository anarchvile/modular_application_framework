# concurrentLoading build

PLG_PATH = ../../_build_linux/plugins

all: folders build_subdirs

folders:
	mkdir -p $(PLG_PATH)/concurrentLoading
	mkdir -p $(PLG_PATH)/concurrentLoading/bin

build_subdirs:
	+$(MAKE) -C plugins
