LIBSHARD = libshard.a

CC = g++
FLAGS = -O0 -g -Wfatal-errors -Wall -pedantic -std=c++20 -Iinclude -Ilib/glfw/include -Ilib/glm -Ilib/vma/include -Ilib/stb
LINKER = -lvulkan -lpthread -ldl lib/glfw/build/src/libglfw3.a

SRC  = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(wildcard src/**/**/*.cpp) $(wildcard src/**/**/**/*.cpp)
SHADERS = $(wildcard **/*.vert) $(wildcard **/*.frag) $(wildcard **/**/*.vert) $(wildcard **/**/*.frag)

OBJ = $(SRC:%.cpp=%.o)
SHADER_SPV = $(SHADERS:%=%.spv)

OUT = out

MODERN_ART_EXAMPLE = examples/modern_art.cpp
CURRENT_EXAMPLE = $(MODERN_ART_EXAMPLE)

.PHONY: all clean cleanobj cleanspv run libs

all: libs build run

libs: lib/glfw/build
	cd lib/glfw/build && cmake .. && make
	clear

build: $(SHADER_SPV) $(OBJ)
	ar rcs $(LIBSHARD) $(OBJ)
	$(CC) -o $(OUT) $(CURRENT_EXAMPLE) $(FLAGS) $(LIBSHARD) $(LINKER)

run:
	./$(OUT)

cleanobj:
	rm $(OBJ)
cleanspv:
	rm $(SHADER_SPV)

clean: cleanobj cleanspv
	rm -rf lib/glfw/build
	rm $(OUT)

lib/glfw/build:
	mkdir $@

%.vert.spv: %.vert
	glslc -o $@ $<

%.frag.spv: %.frag
	glslc -o $@ $<

src/%.o: src/%.cpp
	$(CC) -o $@ -c $< $(FLAGS)