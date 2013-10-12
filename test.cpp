#include <cstdio>
#include <cmath>

#include "linalg.h"

class Histogram {
  struct Bucket {
    int count;
    double min, max;
  };

  Bucket *buckets;
  int count;
  double min, max;

public:
  Histogram(double min, double max, int count)
    : count(count), min(min), max(max)
  {
    buckets = new Bucket[count];
    double d = max - min;
    for (int i = 0 ; i < count ; ++i) {
      buckets[i].count = 0;
      buckets[i].min = min + i * d / count;
      buckets[i].max = min + (i + 1) * d / count;
    }
  }

  void add(double val, int count) {
    if (val < min || val > max)
      return;
    int pos = 0;
    while (buckets[pos].max < val) ++pos;
    buckets[pos].count += count;
  }

  void print() {
    int max = 0;
    for (int i = 0 ; i < count ; ++i) {
      if (buckets[i].count > max)
	max = buckets[i].count;
    }

    double scale = fmin(1.0, 60.0 / max);
    for (int i = 0 ; i < count ; ++i) {
      printf("%8.2f ", (buckets[i].min + buckets[i].max) / 2.0);
      for (int foo = 0 ; foo < round(buckets[i].count * scale) ; ++foo)
	printf("*");
      printf("\n");
    }
    printf("\n");
  }
};

void test_gaussian() {
  Histogram x = Histogram(-1, 1, 25);
  Histogram y = Histogram(-1, 1, 25);
  Histogram z = Histogram(-1, 1, 25);
  Histogram len = Histogram(0, 1, 25);

  for (int i = 0 ; i < 256 * 256 ; ++i) {
    Vector3 v = Vector3::gaussian(0, 0.5);
    x.add(v.x, 1);
    y.add(v.y, 1);
    z.add(v.z, 1);
    len.add(sqrt(v.x * v.x + v.y * v.y), 1);
  }

  x.print();
  y.print();
  z.print();
  len.print();
}

void test_fresnel() {
  double external_index = 1.5;
  double internal_index = 1.0;
  double eta = external_index / internal_index;

  for (double angle = -M_PI / 2; angle <= M_PI / 2; angle += M_PI / 16) {
    double theta1 = cos(angle);
    // Snell's Law
    double theta2sq = 1.0 - eta * eta * (1.0 - theta1 * theta1);
    double theta2 = sqrt(theta2sq);
    // Fresnell Equations
    double rs = (external_index * fabs(theta1) - internal_index * theta2) /
      (external_index * fabs(theta1) + internal_index * theta2);
    double rp = (internal_index * fabs(theta1) - external_index * theta2) /
      (internal_index * fabs(theta1) + external_index * theta2);
    double reflectance = (rs * rs + rp * rp) / 2;
    printf("%5.2f -> %6.4f\n", angle, reflectance);
  }
}

int main() {
  test_gaussian();
  test_fresnel();
  return 0;
}
