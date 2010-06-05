#ifndef PATHTRACE_TRACER_H
#define PATHTRACE_TRACER_H

#include <gtkmm.h>
#include <assert.h>

#include "linalg.h"

class Image {
private:
  Colour *data;

public:
  unsigned int width, height;

  void blit_to(Glib::RefPtr<Gdk::Pixbuf> &pb, double exposure);

  Image(unsigned int width, unsigned int height)
    : width(width), height(height)
  {
    data = new Colour[width * height];
  }

  const Colour& operator()(unsigned int x, unsigned int y) const {
    assert(x < width && y < height);      
    return data[y * width + x];
  }

  Colour& operator()(unsigned int x, unsigned int y) {
    assert(x < width && y < height);
    return data[y * width + x];
  }
};

class Shape {
public:
  virtual double intersect(Ray const &ray) const = 0;
  virtual Vector3 get_normal(Vector3 const &pos) const = 0;
  virtual Shape* clone() const = 0;
};

class Sphere : public Shape {
private:
  Vector3 center;
  double radius;
public:
  Sphere(Vector3 const &center, double const radius)
    : center(center), radius(radius)
  { }
  virtual double intersect(Ray const &ray) const;
  virtual Vector3 get_normal(Vector3 const &pos) const;
  virtual Sphere* clone() const;
};

class Material {
public:
  Colour colour;
  Colour emission;

  Material(Colour colour)
    : colour(colour), emission()
  { }

  Material(Colour colour, Colour emission)
    : colour(colour), emission(emission)
  { }

  virtual Vector3 bounce(Ray const &ray, Vector3 const &normal) const;
  virtual Material* clone() const;
};

class Glass : public Material {
private:
  double ior;
  double reflection;

public:
  Glass(Colour col, double ior, double reflection)
    : Material(col), ior(ior), reflection(reflection)
  { }
  virtual Vector3 bounce(Ray const &ray, Vector3 const &normal) const;
  virtual Material* clone() const;
};

class Chrome : public Material {
public:
  Chrome(Colour col)
    : Material(col)
  { }
  virtual Vector3 bounce(Ray const &ray, Vector3 const &normal) const;
  virtual Material* clone() const;
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

public:
  Camera(Vector3 const &origin, Vector3 const &topleft,
	 Vector3 const &topright, Vector3 const &bottomleft)
    : origin(origin), topleft(topleft),
      topright(topright), bottomleft(bottomleft),
      xd(topright - topleft), yd(bottomleft - topleft)
  { }

  Ray get_ray(double x, double y);
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
