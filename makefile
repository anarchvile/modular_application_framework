# run this makefile to build the entire project
# Check if async calls work in linux, test python plugins, get input
# to build in linux
# if not exist "$(SolutionDir)_build\$(Platform)\$(Configuration)\include" (mklink /J "$(SolutionDir)_build\$(Platform)\$(Configuration)\include" "$(SolutionDir)include")

BUILD_PATH = _build_linux
CONFIG_FILE = startup.cfg

# Maybe switch copy_include and src_build, so that src_build
# copies necessary include files into the global include before
# copying the global include directory into _build_linux.
all: folders copy_config copy_include src_build

folders:
	mkdir -p _build_linux
	mkdir -p _build_linux/bin
	mkdir -p _build_linux/include
	mkdir -p _build_linux/lib
	mkdir -p _build_linux/plugins

	mkdir -p _intermediate_linux
	mkdir -p _intermediate_linux/atomicPlugin
	mkdir -p _intermediate_linux/container
	mkdir -p _intermediate_linux/event
	mkdir -p _intermediate_linux/goodbyeWorld
	mkdir -p _intermediate_linux/helloWorld
	mkdir -p _intermediate_linux/input
	mkdir -p _intermediate_linux/main
	mkdir -p _intermediate_linux/plugin
	mkdir -p _intermediate_linux/pluginManager
	mkdir -p _intermediate_linux/runner
	mkdir -p _intermediate_linux/concurrentLoading

src_build:
	+$(MAKE) -C source/container
	+$(MAKE) -C source/plugin
	+$(MAKE) -C source/event
	+$(MAKE) -C source/pluginManager
	+$(MAKE) -C source/main

	+$(MAKE) -C examplePlugins/runner
	+$(MAKE) -C examplePlugins/atomicPlugin
	+$(MAKE) -C examplePlugins/concurrentLoading
	+$(MAKE) -C examplePlugins/helloWorld
	+$(MAKE) -C examplePlugins/goodbyeWorld
	+$(MAKE) -C examplePlugins/input
	+$(MAKE) -C examplePlugins/pyExPlg1
	+$(MAKE) -C examplePlugins/pyExPlg2

copy_config:
	cp -f $(CONFIG_FILE) $(BUILD_PATH)

copy_include:
	rm -rf _build_linux/include
	mkdir -p _build_linux/include
	cp -r include _build_linux

copy_lib:
	cp -f libpython3.so _build_linux/lib
	cp -f libpython3.6m.so.1.0 _build_linux/lib
