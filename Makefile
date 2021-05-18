cc_files := $(wildcard src/*.cc)
output   = HyperFractal
CC       = g++
CC_args  = -Wall

build:
	@$(CC) $(CC_args) $(cc_files) -o $(output)
	@echo Done.

run: $(build)
	@./HyperFractal $(arg)

clean:
	@rm HyperFractal
