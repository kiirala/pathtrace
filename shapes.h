#ifndef PATHTRACE_SHAPES_H
#define PATHTRACE_SHAPES_H

#include "linalg.h"

class Hit {
public:
  Ray ray;
  double distance;
  Vector3 normal;

  Hit()
    : ray(Vector3(), Vector3()), distance(-1), normal()
  { }

  Hit(Ray const &ray, double const distance, Vector3 const &normal)
    : ray(ray), distance(distance), normal(normal)
  { }

  bool is_hit() {
    return distance > 0;
  }
};

class Shape {
public:
  virtual Hit intersect(Ray const &ray) const = 0;
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
  virtual Hit intersect(Ray const &ray) const;
  virtual Sphere* clone() const;
};

class Plane : public Shape {
private:
  Vector3 point;
  Vector3 normal;
public:
  Plane(Vector3 const &point, Vector3 const &normal)
    : point(point), normal(normal)
  { }
  virtual Hit intersect(Ray const &ray) const;
  virtual Plane* clone() const;
};

class Difference : public Shape {
private:
  Shape *base, *cut;
public:
  Difference(Shape const &base, Shape const &cut)
    : base(base.clone()), cut(cut.clone())
  { }
  virtual Hit intersect(Ray const &ray) const;
  virtual Difference* clone() const;
};

/*
Local Variables:
mode:c++
End:
*/
#endif /* PATHTRACE_SHAPES_H */
