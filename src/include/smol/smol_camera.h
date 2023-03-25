#ifndef SMOL_CAMERA_H
#define SMOL_CAMERA_H

#include <smol/smol_engine.h>
#include <smol/smol_mat4.h>
namespace smol
{
  struct SMOL_ENGINE_API Camera
  {
    enum Type
    {
      PERSPECTIVE = 0,
      ORTHOGRAPHIC = 1
    };

    float aspect;
    float fov;
    float zNear;
    float zFar;
    float top;
    float right;
    float left;
    float bottom;
    Type type;
    uint32 layers;
    Mat4 viewMatrix;

    Camera& setPerspective(float fov, float aspect, float zNear, float zFar);
    Camera& setOrthographic(float left, float right, float top, float bottom, float zNear, float zFar);
    Camera& setLayerMask(uint32 layers);
    uint32 getLayerMask() const;
    const Mat4& getProjectionMatrix() const;
  };
}
#endif  // SMOL_CAMERA_H
