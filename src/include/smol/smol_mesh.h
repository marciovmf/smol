#ifndef SMOL_MESH_H
#define SMOL_MESH_H

#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h> //TODO(marcio): Make this API independent. Remove all GL specifics from this header
#undef SMOL_GL_DEFINE_EXTERN

namespace smol
{

  struct SMOL_ENGINE_API Mesh final
  {
    enum
    {
      MAX_BUFFERS_PER_MESH = 6
    };

    enum Attribute
    {
      //Don't change these values. They're referenced from the shaders
      POSITION = 0,
      UV0 = 1,
      UV1 = 2,
      NORMAL = 3,
      COLOR = 4,
      INDEX // this one does not point to an attribute buffer
    };
    bool dynamic;

    GLuint glPrimitive;
    GLuint vao;
    GLuint ibo;
    GLuint vboPosition;
    GLuint vboNormal;
    GLuint vboUV0;
    GLuint vboUV1;
    GLuint vboColor;
    size_t verticesArraySize;
    size_t indicesArraySize;
    unsigned int numIndices;
    unsigned int numVertices;
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::Mesh>;
  template class SMOL_ENGINE_API smol::Handle<smol::Mesh>;
}
#endif  // SMOL_MESH_H
