cc_files := $(wildcard src/*.cc)
output   = HyperFractal
CC       = g++
CC_args  = -Wall -std=c++11

build:
	@$(CC) $(CC_args) $(cc_files) lib/libraylib.a -o $(output)
	@echo Done.

run: $(build)
	@./HyperFractal 4096 -1 0.0 2.0 "(z^2)+c" 4 150

clean:
	@rm HyperFractal
