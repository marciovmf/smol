#include <smol/smol_vector2.h>
#include <math.h>

namespace smol
{
  Vector2::Vector2(float x, float y):
    x(x), y(y) {}

  Vector2::Vector2(): x(0), y(0) {}

  void Vector2::set(float x, float y)
  {
    x = x;
    y = y;
  }

  Vector2& Vector2::sum(float f)
  {
    x += f;
    y += f;
    return *this;
  }

  Vector2& Vector2::sub(float f)
  {
    x -= f;
    y -= f;
    return *this;
  }

  Vector2& Vector2::mult(float f)
  {
    x *= f;
    y *= f;
    return *this;
  }

  Vector2& Vector2::div(float f)
  {
    x /= f;
    y /= f;
    return *this;
  }

  Vector2& Vector2::sum(Vector2& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  Vector2& Vector2::sub(Vector2& other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  Vector2& Vector2::mult(Vector2& other)
  {
    x *= other.x;
    y *= other.y;
    return *this;
  }

  Vector2& Vector2::div(Vector2& other)
  {
    x /= other.x;
    y /= other.y;
    return *this;
  }

  float Vector2::length()
  {
    return (float) sqrt(x*x + y*y);
  }

  Vector2& Vector2::normalized()
  {
    float len = length();
    x /= len;
    y /= len;
    return *this;
  }

  Vector2& Vector2::abs()
  {
    x = (float)fabs(x);
    y = (float)fabs(y);
    return *this;
  }

  float Vector2::dot(Vector2& other)
  {
    return x * other.x + y * other.y;
  }

  float Vector2::cross(Vector2& other)
  {
    return (float)(x * other.y - y * other.x);
  }
}
