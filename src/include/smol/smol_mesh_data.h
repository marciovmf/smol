#ifndef SMOL_PRIMITIVE_MESHES_H
#define SMOL_PRIMITIVE_MESHES_H

#include <smol/smol_engine.h>
#include <smol/smol_scene.h>

namespace smol
{
  struct Color;
  struct SMOL_ENGINE_API MeshData
  {
    const unsigned int* indices;
    const Vector3* positions;
    const Vector3* normals;
    const Color* colors;
    const Vector2* uv0;
    const Vector2* uv1;
    const int numPositions;
    const int numIndices;

    MeshData(Vector3* positions, int numPositions,
        unsigned int* indices = nullptr, int numIndices = 0,
        Color* colors = nullptr,
        Vector3* normals = nullptr, 
        Vector2* uv0 = nullptr, 
        Vector2* uv1 = nullptr);

    static const MeshData getPrimitiveCube();
    static const MeshData getPrimitiveArrow();
    static const MeshData getPrimitiveSphere();
    static const MeshData getPrimitiveCone();
    static const MeshData getPrimitiveCylinder();
    static const MeshData getPrimitiveQuad();
  };

}

#endif  // SMOL_PRIMITIVE_MESHES_H
