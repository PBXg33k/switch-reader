SOURCES := $(shell find $(SOURCEDIR) -name '*.cpp')
CXX = g++ -std=gnu++17

OUT = test
INCLUDE = ./include

LDLIBS =
LIBS = -ljson-c -lcurl -lSDL2_image -lSDL2_ttf -lSDL2 -lxml2

all: build

build: $(SOURCES)
	$(CXX) -I $(INCLUDE) -o $(OUT) $(LDLIBS) $(SOURCES) $(LIBS)
