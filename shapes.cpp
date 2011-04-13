#include <cmath>

#include "shapes.h"
#include "linalg.h"

Hit Sphere::intersect(Ray const &ray) const {
  Vector3 dist = ray.origin - center;
  double a = ray.direction.dot(ray.direction);
  double b = 2 * dist.dot(ray.direction);
  double c = dist.dot(dist) - radius * radius;
  double discr = b * b - 4 * a * c;
  if (discr > 0.0) {
    double distance;
    if ((-b - sqrt(discr)) / (2 * a) < 1e-8)
      distance = (-b + sqrt(discr)) / (2 * a);
    else
      distance = (-b - sqrt(discr)) / (2 * a);
    Vector3 n = ray.origin + ray.direction * distance - center;
    n.normalize();
    return Hit(ray, distance, n);
  }
  return Hit();
}

Sphere* Sphere::clone() const {
  return new Sphere(center, radius);
}

Hit Plane::intersect(Ray const &ray) const {
  double plane_angle = ray.direction.dot(normal);
  if (plane_angle >= 0)
    return Hit();
  Vector3 start_diff = point - ray.origin;
  double dist = normal.dot(start_diff) / plane_angle;
  return Hit(ray, dist, normal);
}

Plane* Plane::clone() const {
  return new Plane(point, normal);
}

Hit Difference::intersect(Ray const &ray) const {
  Hit base_d = base->intersect(ray);
  Hit cut_d = cut->intersect(ray);
  if (base_d.is_hit()) {
    if (!cut_d.is_hit()) {
      return base_d;
    }
    else {
      if (base_d.distance > cut_d.distance) {
	Ray inray(ray.origin + ray.direction * base_d.distance,
		  ray.direction);
	Hit base_d2 = base->intersect(inray);
	Hit cut_d2 = cut->intersect(inray);
	if (!cut_d2.is_hit())
	  return base_d;
	if (base_d2.distance > cut_d2.distance)
	  return Hit(ray, base_d.distance + cut_d2.distance, -cut_d2.normal);
      }
      else {
	return base_d;
      }
    }
  }
  return Hit();
}

Difference* Difference::clone() const {
  return new Difference(*base, *cut);
}
