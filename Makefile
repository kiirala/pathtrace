CXXFLAGS=`pkg-config --cflags gtkmm-2.4` -g -Wall -Wextra
LDFLAGS=`pkg-config --libs gtkmm-2.4` -g -Wall -Wextra

all: gui

gui: gui.o tracer.o
