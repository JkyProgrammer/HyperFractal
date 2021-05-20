cc_files := $(wildcard src/*.cc)
output   = HyperFractal
CC       = g++
CC_args  = -Wall

build:
	@$(CC) $(CC_args) $(cc_files) -o $(output)
	@echo Done.

run: $(build)
	@./HyperFractal 1024 0.0 0.0 1.0 z^2+c 4

clean:
	@rm HyperFractal
