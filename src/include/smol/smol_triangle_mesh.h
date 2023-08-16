#ifndef SMOL_MESH_DATA_H
#define SMOL_MESH_DATA_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>

namespace smol
{
  struct Color;
  struct Vector3;
  struct Vector2;
  struct Material;
  struct Mesh;

  struct TriangleListInfo
  {
    enum
    {
      MAX_GROUP_NAME_LENGTH = 64,
    };

    char name[MAX_GROUP_NAME_LENGTH];
    int32 materialIndex;    // Material index that should be used to render this
    int32 numIndices;       // Number of vertices in the list (must be a multiple of 3)
    int32 firstIndex;       // index of the beginning of the list ?
  };

  struct SMOL_ENGINE_API TriangleMesh
  {
    const Vector3* positions;           // All imported positions
    const unsigned int* indices;        // All imported indices
    const Vector3* normals;             // All normals
    const Color* colors;                // All colors
    const Vector2* uv0;                 // All UVs
    const Vector2* uv1;                 // All UVs (second pair)
    uint32 numPositions;
    uint32 numIndices;
    uint32 numTriangleLists;
    uint32 numMaterials;
    uint32 numNormals;
    Handle<Material> *materials;        // All materials
    TriangleListInfo* triangleLists;    // All triangle lists
    Handle<Mesh> mesh;                  // API specific mesh information

    TriangleMesh();
    TriangleMesh(Vector3* positions, int numPositions, unsigned int* indices = nullptr, int numIndices = 0);
    TriangleMesh(Vector3* positions, int numPositions,
        unsigned int* indices = nullptr, int numIndices = 0,
        Color* colors = nullptr,
        Vector3* normals = nullptr, 
        Vector2* uv0 = nullptr, 
        Vector2* uv1 = nullptr);

    TriangleMesh& setPosition(Vector3* positions);
    TriangleMesh& setColors(Color* colors);
    TriangleMesh& setUV0(Vector2* uvs);
    TriangleMesh& setUV1(Vector2* uvs);
    TriangleMesh& setIndices(unsigned int* indices);
    TriangleMesh& setTriangleMeshList(TriangleListInfo* triangleLists, uint32 numTriangleLists);
    TriangleMesh& setMaterialList(Handle<Material>* materials, uint32 numMaterials);
    TriangleMesh(const TriangleMesh& other);

    static const TriangleMesh getPrimitiveCube();
    static const TriangleMesh getPrimitiveArrow();
    static const TriangleMesh getPrimitiveSphere();
    static const TriangleMesh getPrimitiveCone();
    static const TriangleMesh getPrimitiveCylinder();
    static const TriangleMesh getPrimitiveQuad();
  };

  struct TriangleMeshFile
  {
    char magic[4] = {'m', 'e', 's','h'};
    uint32 version = 0x0001;
    uint32 offsetIndices;               // offset to a unsigned int array of indices
    uint32 offsetPositions;             // offset to a Vector3 array of positions
    uint32 offsetNormals;               // offset to a Vector3 array of normals
    uint32 offsetUV;                    // offset to a Vector2 array of uvs
    uint32 offsetTriangleLists;         // offset to a TriangleListInfo array
    //uint32 offsetMaterials;             // offset to a MaterialInfo array
    uint32 numIndices;
    uint32 numTriangleLists;
    uint32 numMaterials;
    uint32 numVertices;
  };
}

#endif  //SMOL_MESH_DATA_H 
