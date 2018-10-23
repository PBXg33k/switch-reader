SOURCES := $(shell find $(SOURCEDIR) -name '*.cpp')
CXX = g++ -std=gnu++17

OUT = test

LDLIBS =
LIBS = -ljson-c -lcurl -lSDL2_image -lSDL2_ttf -lSDL2

all: build

build: $(SOURCES)
	$(CXX) -o $(OUT) $(LIBS) $(LDLIBS) $(SOURCES) ./source/main.c
