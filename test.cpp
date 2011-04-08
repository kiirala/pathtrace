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
    Vector3 v = Vector3::gaussian(0, 1.0);
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

int main() {
  test_gaussian();
  return 0;
}
