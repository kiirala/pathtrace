GENERAL=-Os -g -march=native -Wall -Wextra
CXXFLAGS=`pkg-config --cflags gtkmm-2.4` $(GENERAL) -MMD -MP
LDFLAGS=`pkg-config --libs gtkmm-2.4` $(GENERAL)

OBJECTS=gui.o tracer.o material.o shapes.o
PROGRAM=gui

all: $(PROGRAM)

gui: $(OBJECTS)

-include $(OBJECTS:.o=.d)

clean:
	rm -f $(PROGRAM) $(OBJECTS) $(OBJECTS:.o=.d) *~


