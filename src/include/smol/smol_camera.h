#ifndef SMOL_CAMERA_H
#define SMOL_CAMERA_H

#include <smol/smol_engine.h>
#include <smol/smol_color.h>
#include <smol/smol_mat4.h>
#include <smol/smol_rect.h>
namespace smol
{
  enum Layer
  {
    LAYER_0 = 1 << 0,
    LAYER_1 = 1 << 1,
    LAYER_2 = 1 << 2,
    LAYER_3 = 1 << 3,
    LAYER_4 = 1 << 4,
    LAYER_5 = 1 << 5,
    LAYER_6 = 1 << 6,
    LAYER_7 = 1 << 7,
    LAYER_8 = 1 << 8,
    LAYER_9 = 1 << 9,
    LAYER_10 = 1 << 10,
    LAYER_11 = 1 << 11,
    LAYER_12 = 1 << 12,
    LAYER_13 = 1 << 13,
    LAYER_14 = 1 << 14,
    LAYER_15 = 1 << 15,
    LAYER_16 = 1 << 16,
    LAYER_17 = 1 << 17,
    LAYER_18 = 1 << 18,
    LAYER_19 = 1 << 19,
    LAYER_20 = 1 << 20,
    LAYER_21 = 1 << 21,
    LAYER_22 = 1 << 22,
    LAYER_23 = 1 << 23,
    LAYER_24 = 1 << 24,
    LAYER_25 = 1 << 25,
    LAYER_26 = 1 << 26,
    LAYER_27 = 1 << 27,
    LAYER_28 = 1 << 28,
    LAYER_29 = 1 << 29,
    LAYER_30 = 1 << 30,
    LAYER_31 = 1 << 31
  };

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
    unsigned int clearOperation;
    Color clearColor;
    unsigned int priority;

    public:

    Camera(float left, float right, float top, float bottom, float zNear, float zFar);
    Camera(float fov, float aspect, float zNear, float zFar);

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

    unsigned int getClearOperation() const;
    Camera& setClearOperation(unsigned int operation);

    const Color& getClearColor() const;
    Camera& setClearColor(const Color& color);

    unsigned int getPriority() const;
    Camera& setPriority(unsigned int priority);

    Rectf getOrthographicRect() const;

  };
}
#endif  // SMOL_CAMERA_H
