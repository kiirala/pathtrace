GENERAL=-Os -g -march=native -Wall -Wextra
CXXFLAGS=`pkg-config --cflags gtkmm-2.4` $(GENERAL) -MMD -MP
LDFLAGS=`pkg-config --libs gtkmm-2.4` -lpthread $(GENERAL)

OBJECTS=tracer.o material.o shapes.o camera.o
PROGRAMS=gui test

all: $(PROGRAMS)

gui: gui.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LOADLIBES)

test: test.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LOADLIBES)

-include $(OBJECTS:.o=.d) gui.d test.d

clean:
	rm -f $(PROGRAMS) $(OBJECTS) $(OBJECTS:.o=.d) *~


