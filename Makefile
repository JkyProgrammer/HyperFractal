cc_files := $(wildcard src/*.cc)

CC       = g++
CC_args  = -std=c++17 -Ilib/ -Wno-enum-compare -Wno-cpp -fcompare-debug-second
ifeq ($(OS),Windows_NT)
	raylib_flags = -L lib/WIN/ -lraylib -lopengl32 -lgdi32 -lwinmm
	mathlib_flags = -L lib/WIN/
	output   = HyperFractal.exe
else
	raylib_flags = -L lib/MAC/ -lraylib -framework IOKit -framework Cocoa -framework OpenGL
	output   = HyperFractal
endif

HF_args = 256 0.0 0.0 0.75 "(z^2)+c" 1 150
ifeq ($(OS),Windows_NT)
	HF = HyperFractal
else
	HF = ./HyperFractal
endif

build:
	@$(CC) $(CC_args) $(cc_files) $(raylib_flags) $(mathlib_flags) -o $(output)
	@echo Done.

run: $(build)
	@$(HF) $(HF_args)

guirun: $(build)
	@./$(output)

clean:
	@rm HyperFractal
