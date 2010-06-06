#ifndef PATHTRACE_TRACER_H
#define PATHTRACE_TRACER_H

#include <gtkmm.h>
#include <assert.h>

#include "linalg.h"
#include "material.h"
#include "shapes.h"

class Image {
private:
  Colour *data;
  double *variance_t;
  int paints_started;

public:
  unsigned int width, height;

  void blit_to(Glib::RefPtr<Gdk::Pixbuf> &pb, double exposure);
  void blit_variance(Glib::RefPtr<Gdk::Pixbuf> &pb);

  Image(unsigned int width, unsigned int height)
    : paints_started(0), width(width), height(height)
  {
    data = new Colour[width * height];
    variance_t = new double[width * height];
    for (unsigned int i = 0 ; i < width * height ; ++i)
      variance_t[i] = 1.0;
  }

  const Colour& operator()(unsigned int x, unsigned int y) const {
    assert(x < width && y < height);      
    return data[y * width + x];
  }

  Colour& operator()(unsigned int x, unsigned int y) {
    assert(x < width && y < height);
    return data[y * width + x];
  }

  void add(unsigned int x, unsigned int y, Colour const &col) {
    this->operator()(x, y) += col;
    Colour diff = this->operator()(x, y);
    diff /= paints_started;
    diff -= col;
    variance_t[y * width + x] += fmax(fabs(diff.r()), fmax(fabs(diff.g()), fabs(diff.b())));
  }

  double variance(unsigned int x, unsigned int y) const {
    return variance_t[y * width + x] / paints_started;
  }

  void paint_start() {
    paints_started++;
  }
};

class Object {
public:
  Shape *shape;
  Material *material;

  Object(Shape const &shape, Material const &material)
    : shape(shape.clone()), material(material.clone())
  { }
};

class Scene {
public:
  std::vector<Object> objects;

  void add(Object const &o) {
    objects.push_back(o);
  }
};

class Camera {
private:
  Vector3 origin, topleft, topright, bottomleft;
  Vector3 xd, yd;
  Vector3 dof_origin;
  double plane_x, plane_y;
  double focus, aperture;

public:
  Camera(Vector3 const &origin, Vector3 const &topleft,
	 Vector3 const &topright, Vector3 const &bottomleft,
	 double focus, double aperture)
    : origin(origin), topleft(topleft),
      topright(topright), bottomleft(bottomleft),
      xd(topright - topleft), yd(bottomleft - topleft),
      focus(focus), aperture(aperture)
  { }

  Ray get_ray(double x, double y);
  void paint_start();
};

class Tracer {
private:
  Scene &scene;
  Camera &camera;

public:
  Tracer(Scene &scene, Camera &camera)
    : scene(scene), camera(camera)
  { }

  Colour trace(Ray &ray, int bounces);
  void traceImage(Image &img);
};

/*
Local Variables:
mode:c++
End:
*/
#endif /* PATHTRACE_TRACER_H */
