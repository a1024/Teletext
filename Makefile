# -*- Makefile -*-
PROGRAM = bin/Teletext
SOURCES = $(wildcard src/*.cpp)
CXXFLAGS = `pkg-config --cflags gtk+-3.0`
LIBS = -lSDL2 -lGL -lGLU `pkg-config --libs gtk+-3.0`

debug:	CXXFLAGS += -g -D_DEBUG
debug:	$(PROGRAM)

release:	CXXFLAGS += -O
release:	$(PROGRAM)

$(PROGRAM):	$(SOURCES)
	$(CXX) -no-pie $(CXXFLAGS) -march=core2 $(SOURCES) $(LIBS) -o $(PROGRAM)

clean:
	rm $(PROGRAM)
