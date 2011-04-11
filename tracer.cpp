#include <vector>
#include <cstdlib>

#include <assert.h>

#include "tracer.h"
#include "linalg.h"
#include "material.h"
#include "shapes.h"

void Image::blit_to(Glib::RefPtr<Gdk::Pixbuf> &pb, double exposure) {
  assert(width == (unsigned)pb->get_width());
  assert(height == (unsigned)pb->get_height());

  guint8 *pixels = pb->get_pixels();

  for (unsigned int y = 0 ; y < height ; ++y) {
    unsigned int row = y * pb->get_rowstride();
    for (unsigned int x = 0 ; x < width ; ++x) {
      Colour col = data[y * width + x];
      col /= exposure * paints_started;
      Colour col2 = col.to_srgb();
      col = col2.to_byte();
      pixels[row + x * 3] = col.r();
      pixels[row + x * 3 + 1] = col.g();
      pixels[row + x * 3 + 2] = col.b();
    }
  }
}

void Image::blit_variance(Glib::RefPtr<Gdk::Pixbuf> &pb) {
  assert(width == (unsigned)pb->get_width());
  assert(height == (unsigned)pb->get_height());

  guint8 *pixels = pb->get_pixels();

  for (unsigned int y = 0 ; y < height ; ++y) {
    unsigned int row = y * pb->get_rowstride();
    for (unsigned int x = 0 ; x < width ; ++x) {
      double v = variance(x, y);
      Colour col(v, v, v);
      Colour col2 = col.to_srgb();
      col = col2.to_byte();
      pixels[row + x * 3] = col.r();
      pixels[row + x * 3 + 1] = col.g();
      pixels[row + x * 3 + 2] = col.b();
    }
  }
}

Colour Tracer::trace(Ray &ray, int bounces, int maxbounces) {
  //const static int maxbounces = 6;
  if (bounces >= maxbounces) {
    Colour ret(0, 0, 0);
    return ret;
  }

  Hit hitdist;
  Object const *hitobj = 0;
  for (std::vector<Object>::const_iterator i = scene.objects.begin() ;
       i != scene.objects.end() ; ++i) {
    Hit dist = (*i).shape->intersect(ray);
    if (dist.is_hit() && (!hitobj || dist.distance < hitdist.distance)) {
      hitdist = dist;
      hitobj = &(*i);
    }
  }

  if (!hitobj) {
    Colour ret(0, 0, 0);
    return ret;
  }

  Colour ret;
  if (!hitobj->material->colour.is_zero()) {
    Ray newray = hitobj->material->bounce(ray, hitdist.normal, hitdist.distance);
    ret = trace(newray, bounces + 1, maxbounces);
    if (ray.opacity.is_zero() && newray.opacity.is_zero())
      ret *= hitobj->material->colour;
  }
  ret += hitobj->material->emission / (M_PI * M_PI);
  double const d = hitdist.distance;
  ret *= Colour(exp(-ray.opacity.r() * d), exp(-ray.opacity.g() * d), exp(-ray.opacity.b() * d));
  return ret;
}

void Tracer::traceImage(Image &img) {
  double dx = (double)random() / RAND_MAX;
  double dy = (double)random() / RAND_MAX;

  img.paint_start();
  camera.paint_start();
  for (unsigned int y = 0 ; y < img.height ; ++y) {
    for (unsigned int x = 0 ; x < img.width ; ++x) {
      /*
      double variance = img.variance(x, y);
      Ray ray = camera.get_ray((x + dx) / img.width,
			       (y + dy) / img.height);
      Colour col;
      int num_steps = variance * 16 + 1;
      for (int i = 0 ; i < num_steps ; ++i)
	col += trace(ray, 0);
      col /= num_steps;
      img.add(x, y, col);
      */
      Ray ray = camera.get_ray((x + dx) / img.width,
			       (y + dy) / img.height);
      Colour col = trace(ray, 0, 6);
      img(x, y) = col;
    }
  }
}
