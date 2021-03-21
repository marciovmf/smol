#ifndef SMOL_MAT4
#define SMOL_MAT4

#include <smol/smol_engine.h>

namespace smol
{
  struct SMOL_ENGINE_API Mat4
  {
    float e[4][4];
    Mat4& identity();
    static Mat4 perspective(float fov, float aspect, float zNear, float zFar);
    static Mat4 ortho(float left, float right, float top, float bottom, float zNear, float zFar);
  };
}

#endif // SMOL_MAT4
