cc_files := $(wildcard src/*.cc)

CC       = g++
CC_args  = -std=c++17 -O3 -Wno-enum-compare -Wno-format-security 
ifeq ($(OS),Windows_NT)
	raylib_flags = -L lib/WIN/ -lraylib -lopengl32 -lgdi32 -lwinmm
	platform_flags = -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
	output   = HyperFractal.exe
	package = HyperFractal.exe
else
	raylib_flags = -L lib/MAC/ -lraylib -framework IOKit -framework Cocoa -framework OpenGL
	platform_flags = $()
	output   = HyperFractal
	package = HyperFractal.app
endif

package_plist = Info.plist

HF_args = 1024 0.0 0.0 0.75 "(z^2)+c" 4 150
ifeq ($(OS),Windows_NT)
	HF = HyperFractal
else
	HF = ./HyperFractal
endif

build:
	@$(CC) $(CC_args) $(cc_files) $(raylib_flags) $(platform_flags) -o $(output)
	@echo Done.

run: $(build)
	@$(HF) $(HF_args)

package: $(build)
ifeq ($(OS),Windows_NT)
	@echo Nothing to do
else
	@mkdir -p $(package)/Contents/MacOS
	@cp $(output) $(package)/Contents/MacOS/$(output)
	@cp $(package_plist) $(package)/Contents/$(package_plist)
endif

guirun: $(build)
	@./$(output)

clean:
	@rm HyperFractal
