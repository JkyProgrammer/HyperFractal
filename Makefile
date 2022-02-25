cc_files := $(wildcard src/*.cc)

CC       = g++
CC_args  = -std=c++17 -O3 -Wno-enum-compare -Wno-format-security
ifeq ($(OS),Windows_NT)
	raylib_flags = -L lib/WIN/ -lraylib -lopengl32 -lgdi32 -lwinmm
	output   = HyperFractal.exe
else
	raylib_flags = -L lib/MAC/ -lraylib -framework IOKit -framework Cocoa -framework OpenGL
	output   = HyperFractal
endif

HF_args = 1024 0.0 0.0 0.75 "(z^2)+c" 4 150
ifeq ($(OS),Windows_NT)
	HF = HyperFractal
else
	HF = ./HyperFractal
endif

build:
	@$(CC) $(CC_args) $(cc_files) $(raylib_flags) -o $(output)
	@echo Done.

run: $(build)
	@$(HF) $(HF_args)

guirun: $(build)
	@./$(output)

clean:
	@rm HyperFractal
