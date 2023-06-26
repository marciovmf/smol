#include <smol.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector2.h>
#include <math.h>

namespace smol
{
  inline Vector3::Vector3(float x, float y, float z):
    x(x), y(y), z(z) {}

  Vector3::Vector3() {}

  Vector3::Vector3(float xyz):
    x(xyz), y(xyz), z(xyz) {}

  void Vector3::set(float x, float y, float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vector3& Vector3::sum(float f)
  {
    x += f;
    y += f;
    z += f;
    return *this;
  }

  Vector3& Vector3::sub(float f)
  {
    x -= f;
    y -= f;
    z -= f;
    return *this;
  }

  Vector3& Vector3::mult(float f)
  {
    x *= f;
    y *= f;
    z *= f;
    return *this;
  }

  Vector3& Vector3::div(float f)
  {
    x /= f;
    y /= f;
    z /= f;
    return *this;
  }

  Vector3& Vector3::sum(Vector3& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vector3& Vector3::sub(Vector3& other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  Vector3& Vector3::mult(Vector3& other)
  {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
  }

  Vector3& Vector3::div(Vector3& other)
  {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
  }

  Vector3& Vector3::sum(Vector2& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  Vector3& Vector3::sub(Vector2& other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  Vector3& Vector3::mult(Vector2& other)
  {
    x *= other.x;
    y *= other.y;
    return *this;
  }

  Vector3& Vector3::div(Vector2& other)
  {
    x /= other.x;
    y /= other.y;
    return *this;
  }

  float Vector3::length()
  {
    return (float) sqrt(x*x + y*y + z*z);
  }

  Vector3& Vector3::normalized()
  {
    float len = length();
    x /= len;
    y /= len;
    z /= len;
    return *this;
  }

  Vector3& Vector3::abs()
  {
    x = (float)fabs(x);
    y = (float)fabs(y);
    z = (float)fabs(z);
    return *this;
  }

  float Vector3::dot(Vector3& other)
  {
    return x * other.x + y * other.y + z * other.z;
  }

  Vector3 Vector3::cross(Vector3& other)
  {
    Vector3 crossv;
    crossv.x = y * other.z - z * other.y;
    crossv.y = z * other.x - x * other.z;
    crossv.z = x * other.y - y * other.x;
    return crossv;
  }
}
