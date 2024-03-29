#ifndef SMOL_MAT4
#define SMOL_MAT4

#include <smol/smol_engine.h>
#include <smol/smol_vector3.h>

namespace smol
{
  struct SMOL_ENGINE_API Mat4
  {
    float e[4][4];    // Access the matrix members as a 2 dimentional array

    static Mat4 initIdentity();
    static Mat4 initScale(float amount);
    static Mat4 initScale(float x, float y, float z);
    static Mat4 initTranslation(float x, float y, float z);
    static Mat4 initRotation(float x, float y, float z);
    static Mat4 perspective(float fov, float aspect, float zNear, float zFar);
    static Mat4 ortho(float left, float right, float top, float bottom, float zNear, float zFar);
    static Mat4 transpose(const Mat4& m);
    static Mat4 invert(const Mat4& m);
    static Mat4 mul(const Mat4& a, const Mat4& b);
    static Vector3 mul(const Mat4& a, const Vector3& b);

    Mat4& mul(const Mat4& other);
    Mat4 transposed() const;
    Mat4 inverse() const;
  };
}

#endif // SMOL_MAT4
