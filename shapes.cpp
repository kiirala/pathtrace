#include <cmath>

#include "shapes.h"
#include "linalg.h"

double Sphere::intersect(Ray const &ray) const {
  Vector3 dist = ray.origin - center;
  double b = dist.dot(ray.direction);
  double c = dist.dot(dist) - radius * radius;
  double d = b * b - c;
  if (d > 0.0) {
    return -b -sqrt(d);
  }
  return -1;
}

Vector3 Sphere::get_normal(Vector3 const &pos) const {
  Vector3 n = pos - center;
  n.normalize();
  return n;
}

Sphere* Sphere::clone() const {
  return new Sphere(center, radius);
}

double Plane::intersect(Ray const &ray) const {
  double plane_angle = ray.direction.dot(normal);
  if (fabs(plane_angle) < 1e-4)
    return -1;
  Vector3 start_diff = point - ray.origin;
  double dist = normal.dot(start_diff) / plane_angle;
  return dist;
}

Vector3 Plane::get_normal(Vector3 const &/*pos*/) const {
  return normal;
}

Plane* Plane::clone() const {
  return new Plane(point, normal);
}
