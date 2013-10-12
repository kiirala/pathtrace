#include <cmath>
#include <cstdlib>

#include "material.h"
#include "linalg.h"

Ray Material::bounce(Ray const &ray, Vector3 const &normal, double const distance) const {
  Vector3 tangent = normal.generate_normal();
  Vector3 bitangent = normal.cross(tangent);
  Vector3 g = Vector3::gaussian(0, roughness);
  //if (g.z < 0) g = -g;
  Vector3 my_n = tangent * g.x + bitangent * g.y + normal * g.z;
  if (ray.direction.dot(my_n) * ray.direction.dot(normal) < 0) {
    //my_n = -my_n;
    return Ray::InvalidRay();
    //my_n = tangent * (-g.x) + bitangent * (-g.y) + normal * g.z;
  }

  double theta = -ray.direction.dot(my_n);
  Vector3 refl = ray.direction + my_n * theta * 2.0;
  refl.normalize();

  Ray ret(ray, distance, refl);
  return ret;
}

Ray Glass::bounce(Ray const &ray, Vector3 const &smooth_normal, double const distance) const {
  Vector3 tangent = smooth_normal.generate_normal();
  Vector3 bitangent = smooth_normal.cross(tangent);
  Vector3 g = Vector3::gaussian(0, roughness);
  Vector3 normal = tangent * g.x + bitangent * g.y + smooth_normal * g.z;
  if (ray.direction.dot(normal) * ray.direction.dot(smooth_normal) < 0) {
    normal = -normal;
  }

  double theta1 = -ray.direction.dot(normal);
  double index_before = 1.0;
  double index_after = ior;
  Colour internal_opacity(-log(colour.r()), -log(colour.g()), -log(colour.b()));

  if (theta1 < 0.0) {
    index_before = ior;
    index_after = 1.0;
    internal_opacity.set(0.0, 0.0, 0.0);
  }

  double eta = index_before / index_after;

  // Snell's Law
  double theta2sq = 1.0 - eta * eta * (1.0 - theta1 * theta1);
  double theta2 = sqrt(theta2sq);
  // Fresnel Equations
  double rs = (index_before * fabs(theta1) - index_after * theta2) /
    (index_before * fabs(theta1) + index_after * theta2);
  double rp = (index_after * fabs(theta1) - index_before * theta2) /
    (index_after * fabs(theta1) + index_before * theta2);
  double reflectance = (rs * rs + rp * rp) / 2;

  if (theta2sq <= 0 || (double)random() / RAND_MAX < reflectance) {
    Vector3 vec = ray.direction + normal * theta1 * 2.0;
    vec.normalize();
    Ray ret(ray, distance, vec);
    return ret;
  } else {
    if (theta2sq <= 0) return Ray::InvalidRay();
    Vector3 vec;
    if (theta1 > 0)
      vec = ray.direction * eta + normal * (eta * theta1 - theta2);
    else
      vec = ray.direction * eta + normal * (eta * theta1 + theta2);
    vec.normalize();
    Ray ret(ray, distance, vec, index_after, internal_opacity);
    return ret;
  }
}

static double refracted_angle(double index_before, double index_after, double theta1) {
  double eta = index_before / index_after;
  // Snell's Law
  return sqrt(1.0 - eta * eta * (1.0 - theta1 * theta1));
}

static double reflectance(double index_before, double index_after, double theta1) {
  double extra_refl = 0.2;
  double eta = index_before / index_after;
  // Snell's Law
  double theta2sq = 1.0 - eta * eta * (1.0 - theta1 * theta1);
  if (theta2sq <= 0) return 1.0;
  double theta2 = sqrt(theta2sq);
  // Fresnel Equations
  double rs = (index_before * fabs(theta1) - index_after * theta2) /
    (index_before * fabs(theta1) + index_after * theta2);
  double rp = (index_after * fabs(theta1) - index_before * theta2) /
    (index_after * fabs(theta1) + index_before * theta2);
  return ((extra_refl + (rs * rs + rp * rp) / 2)) / (1 + extra_refl);
}

static double interference(double wavelength, double distance) {
  return (1.0 + cos(distance / wavelength * 2 * M_PI)) / 2.0;
}

static double lerp(double a, double b, double pos) {
  return a * (1 - pos) + b * pos;
}

Ray Film::bounce(Ray const &ray, Vector3 const &smooth_normal, double const distance) const {
  /*
  Vector3 tangent = smooth_normal.generate_normal();
  Vector3 bitangent = smooth_normal.cross(tangent);
  Vector3 g = Vector3::gaussian(0, roughness);
  Vector3 normal = tangent * g.x + bitangent * g.y + smooth_normal * g.z;
  if (ray.direction.dot(normal) * ray.direction.dot(smooth_normal) < 0) {
    normal = -normal;
  }
  */
  Vector3 normal = smooth_normal;

  double theta1 = -ray.direction.dot(normal);
  double index_before = 1.0;
  double index_after = ior;

  double refl_outside = reflectance(index_before, index_after, theta1);
  double theta2 = refracted_angle(index_before, index_after, fabs(theta1));
  double refl_inside = reflectance(index_after, index_before, theta2);
  double d = 2.0 * thickness / fabs(theta2);

  if ((double)random() / RAND_MAX < 2 * refl_outside / (1 + refl_outside)) {
    Vector3 vec = ray.direction + normal * theta1 * 2.0;
    vec.normalize();
    Ray ret(ray, distance, vec);
    ret.filter = Colour(lerp(1.0, interference(640e-9, d), refl_inside),
			lerp(1.0, interference(540e-9, d), refl_inside),
			lerp(1.0, interference(450e-9, d), refl_inside));
    return ret;
  } else {
    double eta = index_before / index_after;
    Vector3 vec;
    if (theta1 > 0)
      vec = ray.direction * eta + normal * (eta * theta1 - theta2);
    else
      vec = ray.direction * eta + normal * (eta * theta1 + theta2);
    vec.normalize();
    double theta3 = refracted_angle(index_after, index_before, fabs(theta2));
    Vector3 vec2;
    if (theta2 > 0)
      vec2 = vec / eta + normal * (theta2 / eta - theta3);
    else
      vec2 = vec / eta + normal * (theta2 / eta + theta3);

    Ray ret(ray, distance, vec2);
    double r_in = refl_inside * refl_inside;
    ret.filter = Colour(lerp(1.0, interference(640e-9, d), r_in),
			lerp(1.0, interference(540e-9, d), r_in),
			lerp(1.0, interference(450e-9, d), r_in));
    return ret;
  }
}

Material* Material::clone() const {
  return new Material(*this);
}
Material* Glass::clone() const {
  return new Glass(*this);
}
Material* Film::clone() const {
  return new Film(*this);
}
