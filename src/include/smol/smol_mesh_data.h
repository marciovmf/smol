#ifndef SMOL_MESH_DATA_H
#define SMOL_MESH_DATA_H

#include <smol/smol_engine.h>

namespace smol
{
  struct Color;
  struct Vector3;
  struct Vector2;

  struct SMOL_ENGINE_API MeshData
  {
    const Vector3* positions;
    const unsigned int* indices;
    const Vector3* normals;
    const Color* colors;
    const Vector2* uv0;
    const Vector2* uv1;
    int numPositions;
    int numIndices;

    MeshData();
    MeshData(Vector3* positions, int numPositions, unsigned int* indices = nullptr, int numIndices = 0);
    MeshData(Vector3* positions, int numPositions,
        unsigned int* indices = nullptr, int numIndices = 0,
        Color* colors = nullptr,
        Vector3* normals = nullptr, 
        Vector2* uv0 = nullptr, 
        Vector2* uv1 = nullptr);

    MeshData& setPosition(Vector3* positions);
    MeshData& setColors(Color* colors);
    MeshData& setUV0(Vector2* uvs);
    MeshData& setUV1(Vector2* uvs);
    MeshData& setIndices(unsigned int* indices);

    MeshData(const MeshData& other);

    static const MeshData getPrimitiveCube();
    static const MeshData getPrimitiveArrow();
    static const MeshData getPrimitiveSphere();
    static const MeshData getPrimitiveCone();
    static const MeshData getPrimitiveCylinder();
    static const MeshData getPrimitiveQuad();
  };

}

#endif  //SMOL_MESH_DATA_H
