#include <cmath>
#include <cstdlib>

#include "material.h"
#include "linalg.h"

Vector3 Material::bounce(Ray const &ray, Vector3 const &normal) const {
  double theta = fabs(ray.direction.dot(normal));
  Vector3 refl = ray.direction + normal * theta * 2.0;
  refl.normalize();
  Vector3 r_norm1 = refl.generate_normal();
  Vector3 r_norm2 = refl.cross(r_norm1);

  Vector3 dir;
  do {
    Vector3 dir_refl = Vector3::gaussian(0, roughness);
    dir = r_norm1 * dir_refl.y + r_norm2 * dir_refl.x + refl * dir_refl.z;
    //dir.normalize();
  } while (dir.dot(normal) < 0.0);
  return dir;
  /*
  Vector3 vec;
  do {
    vec.set((double)random() / RAND_MAX * 2.0 - 1.0,
	    (double)random() / RAND_MAX * 2.0 - 1.0,
	    (double)random() / RAND_MAX * 2.0 - 1.0);
  } while(vec.length() > 1.0);
  vec.normalize();
  if (vec.dot(normal) < 0.0) {
    vec = -vec;
  }
  return vec;
  */
}

Vector3 Glass::bounce(Ray const &ray, Vector3 const &normal) const {
  double theta1 = fabs(ray.direction.dot(normal));
  double internal_index = ior;
  double external_index = 1.0;
  if (theta1 < 0.0) {
    internal_index = 1.0;
    external_index = ior;
  }

  double eta = external_index / internal_index;

  double theta2 = sqrt(1.0 - eta * eta * (1.0 - theta1 * theta1));
  double rs = (external_index * theta1 - internal_index * theta2) /
    (external_index * theta1 + internal_index * theta2);
  double rp = (internal_index * theta1 - external_index * theta2) /
    (internal_index * theta1 + external_index * theta2);
  double reflectance = rs * rs + rp * rp;
  if ((double)random() / RAND_MAX < reflectance + reflection) {
    Vector3 vec = ray.direction + normal * theta1 * 2.0;
    vec.normalize();
    return vec;
  }
  else {
    Vector3 vec = (ray.direction + normal * theta1) * eta + normal * (-theta2);
    vec.normalize();
    return vec;
  }
}
/*
Vector3 Chrome::bounce(Ray const &ray, Vector3 const &normal) const {
  double theta = fabs(ray.direction.dot(normal));
  Vector3 refl = ray.direction + normal * theta * 2.0;
  refl.normalize();
  return refl;
}
*/
Material* Material::clone() const {
  return new Material(colour, emission, roughness);
}
Material* Glass::clone() const {
  return new Glass(colour, ior, reflection);
}
/*
Material* Chrome::clone() const {
  return new Chrome(colour);
}
*/
