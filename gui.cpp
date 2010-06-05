#include <cstdio>

#include <gtkmm.h>

#include "tracer.h"

class Workhandler {
private:
  Image buf;
  Tracer &tracer;
  Glib::RefPtr<Gdk::Pixbuf> disp;

public:
  Workhandler(Tracer &tr, Glib::RefPtr<Gdk::Pixbuf> &disp)
    : buf(disp->get_width(), disp->get_height()), tracer(tr), disp(disp)
  { }

  void stop() {
  }

  void post_step() {
    static double steps = 1.0;
    tracer.traceImage(buf);
    buf.blit_to(disp, steps);
    steps += 1.0;
  }
};
Workhandler *wh;

class ImageWindow : public Gtk::Window {
private:
  Glib::RefPtr<Gdk::Pixbuf> image_pb;
  Workhandler *workhandler;

  int steps;
  bool running;

  Gtk::HBox hsplit;
  Gtk::Image image_w;
  Gtk::VBox tools;
  Gtk::Button step, redraw, quit;
  Gtk::Label steps_l;

  /*
  int on_destroy() {
    workhandler->stop();
    Gtk::Main::quit();
    return 0;
  }
  */
public:
  void on_quit_clicked() {
    on_quit();
  }

  bool on_quit() {
    running = false;
    return false;
  }

  void on_redraw() {
    image_w.queue_draw();
  }

  void on_step() {
    workhandler->post_step();
    image_w.queue_draw();
    steps++;
    static char label[64];
    snprintf(label, 64, "%d steps", steps);
    steps_l.set_text(label);
  }

  ImageWindow(Glib::RefPtr<Gdk::Pixbuf> &pb, Workhandler *wh)
    : image_pb(pb), workhandler(wh), steps(0), running(true),
      step("Step"), redraw("Redraw"), quit("Quit"), steps_l("No steps run")
      
  {
    set_border_width(10);
    set_title("Path Tracing");

    add(hsplit);

    image_w.set(image_pb);
    hsplit.add(image_w);
    image_w.show();

    hsplit.add(tools);

    tools.pack_start(steps_l, false, true);
    steps_l.show();

    quit.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_quit_clicked));
    tools.pack_end(quit, false, true);
    quit.show();
    
    redraw.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_redraw));
    tools.pack_end(redraw, false, true);
    redraw.show();

    step.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_step));
    tools.pack_end(step, false, true);
    step.show();

    Gtk::Main::signal_quit().connect(sigc::mem_fun(*this, &ImageWindow::on_quit));

    tools.show();
    hsplit.show();
    show();
  }

  virtual ~ImageWindow() {
  }

  void run() {
    while (running) {
      while( Gtk::Main::events_pending() )
	Gtk::Main::iteration();
      on_step();
    }
  }
};

void stop_work() {
  wh->stop();
  delete wh;
}

int main(int argc, char **argv) {
  Gtk::Main kit(argc, argv);
  int width = 320;
  int height = 240;

  Scene s;
  s.add(Object(Sphere(Vector3(1.0, 2.0, 0.0), 0.5),
	       Glass(Colour(1.00, 1.00, 1.00), 1.5, 0.1)));
  s.add(Object(Sphere(Vector3(-1.1, 2.8, 0.0), 0.5),
	       Chrome(Colour(0.8, 0.8, 0.8))));

  s.add(Object(Sphere(Vector3(0.0, 3.5, -10e6), 10e6-0.5),
	       Material(Colour(0.9, 0.9, 0.9))));
  s.add(Object(Sphere(Vector3(0.0, 10e6, 0.0), 10e6-4.5),
	       Material(Colour(0.9, 0.9, 0.9))));
  s.add(Object(Sphere(Vector3(-10e6, 3.5, 0.0), 10e6-1.9),
	       Material(Colour(0.9, 0.5, 0.5))));
  s.add(Object(Sphere(Vector3(10e6, 3.5, 0.0), 10e6-1.9),
	       Material(Colour(0.5, 0.5, 0.9))));
  s.add(Object(Sphere(Vector3(0.0, 0.0, 10e6), 10e6-2.5),
	       Material(Colour(0.0, 0.0, 0.0),
			Colour(1.6, 1.47, 1.29))));
  s.add(Object(Sphere(Vector3(0.0, -10e6, 0.0), 10e6-2.5),
	       Material(Colour(0.9, 0.9, 0.9))));

  Camera cam(Vector3(0.0, -0.5, 0.0),
	     Vector3(-1.3, 1.0, 1.0),
	     Vector3(1.3, 1.0, 1.0),
	     Vector3(-1.3, 1.0, -1.0));
  
  Tracer tr(s, cam);

  Glib::RefPtr<Gdk::Pixbuf> buf = 
    Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, width, height);
  wh = new Workhandler(tr, buf);
  atexit(stop_work);
  ImageWindow window(buf, wh);
  
  //Gtk::Main::run(window);
  window.run();
  
  return 0;
}

