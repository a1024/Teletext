# -*- Makefile -*-
PROGRAM = bin/Teletext
SOURCES = $(wildcard src/*.cpp)
CXXFLAGS = `pkg-config --cflags gtk+-3.0`
LIBS = -lSDL2 -lGL -lGLU `pkg-config --libs gtk+-3.0`

debug:	CXXFLAGS += -g -D_DEBUG
debug:	$(PROGRAM)

profile:	CXXFLAGS += -O -pg -D_PROFILE
profile:	$(PROGRAM)

result:	gmon.out
	gprof bin/Teletext gmon.out > profiler_results.txt

release:	CXXFLAGS += -O
release:	$(PROGRAM)

$(PROGRAM):	$(SOURCES)
	$(CXX) -no-pie $(CXXFLAGS) -march=core2 $(SOURCES) $(LIBS) -o $(PROGRAM)

clean:
	rm $(PROGRAM)
