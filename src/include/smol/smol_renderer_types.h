#ifndef SMOL_RENDERER_TYPES_H
#define SMOL_RENDERER_TYPES_H

#include <smol/smol_engine.h>
#include <smol/smol_resource_list.h>
#include <smol/smol_color.h>

#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h> //TODO(marcio): Make this API independent. Remove all GL specifics from this header
#undef SMOL_GL_DEFINE_EXTERN

//smol_color.h is not used by this header, but included by convenience and consistency since Color is a type meant to be used by the renderer

namespace smol
{
  enum RenderQueue : char
  {
    QUEUE_OPAQUE = 10,
    QUEUE_TRANSPARENT = 20,
    QUEUE_GUI = 30,
    QUEUE_TERRAIN = 40
  };

  enum Primitive : char
  {
    TRIANGLE,
    TRIANGLE_STRIP,
    LINE,
    POINT
  };

  struct SMOL_ENGINE_API Rect
  {
    int x, y, w, h;
  };

  struct SMOL_ENGINE_API Rectf
  {
    float x, y, w, h;
  };

  struct SMOL_ENGINE_API Texture
  {
    enum Wrap
    {
      REPEAT            = 0,
      REPEAT_MIRRORED   = 1,
      CLAMP_TO_EDGE     = 2
    };

    enum Filter
    {
      LINEAR                  = 0,
      NEAREST                 = 1
    };

    enum Mipmap
    {
      LINEAR_MIPMAP_LINEAR    = 0,
      LINEAR_MIPMAP_NEAREST   = 1,
      NEAREST_MIPMAP_LINEAR   = 2,
      NEAREST_MIPMAP_NEAREST  = 3,
      NO_MIPMAP               = 4
    };

    int width;
    int height;
    GLuint textureObject;
  };

  struct SMOL_ENGINE_API ShaderProgram
  {
    bool valid;
    GLuint programId;
    //TODO(marcio): store uniform locations here
  };

#define SMOL_MATERIAL_MAX_TEXTURES 6
#define SMOL_MAX_BUFFERS_PER_MESH 6
  struct SMOL_ENGINE_API Material
  {
    Handle<ShaderProgram> shader;
    Handle<Texture> textureDiffuse[SMOL_MATERIAL_MAX_TEXTURES];
    int diffuseTextureCount;
    int renderQueue;
    //TODO(marcio): Add more state relevant options here
  };

  struct SMOL_ENGINE_API Mesh
  {
    enum Attribute
    {
      //Don't change these values. They're referenced from the shader
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

  struct SMOL_ENGINE_API Renderable
  {
    Handle<Material> material;
    Handle<Mesh> mesh;
  };

  struct SMOL_ENGINE_API SpriteBatcher
  {
    static const size_t positionsSize;
    static const size_t indicesSize;
    static const size_t colorsSize;
    static const size_t uvsSize;
    static const size_t totalSpriteSize;

    Handle<Renderable> renderable;
    Arena arena;
    int spriteCount;
    int spriteCapacity;
    bool dirty;
  };
}

#endif  // SMOL_RENDERER_TYPES_H
