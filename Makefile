LIBSHARD = libshard.a

CC = g++
FLAGS = -O0 -g -Wfatal-errors -Wall -pedantic -std=c++20 -Iinclude -Ilib/glfw/include -Ilib/glm -Ilib/vma/include
LINKER = -lvulkan -lpthread -ldl lib/glfw/build/src/libglfw3.a

SRC  = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp) $(wildcard src/**/**/*.cpp) $(wildcard src/**/**/**/*.cpp)
SHADERS = $(wildcard shaders/*.vert) $(wildcard shaders/*.frag)

OBJ = $(SRC:%.cpp=%.o)
SHADER_SPV = $(SHADERS:%=%.spv)

OUT = out
CURRENT_EXAMPLE = examples/main.cpp

.PHONY: all clean cleanobj run libs

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

clean: cleanobj
	rm -rf lib/glfw/build
	rm -rf lib/vma/build
	rm $(OUT)

lib/glfw/build:
	mkdir $@

shaders/%.vert.spv: shaders/%.vert
	glslc -o $@ $<

shaders/%.frag.spv: shaders/%.frag
	glslc -o $@ $<

src/%.o: src/%.cpp
	$(CC) -o $@ -c $< $(FLAGS)