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

  Material(Colour colour)
    : colour(colour), emission(), roughness(default_roughness)
  { }

  Material(Colour colour, Colour emission)
    : colour(colour), emission(emission), roughness(default_roughness)
  { }

  Material(Colour colour, double roughness)
    : colour(colour), emission(), roughness(roughness)
  { }

  Material(Colour colour, Colour emission, double roughness)
    : colour(colour), emission(emission), roughness(roughness)
  { }

  virtual Vector3 bounce(Ray const &ray, Vector3 const &normal) const;
  virtual Material* clone() const;
};

class Glass : public Material {
private:
  double ior;
  double reflection;

public:
  Glass(Colour col, double ior, double reflection)
    : Material(col), ior(ior), reflection(reflection)
  { }
  virtual Vector3 bounce(Ray const &ray, Vector3 const &normal) const;
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
