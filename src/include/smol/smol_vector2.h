#ifndef SMOL_VECTOR2
#define SMOL_VECTOR2

#include <smol/smol_engine.h>

namespace smol
{
  struct SMOL_ENGINE_API Vector2
  {
    float x;
    float y;

    Vector2();
    Vector2(float xy);
    Vector2(float x, float y);

    Vector2& sum(float f);
    Vector2& sub(float f);
    Vector2& mult(float f);
    Vector2& div(float f);

    Vector2& sum(Vector2& other);
    Vector2& sub(Vector2& other);
    Vector2& mult(Vector2& other);
    Vector2& div(Vector2& other);

    float length();
    Vector2& normalized();
    Vector2& abs();

    float dot(Vector2& other);
    float cross(Vector2& other);
    void set(float x, float y);
  };
}

#endif  // SMOL_VECTOR2
