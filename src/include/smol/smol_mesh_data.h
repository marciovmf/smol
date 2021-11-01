#ifndef SMOL_PRIMITIVE_MESHES_H
#define SMOL_PRIMITIVE_MESHES_H

#include <smol/smol_engine.h>
#include <smol/smol_scene.h>

namespace smol
{
  struct SMOL_ENGINE_API MeshData
  {
    const unsigned int* indices;
    const Vector3* positions;
    const Vector3* normals;
    const Vector3* colors;
    const Vector2* uv0;
    const Vector2* uv1;
    const size_t numPositions;
    const size_t numIndices;

    MeshData(Vector3* positions, size_t numPositions,
        unsigned int* indices = nullptr, size_t numIndices = 0,
        Vector3* colors = nullptr,
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
