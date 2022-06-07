#include <smol/smol.h>
#include <smol/smol_log.h>
#include <smol/smol_mat4.h>
#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

namespace smol
{
  Mat4 Mat4::initIdentity()
  {
    Mat4 m = {};
    m.e[0][0] = m.e[1][1] = m.e[2][2] = m.e[3][3] = 1.0f;
    return m;
  }

  Mat4 Mat4::initScale(float amount)
  {
    return Mat4::initScale(amount, amount, amount);
  }

  Mat4 Mat4::initScale(float x, float y, float z)
  {
    Mat4 m = Mat4::initIdentity();
    m.e[0][0] = x;
    m.e[1][1] = y;
    m.e[2][2] = z;
    return m;
  }

  Mat4 Mat4::initTranslation(float x, float y, float z)
  {
    Mat4 m = Mat4::initIdentity();
    m.e[3][0] = x;
    m.e[3][1] = y;
    m.e[3][2] = z;
    return m;
  }

  Mat4 Mat4::initRotation(float x, float y, float z)
  {
    const double toRad = (M_PI/180.0);

    const double angleX = x * toRad;
    const double angleY = y * toRad;
    const double angleZ = z * toRad;

    const double cx = cos(angleX);
    const double cy = cos(angleY);
    const double cz = cos(angleZ);

    const double sx = sin(angleX);
    const double sy = sin(angleY);
    const double sz = sin(angleZ);

    // Right-hand rotation matrices
    Mat4 xRot = Mat4::initIdentity();
    xRot.e[1][1] = (float)(cx);
    xRot.e[1][2] = (float)(sx);
    xRot.e[2][1] = (float)(-sx);
    xRot.e[2][2] = (float)(cx);

    Mat4 yRot = Mat4::initIdentity();
    yRot.e[0][0] = (float)(cy);
    yRot.e[0][2] = (float)(-sy);
    yRot.e[2][0] = (float)(sy);
    yRot.e[2][2] = (float)(cy);
    
    Mat4 zRot = Mat4::initIdentity();
    zRot.e[0][0] = (float)(cz);
    zRot.e[0][1] = (float)(sz);
    zRot.e[1][0] = (float)(-sz);
    zRot.e[1][1] = (float)(cz);

    return Mat4::mul(zRot, Mat4::mul(yRot, xRot));
  }

  Mat4 Mat4::mul(const Mat4& a, const Mat4& b)
  {
    Mat4 m; 

    for (int line = 0; line < 4; line++)
    {
      for (int column = 0; column < 4; column++)
      {
        m.e[column][line] = 
          a.e[0][line] * b.e [column][0] +
          a.e[1][line] * b.e [column][1] +
          a.e[2][line] * b.e [column][2] +
          a.e[3][line] * b.e [column][3];
      }
    }

    return m;
  }

  Mat4 Mat4::perspective(float fov, float aspect, float zNear, float zFar)
  {
    SMOL_ASSERT(zNear >= 0.0f, "zNear must be positive.",0);
    SMOL_ASSERT(zFar > 0.0f, "zFar must be positive.",0);
    SMOL_ASSERT(zFar > zNear, "zFar must be larger than zNear.",0);

    fov = (float) (fov * (M_PI/180.0));

    const float tanHalfFov = (float) tan(fov/2);
    Mat4 m = Mat4::initIdentity();
    m.e[0][0] = 1 / (aspect * tanHalfFov);
    m.e[1][1] = 1 / tanHalfFov;
    m.e[2][2] = - (zFar + zNear) / (zFar - zNear);
    m.e[2][3] = -1;
    m.e[3][2] =  - (2 * zFar * zNear) / (zFar - zNear);
    m.e[3][3] = 0.0f;
    return m;
  }

  Mat4 Mat4::ortho(float left, float right, float top, float bottom, float zNear, float zFar)
  {
    Mat4 m = Mat4::initIdentity();
    m.e[0][0] = 2 / (right - left);
    m.e[1][1] = 2 / (top - bottom);
    m.e[2][2] = - 2 / (zFar - zNear);
    m.e[3][0] = - (right + left) / (right - left);
    m.e[3][1] = - (top + bottom) / (top - bottom);
    m.e[3][2] = - (zFar + zNear) / (zFar - zNear);
    return m;
  }

  Mat4 Mat4::transpose(Mat4& m)
  {
    Mat4 t;
    for(int line = 0; line < 4; line++)
    {
      for(int column = 0; column < 4; column++)
      {
        t.e[column][line] = m.e[line][column];
      }
    }
    return t;
  }

  Vector3 Mat4::mul(const Mat4& a, const Vector3& b)
  {
    Vector3 t(
        a.e[0][0] * b.x + a.e[0][1] * b.y + a.e[0][2] * b.z + a.e[0][3],
        a.e[1][0] * b.x + a.e[1][1] * b.y + a.e[1][2] * b.z + a.e[1][3],
        a.e[2][0] * b.x + a.e[2][1] * b.y + a.e[2][2] * b.z + a.e[2][3]);
    return t;
  }

  Mat4& Mat4::mul(const Mat4& other)
  {
    Mat4 result = Mat4::mul(*this, other);
    *this = result;
    return *this;
  }

  Mat4& Mat4::transposed()
  {
    Mat4 t = Mat4::transpose(*this);
    *this = t;
    return *this;
  }
}
