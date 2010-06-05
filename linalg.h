#ifndef PATHTRACE_LINALG_H
#define PATHTRACE_LINALG_H

#include <cmath>

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
};

struct Colour : public Vector3 {
private:
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

  double r() const { return x; }
  double g() const { return y; }
  double b() const { return z; }

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

  Ray(Vector3 const &origin, Vector3 const &direction)
    : origin(origin), direction(direction)
  { }
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
