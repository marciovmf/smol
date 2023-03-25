#include <smol/smol_camera.h>

namespace smol
{
  Camera& Camera::setPerspective(float fov, float aspect, float zNear, float zFar)
  {
    this->type = Camera::PERSPECTIVE;
    this->fov = fov;
    this->aspect = aspect;
    this->zNear = zNear;
    this->zFar = zFar;
    this->viewMatrix = Mat4::perspective(fov, aspect, zNear, zFar);
    return *this;
  }

  Camera& Camera::setOrthographic(float left, float right, float top, float bottom, float zNear, float zFar)
  {
    this->type = Camera::ORTHOGRAPHIC;
    this->left = left;
    this->right = right;
    this->top = top;
    this->bottom = bottom;
    this->viewMatrix = Mat4::ortho(left, right, top, bottom, zNear, zFar);
    return *this;
  }

  Camera& Camera::setLayerMask(uint32 layers)
  {
    this->layers = layers;
    return *this;
  }

  inline uint32 Camera::getLayerMask() const
  {
    return layers;
  }

  inline const Mat4& Camera::getProjectionMatrix() const
  {
    return viewMatrix;
  }

}
