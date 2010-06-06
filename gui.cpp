#include <cstdio>
#include <ctime>

#include <gtkmm.h>

#include "tracer.h"
#include "shapes.h"
#include "material.h"

class Workhandler {
private:
  Image buf;
  Tracer &tracer;
  Glib::RefPtr<Gdk::Pixbuf> disp;
  double exposure;

public:
  Workhandler(Tracer &tr, Glib::RefPtr<Gdk::Pixbuf> &disp)
    : buf(disp->get_width(), disp->get_height()), tracer(tr), disp(disp),
      exposure(1.0)
  { }

  void stop() {
  }

  void post_step() {
    tracer.traceImage(buf);
    buf.blit_to(disp, exposure);
    //buf.blit_variance(disp);
  }

  void change_exposure(double new_val) {
    exposure = new_val;
    buf.blit_to(disp, exposure);
  }
};
Workhandler *wh;

class ImageWindow : public Gtk::Window {
private:
  Glib::RefPtr<Gdk::Pixbuf> image_pb;
  Workhandler *workhandler;

  int steps;
  bool running, paused;
  time_t start_time;

  Gtk::HBox hsplit;
  Gtk::Image image_w;
  Gtk::VBox tools;
  Gtk::Button pause, step, redraw, quit;
  Gtk::Label steps_l;
  Gtk::Adjustment exposure_adj;
  Gtk::HScale exposure;

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
    snprintf(label, 64, "%d steps\n%ld seconds", steps, time(0) - start_time);
    steps_l.set_text(label);
  }

  void on_pause() {
    paused = !paused;
    if (paused)
      pause.set_label("Resume");
    else
      pause.set_label("Pause");
  }

  void on_change_exposure() {
    double val = exposure_adj.get_value();
    workhandler->change_exposure(val);
    image_w.queue_draw();
  }

  ImageWindow(Glib::RefPtr<Gdk::Pixbuf> &pb, Workhandler *wh)
    : image_pb(pb), workhandler(wh), steps(0), running(true), paused(false),
      start_time(time(0)),
      pause("Pause"),
      step("Step"), redraw("Redraw"), quit("Quit"), steps_l("No steps run\n"),
      exposure_adj(1.0, 0.0, 4.0, 0.01, 0.1, 0.0), exposure(exposure_adj)      
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

    exposure_adj.signal_value_changed().connect(sigc::mem_fun(*this, &ImageWindow::on_change_exposure));
    exposure.set_size_request(200, 50);
    tools.pack_start(exposure, false, true);
    exposure.show();

    quit.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_quit_clicked));
    tools.pack_end(quit, false, true);
    quit.show();
    
    redraw.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_redraw));
    tools.pack_end(redraw, false, true);
    redraw.show();

    step.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_step));
    tools.pack_end(step, false, true);
    step.show();

    pause.signal_clicked().connect(sigc::mem_fun(*this, &ImageWindow::on_pause));
    tools.pack_end(pause, false, true);
    pause.show();

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
      if (!paused)
	on_step();
      else
	Gtk::Main::iteration();
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
  s.add(Object(Sphere(Vector3(1.0, 1.6, 0.0), 0.5),
	       Glass(Colour(1.00, 1.00, 1.00), 1.5, 0.1)));
  s.add(Object(Sphere(Vector3(-1.1, 2.8, 0.0), 0.5),
	       Material(Colour(0.8, 0.8, 0.8), 0.1)));
  for (int i = 0 ; i < 3 ; ++i) {
    s.add(Object(Sphere(Vector3(-1.0 + i * 0.7, 1.4 + i * 0.5, -0.25), 0.25),
		 Material(Colour(0.96, 0.65, 0.55), pow(0.2, 2/*3 - i*/))));
  }

  s.add(Object(Plane(Vector3(0.0, 3.5, -0.5), Vector3(0, 0, 1)),
	       Material(Colour(0.9, 0.9, 0.9))));
  s.add(Object(Plane(Vector3(0.0, 4.5, 0.0), Vector3(0, -1, 0)),
	       Material(Colour(0.9, 0.9, 0.9))));
  s.add(Object(Plane(Vector3(-1.9, 3.5, 0.0), Vector3(1, 0, 0)),
	       Material(Colour(0.9, 0.5, 0.5))));
  s.add(Object(Plane(Vector3(1.9, 3.5, 0.0), Vector3(-1, 0, 0)),
	       Material(Colour(0.5, 0.5, 0.9))));
  s.add(Object(Plane(Vector3(0.0, 0.0, 2.5), Vector3(0, 0, -1)),
	       Material(Colour(0.0, 0.0, 0.0),
			Colour(12.6, 11.6, 10.2))));
  s.add(Object(Plane(Vector3(0.0, -2.5, 0.0), Vector3(0, 1, 0)),
	       Material(Colour(0.9, 0.9, 0.9))));

  Camera cam(Vector3(0.0, -0.5, 0.0),
	     Vector3(-1.3, 1.0, 1.0),
	     Vector3(1.3, 1.0, 1.0),
	     Vector3(-1.3, 1.0, -1.0),
	     4.0, 0.03);
  
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
