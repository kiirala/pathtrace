#ifndef PATHTRACE_MATERIAL_H
#define PATHTRACE_MATERIAL_H

#include "linalg.h"

class Material {
private:
  const static double default_roughness = 1.0;
public:
  Colour colour;
  Colour emission;
  double roughness;
  bool opaque;

  Material(Colour colour)
    : colour(colour), emission(), roughness(default_roughness), opaque(true)
  { }

  Material(Colour colour, Colour emission)
    : colour(colour), emission(emission), roughness(default_roughness), opaque(true)
  { }

  Material(Colour colour, double roughness)
    : colour(colour), emission(), roughness(roughness), opaque(true)
  { }

  Material(Colour colour, Colour emission, double roughness)
    : colour(colour), emission(emission), roughness(roughness), opaque(true)
  { }

  virtual Ray bounce(Ray const &ray, Vector3 const &normal, double const distance) const;
  virtual Material* clone() const;
};

class Glass : public Material {
public:
  double ior;

  Glass(Colour const &col, double ior, double roughness)
    : Material(col, roughness), ior(ior)
  {
    this->opaque = false;
  }
  virtual Ray bounce(Ray const &ray, Vector3 const &normal, double const distance) const;
  virtual Material* clone() const;
};

class Film : public Glass {
public:
  double thickness;

  Film(double thickness, double ior, double roughness)
    : Glass(Colour(1.0, 1.0, 1.0), ior, roughness), thickness(thickness)
  { }

  virtual Ray bounce(Ray const &ray, Vector3 const &normal, double const distance) const;
  virtual Material* clone() const;
};

class Chrome : public Material {
public:
  Chrome(Colour col)
    : Material(col, 0.1)
  { }
  //virtual Vector3 bounce(Ray const &ray, Vector3 const &normal) const;
  //virtual Material* clone() const;
};

/*
Local Variables:
mode:c++
End:
*/
#endif /* PATHTRACE_MATERIAL_H */
