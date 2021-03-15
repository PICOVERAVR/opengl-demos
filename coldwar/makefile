# you can use the GCC -H option to list all included directories, helps with build debugging
# and make -j for makefile debugging

# specify directories to search for files
# special search paths are for GLAD headers
VPATH := ./src ./header ./header/glad ./header/KHR
CC := g++

# get compilation options from libraries used, assuming they're actually installed
LIBS := $(shell pkg-config --libs glfw3 assimp glm) -ldl -pthread
LIBFLAGS := $(shell pkg-config --cflags glfw3 assimp glm)

DIRS := -I header
CFLAGS := -Wall -std=c++14 $(DIRS) $(LIBFLAGS)

# specify a single target name here
TARGETS := debug release
MAINS := $(addsuffix .o, $(TARGETS) )

# specify a list of object files we want
OBJFILES := camera.o main.o program.o vloader.o window.o glad.o map.o unit.o prompt.o

# NOTE: glad.c has to be renamed to glad.cpp to make things easier
# also glad.h and khrplatform have to be in folders to avoid having to adjust glad.cpp

# all and clean aren't actually creating files, so they are declared as phony targets
.PHONY: top clean

# update CFLAGS conditionally based on make target
debug: CFLAGS += -ggdb
release: CFLAGS += -Ofast -march=native

# build for all possible targets
top: debug

# clean out .o and executable files
clean:
	@rm -f $(TARGETS) $(OBJFILES)

# for each object file: that matches %.o: replace with %.cpp and use stb_image.h
$(OBJFILES): %.o: %.cpp %.h stb_image.h
	$(CC) -c -o $@ $< $(CFLAGS)

# for each target: compile in all dependancies as well as all library link flags we need
$(TARGETS): % : $(OBJFILES)
	$(CC) -o $@ $(LIBS) $^

# for each cpp file: update timestamp, but don't create a file if you can't find it.
%.cpp:
	touch -c $@

%.h:
	touch -c $@

