#ifndef SMOL_RENDERER_TYPES_H
#define SMOL_RENDERER_TYPES_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_color.h>
#include <smol/smol_rect.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector4.h>
#include <smol/smol_mat4.h>

#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h> //TODO(marcio): Make this API independent. Remove all GL specifics from this header
#undef SMOL_GL_DEFINE_EXTERN

//smol_color.h is not used by this header, but included by convenience and consistency since Color is a type meant to be used by the renderer

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

    void setPerspective(float fov, float aspect, float zNear, float zFar);
    void setOrthographic(float left, float right, float top, float bottom, float zNear, float zFar);
    void setLayers(uint32 layers);
    uint32 getLayerMask() const;
    const Mat4& getProjectionMatrix() const;
  };

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
    enum DepthTest
    {
      DISABLE         = 0,
      LESS            = 1,
      LESS_EQUAL      = 2,
      EQUAL           = 3,
      GREATER         = 4,
      GREATER_EQUAL   = 5,
      DIFFERENT       = 6,
      NEVER           = 7,
      ALWAYS          = 8
    };

    enum CullFace
    {
      BACK            = 0,
      FRONT           = 1,
      FRONT_AND_BACK  = 2,
      NONE            = 3
    };

    Handle<ShaderProgram> shader;
    Handle<Texture> textureDiffuse[SMOL_MATERIAL_MAX_TEXTURES];
    int diffuseTextureCount;
    int renderQueue;
    MaterialParameter parameter[SMOL_MAX_SHADER_PARAMETERS];
    int parameterCount;
    DepthTest depthTest;
    CullFace cullFace;

    Material* setSampler2D(const char* name, unsigned int value);
    Material* setUint(const char* name, unsigned int value);
    Material* setInt(const char* name, int value);
    Material* setFloat(const char* name, float value);
    Material* setVec2(const char* name, const Vector2& value);
    Material* setVec3(const char* name, const Vector3& value);
    Material* setVec4(const char* name, const Vector4& value);

    private:
    MaterialParameter* getParameter(const char* name, ShaderParameter::Type type);
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
