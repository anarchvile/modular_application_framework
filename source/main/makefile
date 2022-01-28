# main build

BUILD_PATH = ../../_build_linux
INC_PATH = ../../_build_linux/include/
CONFIG_FILE = ../../main.cfg
EXE_PATH = ../../_build_linux/bin/
PLG_MAN = ../../_build_linux/lib/pluginManager.a
CFLAGS = -std=c++11 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(INC_PATH)

all: copy_config main

copy_config:
	cp -f $(CONFIG_FILE) $(BUILD_PATH)

main: main.cpp
	g++ $(PLG_MAN) $(CFLAGS) -o $(EXE_PATH)main main.cpp main.h

clean:
	rm main
