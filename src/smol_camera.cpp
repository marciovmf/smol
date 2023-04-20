#include <smol/smol_camera.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_renderer.h>

namespace smol
{
  Camera::Camera(Type type, float fovOrSize, float zNear, float zFar)
    :clearOperation(ClearOperation::COLOR | ClearOperation::DEPTH),
    type(type),
    clearColor(Color::GRAY),
    layers(Layer::LAYER_0),
    rect(0.0f, 0.0f, 1.0f, 1.0f),
    priority(0),
    orthographicSize(0.0f)
  {
    if (type == Type::PERSPECTIVE)
      setPerspective(fovOrSize, zNear, zFar);
    else
      setOrthographic(fovOrSize, zNear, zFar);
  }

  Camera& Camera::setPerspective(float fov, float zNear, float zFar)
  {
    Rect viewport = SystemsRoot::get()->renderer.getViewport();
    this->type = Camera::PERSPECTIVE;
    this->fov = fov;
    this->aspect = (float)(viewport.w / viewport.h);
    this->zNear = zNear;
    this->zFar = zFar;
    this->viewMatrix = Mat4::perspective(fov, aspect, zNear, zFar);
    return *this;
  }

  Camera& Camera::setOrthographic(float size, float zNear, float zFar)
  {
    Rect viewport = SystemsRoot::get()->renderer.getViewport();
    float hSize = (size * viewport.w) / viewport.h;

    this->type = Camera::ORTHOGRAPHIC;
    this->zNear = zNear;
    this->zFar = zFar;
    this->left = -hSize;
    this->right = hSize;
    this->top = size;
    this->bottom = -size;
    this->orthographicSize = size;

    this->viewMatrix = Mat4::ortho(left, right, top, bottom, zNear, zFar);
    return *this;
  }

  void Camera::update()
  {
    if (type == Camera::PERSPECTIVE)
    {
      setPerspective(fov,  zNear, zFar);
    }
    else
    {
      setOrthographic(top, zNear, zFar); // size is assigned to top on orthographic cameras
    }
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
    return *this;
  }

  float Camera::getFOV() const { return fov; }

  float Camera::getAspect() const { return aspect; }

  float Camera::getNearClipDistance() const { return zNear; }

  float Camera::getFarClipDistance() const { return zFar; }

  Camera::Type Camera::getCameraType() const { return type; }

  unsigned int Camera::getClearOperation() const { return clearOperation; }

  Camera& Camera::setClearOperation(unsigned int operation)
  {
    this->clearOperation = operation;
    return *this;
  }

  const Color& Camera::getClearColor() const
  {
    return clearColor;
  }

  Camera& Camera::setClearColor(const Color& color)
  {
    clearColor = color;
    return *this;
  }

  unsigned int Camera::getPriority() const
  {
    return priority;
  }

  Camera& Camera::setPriority(unsigned int priority)
  {
    this->priority = priority;
    return *this;
  }

  float Camera::getOrthographicSize() const { return orthographicSize; }
}
