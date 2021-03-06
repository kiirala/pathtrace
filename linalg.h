#ifndef PATHTRACE_LINALG_H
#define PATHTRACE_LINALG_H

#include <cmath>
#include <cstdlib>

struct Vector3 {
  double x, y, z;

  Vector3()
    : x(0.0), y(0.0), z(0.0) { }
  Vector3(double const x, double const y, double const z)
    : x(x), y(y), z(z) { }

  double length() const {
    return sqrt(x * x + y * y + z * z);
  }

  void normalize() {
    double length = sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
  }

  void set(double const x, double const y, double const z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  const Vector3 operator- () const {
    Vector3 res(-x, -y, -z);
    return res;
  }

  const Vector3 operator- (Vector3 const &other) const {
    Vector3 res;
    res.x = x - other.x;
    res.y = y - other.y;
    res.z = z - other.z;
    return res;
  }

  Vector3 operator-= (Vector3 const &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  const Vector3 operator+ (Vector3 const &other) const {
    Vector3 res;
    res.x = x + other.x;
    res.y = y + other.y;
    res.z = z + other.z;
    return res;
  }

  Vector3 operator+= (Vector3 const &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  const Vector3 operator* (double const f) const {
    Vector3 res(x * f, y * f, z * f);
    return res;
  }

  Vector3 operator*= (double const f) {
    this->x *= f;
    this->y *= f;
    this->z *= f;
    return *this;
  }

  Vector3 operator*= (Vector3 const &other) {
    this->x *= other.x;
    this->y *= other.y;
    this->z *= other.z;
    return *this;
  }

  const Vector3 operator/ (double const f) const {
    Vector3 res(x / f, y / f, z / f);
    return res;
  }

  const Vector3 operator/ (Vector3 const other) const {
    Vector3 res(x / other.x, y / other.y, z / other.z);
    return res;
  }

  Vector3 operator/= (double const f) {
    this->x /= f;
    this->y /= f;
    this->z /= f;
    return *this;
  }

  Vector3 operator/= (Vector3 const &other) {
    this->x /= other.x;
    this->y /= other.y;
    this->z /= other.z;
    return *this;
  }

  double dot(Vector3 const &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  const Vector3 cross(Vector3 const &other) const {
    Vector3 res;
    res.x = y * other.z - z * other.y;
    res.y = z * other.x - x * other.z;
    res.z = x * other.y - y * other.x;
    return res;
  }

  const Vector3 at_length(double const new_length) const {
    double const factor = new_length / length();
    Vector3 res = *this * factor;
    return res;
  }

  const Vector3 generate_normal() const {
    Vector3 rand1(1.0, 0.0, 0.0);
    Vector3 rand2(0.0, 1.0, 0.0);
    Vector3 normal;
    if (fabs(this->dot(rand1)) < fabs(this->dot(rand2))) {
      normal = this->cross(rand1);
    }
    else {
      normal = this->cross(rand2);
    }
    return normal;
  }

  bool is_zero() const {
    return x == 0.0 && y == 0.0 && z == 0.0;
  }

  const Vector3 static gaussian(double mean, double variance) {
    /* 1: (sin(rot1), 0, cos(rot1))
     * 2: (sin(rot1) * cos(rot2), sin(rot1) * sin(rot2), cos(rot1)) */
    double u1 = (double)random() / RAND_MAX;
    double u2 = (double)random() / RAND_MAX;
    // Box-Muller transform
    double nat1 = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
    //double nat2 = sqrt(-2 * log(u1)) * sin(2 * M_PI * u2);
    double rot1 = variance * nat1 + mean;
    double rot2 = (double)random() / RAND_MAX * 2 * M_PI;/*var1 * nat1 + mean;*/
    Vector3 ret(sin(rot1) * cos(rot2), sin(rot1) * sin(rot2), cos(rot1));
    return ret;
  }

  const Vector3 static uniform_random() {
    double rot1 = acos((double)random() / RAND_MAX * 2 - 1);
    double rot2 = (double)random() / RAND_MAX * 2 * M_PI;
    Vector3 ret(sin(rot1) * cos(rot2), sin(rot1) * sin(rot2), cos(rot1));
    return ret;
  }
};

struct Colour : public Vector3 {
private:
  static double expose(double const val, double const exposure) {
    return 1.0 - exp(val * -exposure);
  }

  static double to_srgb(double val) {
    double a = 0.055;
    double gamma = 2.4;
    if (val < 0.0031308)
      return 12.92 * val;
    else
      return (1 + a) * pow(val, 1.0 / gamma) - a;
  }

  static double to_byte(double val) {
    val *= 255;
    val = round(val);
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    return val;
  }

public:
  Colour()
    : Vector3()
  { }

  Colour(double r, double g, double b)
    : Vector3(r, g, b)
  { }

  Colour(Vector3 const &v)
    : Vector3(v)
  { }

  double r() const { return x; }
  double g() const { return y; }
  double b() const { return z; }

  const Colour expose(double exposure) const {
    Colour ret(expose(x, exposure), expose(y, exposure), expose(z, exposure));
    return ret;
  }

  const Colour to_srgb() const {
    Colour ret(to_srgb(x), to_srgb(y), to_srgb(z));
    return ret;
  }

  const Colour to_byte() const {
    Colour ret(to_byte(x), to_byte(y), to_byte(z));
    return ret;
  }
};

struct Ray {
  Vector3 origin;
  Vector3 direction;
  double ior;
  Colour opacity;
  Colour filter;
  bool valid;

  Ray(Vector3 const &origin, Vector3 const &direction)
    : origin(origin), direction(direction), ior(1.0), opacity(),
      filter(1.0, 1.0, 1.0), valid(true)
  { }

  Ray(Vector3 const &origin, Vector3 const &direction,
      double const ior, Colour const &opacity)
    : origin(origin), direction(direction), ior(ior), opacity(opacity),
      filter(1.0, 1.0, 1.0), valid(true)
  { }

  Ray(Ray const &other, double const distance, Vector3 const &direction)
    : origin(other.origin + other.direction * distance), direction(direction),
      ior(other.ior), opacity(other.opacity), filter(other.filter),
      valid(other.valid)
  { }

  Ray(Ray const &other, double const distance, Vector3 const &direction,
      double const ior, Colour const &opacity)
    : origin(other.origin + other.direction * distance), direction(direction),
      ior(ior), opacity(opacity), filter(other.filter), valid(other.valid)
  { }

  static Ray InvalidRay() {
    Ray ret(Vector3(0, 0, 0), Vector3(0, 0, 0));
    ret.valid = false;
    return ret;
  }
};

struct Vertex {
  Vector3 loc;
  Vector3 normal;

  Vertex()
    : loc(), normal() { }
  Vertex(Vector3 const &loc, Vector3 const &normal)
    : loc(loc), normal(normal) { }
};

/*
Local Variables:
mode:c++
End:
*/
#endif /* PATHTRACE_LINALG_H */
