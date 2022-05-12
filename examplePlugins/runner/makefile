# runner build

PLG_PATH = ../../_build_linux/plugins

all: folders build_subdirs

folders:
	mkdir -p $(PLG_PATH)/runner
	mkdir -p $(PLG_PATH)/runner/bin
	mkdir -p $(PLG_PATH)/runner/bindings

build_subdirs:
	+$(MAKE) -C plugins
	+$(MAKE) -C bindings
