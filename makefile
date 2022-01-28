# run this makefile to build the entire project

all: folders src_build

folders:
	mkdir -p _build_linux
	mkdir -p _build_linux/bin
	mkdir -p _build_linux/plugins
	mkdir -p _build_linux/include
	mkdir -p _build_linux/lib

	mkdir -p _intermediate_linux
	mkdir -p _intermediate_linux/container
	mkdir -p _intermediate_linux/main
	mkdir -p _intermediate_linux/plugin
	mkdir -p _intermediate_linux/pluginManager
	
	mkdir -p _intermediate_linux/runner
	mkdir -p _intermediate_linux/helloWorld
	mkdir -p _intermediate_linux/goodbyeWorld
	mkdir -p _intermediate_linux/concurrentLoading

src_build:
	+$(MAKE) -C source/container
	+$(MAKE) -C source/plugin
	+$(MAKE) -C source/pluginManager
	+$(MAKE) -C examplePlugins/runner
	+$(MAKE) -C examplePlugins/helloWorld
	+$(MAKE) -C examplePlugins/goodbyeWorld
	+$(MAKE) -C examplePlugins/concurrentLoading
	+$(MAKE) -C source/main

