# helloWorld scripts build

FINAL_PATH = ../../../_build_linux/plugins/helloWorld/scripts
SRC_INC_FILES = helloWorld.py

all: copy_py

copy_py:
	for u in $(SRC_INC_FILES); do echo $$u; cp -f $$u $(FINAL_PATH); done
