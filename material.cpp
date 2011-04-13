#include <cmath>
#include <cstdlib>

#include "material.h"
#include "linalg.h"

Ray Material::bounce(Ray const &ray, Vector3 const &normal, double const distance) const {
  Vector3 tangent = normal.generate_normal();
  Vector3 bitangent = normal.cross(tangent);
  Vector3 g = Vector3::gaussian(0, roughness);
  if (g.z < 0) g = -g;
  Vector3 my_n = tangent * g.x + bitangent * g.y + normal * g.z;
  if (my_n.dot(ray.direction) > 0) {
    my_n = tangent * (-g.x) + bitangent * (-g.y) + normal * g.z;    
  }

  double theta = fabs(ray.direction.dot(my_n));
  Vector3 refl = ray.direction + my_n * theta * 2.0;
  refl.normalize();

  Ray ret(ray, distance, refl);
  return ret;
}

Ray Glass::bounce(Ray const &ray, Vector3 const &normal, double const distance) const {
  double theta1 = -ray.direction.dot(normal);
  double internal_index = ior;
  double external_index = ray.ior;
  //Colour internal_opacity(Vector3(1.0, 1.0, 1.0) / colour - Vector3(1.0, 1.0, 1.0));
  Colour internal_opacity(-log(colour.r()), -log(colour.g()), -log(colour.b()));

  if (theta1 < 0.0) {
    internal_index = 1.0;
    external_index = ior;
    internal_opacity.set(0.0, 0.0, 0.0);
  }
  //theta1 = fabs(theta1);

  double eta = external_index / internal_index;

  // Snell's Law
  double theta2sq = 1.0 - eta * eta * (1.0 - theta1 * theta1);
  double theta2 = sqrt(theta2sq);
  // Fresnell Equations
  double rs = (external_index * fabs(theta1) - internal_index * theta2) /
    (external_index * fabs(theta1) + internal_index * theta2);
  double rp = (internal_index * fabs(theta1) - external_index * theta2) /
    (internal_index * fabs(theta1) + external_index * theta2);
  double reflectance = (rs * rs + rp * rp) / 2;
  if (theta2sq <= 0 || (double)random() / RAND_MAX < reflectance + (1 - reflectance) * reflection) {
    Vector3 vec = ray.direction + normal * theta1 * 2.0;
    vec.normalize();
    Ray ret(ray, distance, vec);
    return ret;
  }
  else {
    Vector3 vec;
    if (theta1 < 0)
      vec = ray.direction * eta + normal * (eta * theta1 - theta2);
    else
      vec = ray.direction * eta + normal * (eta * theta1 + theta2);
    vec.normalize();
    Ray ret(ray, distance, vec, internal_index, internal_opacity);
    return ret;
  }
}

Material* Material::clone() const {
  return new Material(*this);
}
Material* Glass::clone() const {
  return new Glass(*this);
}
