SOURCES := $(shell find $(SOURCEDIR) -name '*.cpp')
CXX = g++ -std=gnu++17

OUT = test

LDLIBS =
LIBS = -ljson-c -lcurl -lSDL2_image -lboost_regex -lSDL2_ttf -lSDL2

all: build

build: $(SOURCES)
	$(CXX) -o $(OUT) $(LDLIBS) $(SOURCES) ./source/main.c $(LIBS)
