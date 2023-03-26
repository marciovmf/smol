#ifndef SMOL_CAMERA_H
#define SMOL_CAMERA_H

#include <smol/smol_engine.h>
#include <smol/smol_color.h>
#include <smol/smol_mat4.h>
#include <smol/smol_rect.h>
namespace smol
{
  struct SMOL_ENGINE_API Camera
  {
    enum Flag: unsigned int
    {
      NONE                = 0,
      VIEWPORT_CHANGED    = 1 << 1,
      PROJECTION_CHANGED  = 1 << 2,
      CLEAR_COLOR_CHANGED = 1 << 3
    };

    enum Type
    {
      PERSPECTIVE       = 0,
      ORTHOGRAPHIC      = 1
    };

    enum ClearOperation
    {
      DONT_CLEAR        = 0,
      COLOR             = 1 << 0,
      DEPTH             = 1 << 1
    };

    private:
    float aspect;
    float fov;
    float zNear;
    float zFar;
    float top;
    float right;
    float left;
    float bottom;
    Rectf rect;
    Type type;
    uint32 layers;
    Mat4 viewMatrix;
    unsigned int flags;
    unsigned int clearOperation;
    Color clearColor;

    public:
    Camera::Camera();
    Camera& setPerspective(float fov, float aspect, float zNear, float zFar);
    Camera& setOrthographic(float left, float right, float top, float bottom, float zNear, float zFar);
    Camera& setLayerMask(uint32 layers);
    uint32 getLayerMask() const;
    const Mat4& getProjectionMatrix() const;

    const Rectf& getViewportRect() const;
    Camera& setViewportRect(const Rectf& rect);

    float getFOV() const;
    float getAspect() const;
    float getNearClipDistance() const;
    float getFarClipDistance() const;
    Camera::Type getCameraType() const;
    unsigned int getStatusFlags() const;
    void resetStatusFlags();

    unsigned int getClearOperation() const;
    Camera& setClearOperation(unsigned int operation);

    const Color& getClearColor() const;
    Camera& setClearColor(const Color& color);

  };
}
#endif  // SMOL_CAMERA_H
