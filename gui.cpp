#include <cstdio>
#include <ctime>
#include <pthread.h>
#include <errno.h>

#include <gtkmm.h>

#include "tracer.h"
#include "shapes.h"
#include "material.h"

class Workhandler {
private:
  Image buf;
  pthread_t *thread;
  Tracer &tracer;
  Glib::RefPtr<Gdk::Pixbuf> disp;
  double exposure;
  int thread_count;
  bool running;
  pthread_mutex_t buf_mutex;
  
public:
  sigc::signal<void> signal_frame;

  Workhandler(Tracer &tr, Glib::RefPtr<Gdk::Pixbuf> &disp, int threads = 2)
    : buf(disp->get_width(), disp->get_height()), tracer(tr), disp(disp),
      exposure(1.0), thread_count(threads), running(true)
  {
    pthread_mutex_init(&buf_mutex, 0);
    thread = new pthread_t[thread_count];
    for (int i = 0 ; i < thread_count ; ++i) {
      int ret = pthread_create(thread + i, 0, run_renderer,
			       static_cast<void*>(this));
      if (ret != 0) {
	errno = ret;
	perror("Failed to create thread");
      }
    }
  }

  ~Workhandler() {
    stop();
    delete [] thread;
  }

  void stop() {
    running = false;
    for (int i = 0 ; i < thread_count ; ++i) {
      errno = pthread_join(thread[i], 0);
      if (errno != 0)
	perror("Failed to join thread");
    }
    thread_count = 0;
  }

  static void* run_renderer(void *wh_void) {
    Workhandler *wh = static_cast<Workhandler*>(wh_void);
    Image buf(wh->disp->get_width(), wh->disp->get_height());
    Tracer tracer(wh->tracer);
    while (wh->running) {
      tracer.traceImage(buf);
      pthread_mutex_lock(&wh->buf_mutex);
      wh->buf.add(buf);
      wh->buf.blit_to(wh->disp, wh->exposure);
      pthread_mutex_unlock(&wh->buf_mutex);
      wh->signal_frame.emit();
    }
    pthread_exit(0);
    return 0;
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
  time_t elapsed_time;

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
    gdk_threads_leave();
    workhandler->stop();
    gdk_threads_enter();
    return false;
  }

  void on_redraw() {
    image_w.queue_draw();
  }

  void on_step() {
    workhandler->post_step();
    on_frame();
  }

  void on_frame() {
    gdk_threads_enter();
    image_w.queue_draw();
    steps++;
    static char label[64];
    time_t total_time = elapsed_time;
    if (!paused) total_time += time(0) - start_time;
    snprintf(label, 64, "%d steps\n%ld seconds", steps, total_time);
    steps_l.set_text(label);
    gdk_threads_leave();
  }

  void on_pause() {
    paused = !paused;
    if (paused) {
      elapsed_time += time(0) - start_time;
      pause.set_label("Resume");
    }
    else {
      start_time = time(0);
      pause.set_label("Pause");
    }
  }

  void on_change_exposure() {
    double val = exposure_adj.get_value();
    workhandler->change_exposure(val);
    image_w.queue_draw();
  }

  ImageWindow(Glib::RefPtr<Gdk::Pixbuf> &pb, Workhandler *wh)
    : image_pb(pb), workhandler(wh), steps(0), running(true), paused(false),
      start_time(time(0)), elapsed_time(0),
      pause(paused ? "Start" : "Pause"),
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
    workhandler->signal_frame.connect(sigc::mem_fun(*this, &ImageWindow::on_frame));
  }

  virtual ~ImageWindow() {
  }

  void run() {
    while (running) {
      gdk_threads_enter();
      Gtk::Main::iteration();
      gdk_threads_leave();
    }
  }
};

void stop_work() {
  wh->stop();
  delete wh;
}

static void print_help(const char* name) {
  fprintf(stderr,
	  "Usage: %s [-t COUNT] [-s WIDTHxHEIGHT]\n"
	  "    -t: set thread count\n"
	  "    -s: set screen size (e.g. 640x480)",
	  name);
}

int main(int argc, char **argv) {
  g_thread_init(0);
  gdk_threads_init();
  Gtk::Main kit(argc, argv);
  int width = 640;
  int height = 480;

  int threads = 1;
  int opt;
  while ((opt = getopt(argc, argv, "t:s:")) != -1) {
    switch (opt) {
    case 's': {
      int r = sscanf(optarg, "%dx%d", &width, &height);
      if (r == 2)
        break;
      print_help(argv[0]);
      exit(EXIT_FAILURE);      
    }
    case 't': {
      int r = sscanf(optarg, "%d", &threads);
      if (r == 1)
        break;
      print_help(argv[0]);
      exit(EXIT_FAILURE);
    }
    case 'h':
    case '?':
      print_help(argv[0]);
      exit(EXIT_SUCCESS);
    default:
      print_help(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  Scene s;
  s.add(Object(Sphere(Vector3(1.0, 1.6, 0.0), 0.5),
	       Glass(Colour(0.9, .99, .99), 1.5, 0.3)));
  s.add(Object(Difference(Sphere(Vector3(-1.1, 2.8, 0.0), 0.5),
			  Sphere(Vector3(-0.8, 2.6, 0.1), 0.5)),
	       Material(Colour(0.8, 0.8, 0.8), 0.01)));
  for (int i = 0 ; i < 3 ; ++i) {
    s.add(Object(Sphere(Vector3(-1.0 + i * 0.7, 1.4 + i * 0.5, -0.25), 0.25),
		 Material(Colour(0.96, 0.65, 0.55), 0.04)));
  }

  s.add(Object(Plane(Vector3(0.0, 3.5, -0.5), Vector3(0, 0, 1)),
	       Material(Colour(0.9, 0.9, 0.9))));
  s.add(Object(Plane(Vector3(0.0, 4.5, 0.0), Vector3(0, -1, 0)),
	       Material(Colour(0.9, 0.9, 0.9)))); //takaseinä
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
	     4.0, 0.015);
  
  Tracer tr(s, cam);

  Glib::RefPtr<Gdk::Pixbuf> buf = 
    Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, width, height);
  wh = new Workhandler(tr, buf, threads);
  atexit(stop_work);
  ImageWindow window(buf, wh);
  
  //Gtk::Main::run(window);
  window.run();
  
  return 0;
}
