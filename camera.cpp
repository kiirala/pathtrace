#include <cmath>

#include "camera.h"
#include "linalg.h"

Ray Camera::get_ray(double x, double y) {
  Vector3 p = topleft + xd * (x + plane_x) + yd * (y + plane_y);
  Vector3 direction = p - dof_origin;
  direction.normalize();
  return Ray(dof_origin, direction);
}

void Camera::paint_start() {
  double dir = (double)random() / RAND_MAX * M_PI * 2;
  double len = (double)random() / RAND_MAX * aperture;
  double dof_x = len * cos(dir);
  double dof_y = len * sin(dir);
  dof_origin = origin + xd * dof_x + yd * dof_y;
  double plane_dist = ((topleft + xd * 0.5 + yd * 0.5) - origin).length();
  plane_x = dof_x * ((focus - plane_dist) / focus);
  plane_y = dof_y * ((focus - plane_dist) / focus);
}