/*
import pygtk
pygtk.require('2.0')
import gtk
import numpy
import time
import threading

import scene
import painter
import tracer
import camera
import workqueue

class ImageWindow:
    def do_step(self, widget, data=None):
        self.workhandler.step()

    def do_redraw(self, widget, data=None):
        self.image_w.queue_draw()

    def change_range(self, adj):
        self.workhandler.queue.put('paint', adj.value)

    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        self.workhandler.quit()
        #gtk.main_quit()
        self.running = False

    def __init__(self, image, workhandler):
        self.image_pb = image
        self.workhandler = workhandler
        self.running = True

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
        self.window.set_border_width(10)

        self.hsplit = gtk.HBox()
        self.window.add(self.hsplit)

        self.image_w = gtk.Image()
        self.image_w.set_from_pixbuf(self.image_pb)
        self.hsplit.add(self.image_w)
        self.image_w.show()

        self.tools = gtk.VBox()
        self.hsplit.add(self.tools)

        self.button = gtk.Button("Quit")
        self.button.connect_object("clicked", gtk.Widget.destroy, self.window)
        self.tools.pack_end(self.button, expand=False)
        self.button.show()

        self.redraw = gtk.Button("Redraw")
        self.redraw.connect('clicked', self.do_redraw, None)
        self.tools.pack_end(self.redraw, expand=False)
        self.redraw.show()

        self.step = gtk.Button("Step")
        self.step.connect('clicked', self.do_step, None)
        self.tools.pack_end(self.step, expand=False)
        self.step.show()

        self.colrange_adj = gtk.Adjustment(1.0, 0.0, 10.0, 0.05)
        self.colrange_adj.connect('value_changed', self.change_range)
        self.colrange = gtk.HScale(self.colrange_adj)
        self.colrange.set_size_request(200, 50)
        self.tools.pack_start(self.colrange, expand=False)
        self.colrange.show()

        self.tools.show()
        self.hsplit.show()
        self.window.show()

    def main(self):
        #gtk.main()
        while self.running:
            gtk.main_iteration(block=False)
            time.sleep(0.1)

if __name__ == '__main__':
    class Workhandler:
        def step(self):
            pb = numpy.zeros((self.size[1], self.size[0], 3), float)
            self.queue.put('trace', pb)
            print "Step put in"

        def quit(self):
            self.killswitch.set()
            for tr in self.tracers:
                self.queue.put('trace', None)
            self.queue.put('paint', None)

            for tr in self.tracers:
                if tr.is_alive():
                    tr.join()
            if self.painter.is_alive():
                self.painter.join()

        def __init__(self, scene, camera, pixbuf):
            self.size = camera.viewport_size
            self.queue = workqueue.WorkQueue()
            self.killswitch = threading.Event()
            self.killswitch.clear()
            self.tracers = [tracer.Tracer(self.queue, self.killswitch,
                                          camera, scene) for x in range(4)]
            self.painter = painter.Painter(self.queue, self.killswitch,
                                           pixbuf)
            for tr in self.tracers:
                tr.start()
            self.painter.start()

    size = (320, 240)

    s = scene.Scene()
    s.add(scene.Object(scene.Sphere((1.0, 2.0, 0.0), 0.5),
                       scene.Glass((1.00, 1.00, 1.00), 1.5, 0.1)))
    s.add(scene.Object(scene.Sphere((-1.1, 2.8, 0.0), 0.5),
                       scene.Chrome((0.8, 0.8, 0.8))))

    s.add(scene.Object(scene.Sphere((0.0, 3.5, -10e6), 10e6-0.5),
                       scene.Material((0.9, 0.9, 0.9))))
    s.add(scene.Object(scene.Sphere((0.0, 10e6, 0.0), 10e6-4.5),
                       scene.Material((0.9, 0.9, 0.9))))
    s.add(scene.Object(scene.Sphere((-10e6, 3.5, 0.0), 10e6-1.9),
                       scene.Material((0.9, 0.5, 0.5))))
    s.add(scene.Object(scene.Sphere((10e6, 3.5, 0.0), 10e6-1.9),
                       scene.Material((0.5, 0.5, 0.9))))
    s.add(scene.Object(scene.Sphere((0.0, 0.0, 10e6), 10e6-2.5),
                       scene.Material((0.0, 0.0, 0.0),
                                      (1.6, 1.47, 1.29))))
    s.add(scene.Object(scene.Sphere((0.0, -10e6, 0.0), 10e6-2.5),
                       scene.Material((0.9, 0.9, 0.9))))

    image_pb = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, False, 8,
                              size[0], size[1])

    cam = camera.Camera(size,
                        (0.0, -0.5, 0.0),
                        (-1.3, 1.0, 1.0),
                        (1.3, 1.0, 1.0),
                        (-1.3, 1.0, -1.0))

    wh = Workhandler(s, cam, image_pb)

    iw = ImageWindow(image_pb, wh)
    iw.main()
*/
