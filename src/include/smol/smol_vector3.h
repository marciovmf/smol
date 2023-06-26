#ifndef SMOL_VECTOR3
#define SMOL_VECTOR3

#include <smol/smol_engine.h>

namespace smol
{
  struct Vector2;

  struct SMOL_ENGINE_API Vector3
  {
    float x;
    float y;
    float z;

    Vector3();
    Vector3(float xyz);
    Vector3(float x, float y, float z);

    Vector3& sum(float f);
    Vector3& sub(float f);
    Vector3& mult(float f);
    Vector3& div(float f);

    Vector3& sum(Vector3& other);
    Vector3& sub(Vector3& other);
    Vector3& mult(Vector3& other);
    Vector3& div(Vector3& other);

    Vector3& sum(Vector2& other);
    Vector3& sub(Vector2& other);
    Vector3& mult(Vector2& other);
    Vector3& div(Vector2& other);

    float length();
    Vector3& normalized();
    Vector3& abs();

    float dot(Vector3& other);
    Vector3 cross(Vector3& other);
    void set(float x, float y, float z);
  };
}

#endif  // SMOL_VECTOR3
