# main build

EXE_PATH = ../../_build_linux/bin
BUILD_INC_PATH = ../../_build_linux/include
PLG_MAN = ../../_build_linux/lib/pluginManager.a
PY_LIB = ../../_build_linux/lib/libpython3.so
CFLAGS = -pthread -g -std=c++17 -DLINUX_64 -fPIC -Wl,--no-as-needed -ldl -I $(BUILD_INC_PATH)

all: main

main: main.cpp
#	g++ $(PLG_MAN) -L $(PY_LIB) $(CFLAGS) -o $(EXE_PATH)/main main.cpp main.h
	g++ $(PLG_MAN) $(CFLAGS) `python3-config --cflags --ldflags` -o $(EXE_PATH)/main main.cpp main.h

clean:
	rm main
