#ifndef PATHTRACE_SHAPES_H
#define PATHTRACE_SHAPES_H

#include "linalg.h"

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

class Plane : public Shape {
private:
  Vector3 point;
  Vector3 normal;
public:
  Plane(Vector3 const &point, Vector3 const &normal)
    : point(point), normal(normal)
  { }
  virtual double intersect(Ray const &ray) const;
  virtual Vector3 get_normal(Vector3 const &pos) const;
  virtual Plane* clone() const;
};

/*
Local Variables:
mode:c++
End:
*/
#endif /* PATHTRACE_SHAPES_H */
