#include <smol/smol.h>
#include <smol/smol_mat4.h>
#include <math.h>

namespace smol
{
  Mat4& Mat4::identity()
  {
    e[0][0] = 1.0f; e[0][1] = 0.0f; e[0][2] = 0.0f; e[0][3] = 0.0f;
    e[1][0] = 0.0f; e[1][1] = 1.0f; e[1][2] = 0.0f; e[1][3] = 0.0f;
    e[2][0] = 0.0f; e[2][1] = 0.0f; e[2][2] = 1.0f; e[2][3] = 0.0f;
    e[3][0] = 0.0f; e[3][1] = 0.0f; e[3][2] = 0.0f; e[3][3] = 1.0f;
    return *this;
  }


    Mat4 Mat4::perspective(float fov, float aspect, float zNear, float zFar)
    {
      const float tanHalfFov = (float) tan(fov/2);
      Mat4 result;
      result.identity();

      result.e[0][0] = 1 / (aspect * tanHalfFov);
      result.e[1][1] = 1 / tanHalfFov;
      result.e[2][2] = - (zFar + zNear) / (zFar - zNear);
      result.e[2][3] = -1;
      result.e[3][2] =  - (2 * zFar * zNear) / (zFar - zNear);
      return result;
    }

    Mat4 Mat4::ortho(float left, float right, float top, float bottom, float zNear, float zFar)
    {
      SMOL_ASSERT(zNear >= 0.0f, "zNear must be positive.");
      SMOL_ASSERT(zFar > 0.0f, "zFar must be positive.");
      SMOL_ASSERT(zFar > zNear, "zFar must be larger than zNear.");

      Mat4 result;
      result.identity();
      result.e[0][0] = 2 / (right - left);
      result.e[1][1] = 2 / (top - bottom);
      result.e[2][2] = - 2 / (zFar - zNear);
      result.e[3][0] = - (right + left) / (right - left);
      result.e[3][1] = - (top + bottom) / (top - bottom);
      result.e[3][2] = - (zFar + zNear) / (zFar - zNear);
      result.e[3][3] = 1;
      return result;
    }
}
