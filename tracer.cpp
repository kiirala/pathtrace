#include <vector>
#include <cstdlib>

#include <assert.h>

#include "linalg.h"
#include "tracer.h"

void Image::blit_to(Glib::RefPtr<Gdk::Pixbuf> &pb, double exposure) {
  assert(width == (unsigned)pb->get_width());
  assert(height == (unsigned)pb->get_height());

  guint8 *pixels = pb->get_pixels();

  for (unsigned int y = 0 ; y < height ; ++y) {
    unsigned int row = y * pb->get_rowstride();
    for (unsigned int x = 0 ; x < width ; ++x) {
      Colour col = data[y * width + x];
      col /= exposure;
      Colour col2 = col.to_srgb();
      col = col2.to_byte();
      pixels[row + x * 3] = col.r();
      pixels[row + x * 3 + 1] = col.g();
      pixels[row + x * 3 + 2] = col.b();
    }
  }
}

Vector3 Material::bounce(Ray const &ray, Vector3 const &normal) const {
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

Vector3 Chrome::bounce(Ray const &ray, Vector3 const &normal) const {
  double theta = fabs(ray.direction.dot(normal));
  Vector3 refl = ray.direction + normal * theta * 2.0;
  refl.normalize();
  return refl;
}

Material* Material::clone() const {
  return new Material(colour, emission);
}
Material* Glass::clone() const {
  return new Glass(colour, ior, reflection);
}
Material* Chrome::clone() const {
  return new Chrome(colour);
}

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

Ray Camera::get_ray(double x, double y) {
  Vector3 p = topleft + xd * x + yd * y;
  Vector3 direction = p - origin;
  direction.normalize();
  return Ray(origin, direction);
}

Colour Tracer::trace(Ray &ray, int bounces) {
  if (bounces >= 6) {
    Colour ret(0, 0, 0);
    return ret;
  }

  double hitdist = -1;
  Object const *hitobj = 0;
  for (std::vector<Object>::const_iterator i = scene.objects.begin() ;
       i != scene.objects.end() ; ++i) {
    double dist = (*i).shape->intersect(ray);
    if (dist > 1e-4 && (!hitobj || dist < hitdist)) {
      hitdist = dist;
      hitobj = &(*i);
    }
  }

  if (!hitobj) {
    Colour ret(0, 0, 0);
    return ret;
  }

  Vector3 point = ray.origin + ray.direction * hitdist;
  Vector3 normal = hitobj->shape->get_normal(point);
  Vector3 direction = hitobj->material->bounce(ray, normal);
  Ray newray(point, direction);
  Colour ret = trace(newray, bounces + 1);
  ret *= hitobj->material->colour;
  ret += hitobj->material->emission;
  return ret;
}

void Tracer::traceImage(Image &img) {
  for (unsigned int y = 0 ; y < img.height ; ++y) {
    for (unsigned int x = 0 ; x < img.width ; ++x) {
      Ray ray = camera.get_ray((double)x / img.width, (double)y / img.height);
      Colour col = trace(ray, 0);
      img(x, y) += col;
    }
  }
}
