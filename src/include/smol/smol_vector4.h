#ifndef SMOL_VECTOR4
#define SMOL_VECTOR4

#include <smol/smol_engine.h>

namespace smol
{
  struct Vector3;

  struct SMOL_ENGINE_API Vector4
  {
    float x;
    float y;
    float z;
    float w;

    Vector4();
    Vector4(float xyzw);
    Vector4(float x, float y, float z, float w);

    Vector4& sum(float f);
    Vector4& sub(float f);
    Vector4& mult(float f);
    Vector4& div(float f);

    Vector4& sum(Vector4& other);
    Vector4& sub(Vector4& other);
    Vector4& mult(Vector4& other);
    Vector4& div(Vector4& other);

    Vector4& sum(Vector3& other);
    Vector4& sub(Vector3& other);
    Vector4& mult(Vector3& other);
    Vector4& div(Vector3& other);

    float length();
    Vector4& normalized();
    Vector4& abs();

    void set(float x, float y, float z, float w);
  };
}

#endif  // SMOL_VECTOR4
