cc_files := $(wildcard src/*.cc)
output   = HyperFractal
ifeq ($(OS),Windows_NT)
	@echo This Makefile is not configured for building on Windows
else
	CC       = g++
	CC_args  = -Wall -std=c++17
	raylib_flags = -framework IOKit -framework Cocoa -framework OpenGL lib/libraylib.a
endif

build:
	@$(CC) $(CC_args) $(cc_files) $(raylib_flags) -o $(output)
	@echo Done.

run: $(build)
	@./HyperFractal 1024 0.0 0.0 0.75 "(z^2)+c" 4 150

guirun: $(build)
	@./HyperFractal

clean:
	@rm HyperFractal
