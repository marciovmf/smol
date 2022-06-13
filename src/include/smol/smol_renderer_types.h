#ifndef SMOL_RENDERER_TYPES_H
#define SMOL_RENDERER_TYPES_H

#include <smol/smol_engine.h>
#include <smol/smol_resource_list.h>
#include <smol/smol_color.h>
#include <smol/smol_rect.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector4.h>

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


  struct SMOL_ENGINE_API Texture
  {
    enum Wrap
    {
      REPEAT            = 0,
      REPEAT_MIRRORED   = 1,
      CLAMP_TO_EDGE     = 2,
      MAX_WRAP_OPTIONS
    };

    enum Filter
    {
      LINEAR                  = 0,
      NEAREST                 = 1,
      MAX_FILTER_OPTIONS
    };

    enum Mipmap
    {
      LINEAR_MIPMAP_LINEAR    = 0,
      LINEAR_MIPMAP_NEAREST   = 1,
      NEAREST_MIPMAP_LINEAR   = 2,
      NEAREST_MIPMAP_NEAREST  = 3,
      NO_MIPMAP               = 4,
      MAX_MIPMAP_OPTIONS
    };

    int width;
    int height;
    GLuint textureObject;
  };

#define SMOL_MAX_SHADER_PARAMETER_NAME_LEN 64
  struct ShaderParameter
  {
    public:
    enum Type
    {
      SAMPLER_2D,
      VECTOR2,
      VECTOR3,
      VECTOR4,
      FLOAT,
      INT,
      UNSIGNED_INT,
      INVALID
    };

    Type type;
    char name[SMOL_MAX_SHADER_PARAMETER_NAME_LEN];
    GLuint location;
  };

  struct SMOL_ENGINE_API MaterialParameter : public ShaderParameter
  {
    union
    {
      float floatValue;
      Vector2 vec2Value;
      Vector3 vec3Value;
      Vector4 vec4Value;
      int32 intValue;
      uint32 uintValue;
    };
  };

#define SMOL_MAX_SHADER_PARAMETERS 16
  struct SMOL_ENGINE_API ShaderProgram
  {
    bool valid;
    GLuint programId;
    ShaderParameter parameter[SMOL_MAX_SHADER_PARAMETERS];
    int parameterCount;
  };

#define SMOL_MATERIAL_MAX_TEXTURES 6
#define SMOL_MAX_BUFFERS_PER_MESH 6
  struct SMOL_ENGINE_API Material
  {
    Handle<ShaderProgram> shader;
    Handle<Texture> textureDiffuse[SMOL_MATERIAL_MAX_TEXTURES];
    int diffuseTextureCount;
    int renderQueue;
    MaterialParameter parameter[SMOL_MAX_SHADER_PARAMETERS];
    int parameterCount;


    Material* setSampler2D(const char* name, unsigned int value);
    Material* setUint(const char* name, unsigned int value);
    Material* setInt(const char* name, int value);
    Material* setFloat(const char* name, float value);
    Material* setVec2(const char* name, const Vector2& value);
    Material* setVec3(const char* name, const Vector3& value);
    Material* setVec4(const char* name, const Vector4& value);

    private:
    MaterialParameter* getParameter(const char* name, ShaderParameter::Type type);
    //TODO(marcio): Add more state relevant options here
  };

  struct SMOL_ENGINE_API Mesh
  {
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
