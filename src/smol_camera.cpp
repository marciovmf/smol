#include <smol/smol_camera.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_renderer.h>

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
    this->flags |= (unsigned int) Flag::PROJECTION_CHANGED;
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
    this->flags |= (unsigned int) Flag::PROJECTION_CHANGED;
    return *this;
  }

  Camera& Camera::setLayerMask(uint32 layers)
  {
    this->layers = layers;
    return *this;
  }

  inline uint32 Camera::getLayerMask() const { return layers; }

  inline const Mat4& Camera::getProjectionMatrix() const { return viewMatrix; }

  const Rectf& Camera::getViewportRect() const { return rect; }

  Camera& Camera::setViewportRect(const Rectf& rect)
  {
    this->rect = rect;

    if (this->rect.x > 1.0f) this->rect.x = 1.0f;
    if (this->rect.y > 1.0f) this->rect.y = 1.0f;
    if (this->rect.w > 1.0f) this->rect.w = 1.0f;
    if (this->rect.h > 1.0f) this->rect.h = 1.0f;

    if (this->rect.x < 0.0f) this->rect.x = 0.0f;
    if (this->rect.y < 0.0f) this->rect.y = 0.0f;
    if (this->rect.w < 0.0f) this->rect.w = 0.0f;
    if (this->rect.h < 0.0f) this->rect.h = 0.0f;

    flags |= (unsigned int) Flag::VIEWPORT_CHANGED;
    return *this;
  }

  float Camera::getFOV() const { return fov; }

  float Camera::getAspect() const { return aspect; }

  float Camera::getNearClipDistance() const { return zNear; }

  float Camera::getFarClipDistance() const { return zFar; }

  Camera::Type Camera::getCameraType() const { return type; }

  unsigned int Camera::getFlags() const { return flags; }

  void Camera::clearFlags() { flags = 0; };

}
