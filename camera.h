#ifndef PATHTRACE_CAMERA_H
#define PATHTRACE_CAMERA_H

#include "linalg.h"

class Camera {
private:
  Vector3 origin, topleft, topright, bottomleft;
  Vector3 xd, yd;
  Vector3 dof_origin;
  double plane_x, plane_y;
  double focus, aperture;

public:
  Camera(Vector3 const &origin, Vector3 const &topleft,
	 Vector3 const &topright, Vector3 const &bottomleft,
	 double focus, double aperture)
    : origin(origin), topleft(topleft),
      topright(topright), bottomleft(bottomleft),
      xd(topright - topleft), yd(bottomleft - topleft),
      focus(focus), aperture(aperture)
  { }

  Ray get_ray(double x, double y);
  void paint_start();
};


/*
Local Variables:
mode:c++
End:
*/
#endif /* PATHTRACE_CAMERA_H */
