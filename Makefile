GENERAL=-O0 -g -march=native -Wall -Wextra
CXXFLAGS=`pkg-config --cflags gtkmm-2.4` $(GENERAL) -MMD -MP
LDFLAGS=`pkg-config --libs gtkmm-2.4` $(GENERAL)

OBJECTS=tracer.o material.o shapes.o camera.o
PROGRAMS=gui test

all: $(PROGRAMS)

gui: gui.o $(OBJECTS)

test: test.o $(OBJECTS)

-include $(OBJECTS:.o=.d) gui.d test.d

clean:
	rm -f $(PROGRAMS) $(OBJECTS) $(OBJECTS:.o=.d) *~


