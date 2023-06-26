#include <smol.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector4.h>
#include <math.h>

namespace smol
{

  inline Vector4::Vector4(float xyzw):
    x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}

  inline Vector4::Vector4(float x, float y, float z, float w):
    x(x), y(y), z(z), w(w){}

  Vector4::Vector4() {}

  void Vector4::set(float x, float y, float z, float w)
  {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  Vector4& Vector4::sum(float f)
  {
    x += f;
    y += f;
    z += f;
    w += f;
    return *this;
  }

  Vector4& Vector4::sub(float f)
  {
    x -= f;
    y -= f;
    z -= f;
    w -= f;
    return *this;
  }

  Vector4& Vector4::mult(float f)
  {
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
  }

  Vector4& Vector4::div(float f)
  {
    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return *this;
  }

  Vector4& Vector4::sum(Vector4& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }

  Vector4& Vector4::sub(Vector4& other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
  }

  Vector4& Vector4::mult(Vector4& other)
  {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;
    return *this;
  }

  Vector4& Vector4::div(Vector4& other)
  {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;
    return *this;
  }


  Vector4& Vector4::sum(Vector3& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vector4& Vector4::sub(Vector3& other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  Vector4& Vector4::mult(Vector3& other)
  {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
  }

  Vector4& Vector4::div(Vector3& other)
  {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
  }

  float Vector4::length()
  {
    return (float) sqrt(x*x + y*y + z*z + w*w);
  }

  Vector4& Vector4::normalized()
  {
    float len = length();
    x /= len;
    y /= len;
    z /= len;
    w /= len;
    return *this;
  }

  Vector4& Vector4::abs()
  {
    x = (float)fabs(x);
    y = (float)fabs(y);
    z = (float)fabs(z);
    w = (float)fabs(w);
    return *this;
  }

}
