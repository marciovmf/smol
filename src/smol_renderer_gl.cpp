
#include "include/smol/gl/glcorearb.h"
#include "include/smol/smol_mat4.h"
#include <smol/smol_gl.h>             // must be included first
#include <smol/smol_random.h>
#include <smol/smol_renderer.h>
#include <smol/smol_material.h>
#include <smol/smol_image.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_triangle_mesh.h>
#include <smol/smol_scene.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_render_target.h>
#include <smol/smol_platform.h>
#include <smol/smol_config_manager.h>

#ifndef SMOL_RELEASE
#define checkGlError() _checkNoGlError(__FILE__, __LINE__)
static void _clearGlError()
{
  GLenum err;
  do
  {
    err = glGetError();
  }while (err != GL_NO_ERROR);
}

static int32 _checkNoGlError(const char* file, uint32 line)
{
  _clearGlError();
  const char* error = "UNKNOWN ERROR CODE";
  GLenum err = glGetError();
  int32 success = 1;
  uchar noerror = 1;
  while(err!=GL_NO_ERROR)
  {
    switch(err)
    {
      case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
      case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
      case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
      case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
    }
    success=0;
    debugLogError("GL ERROR %s at %s:%d",error, file, line);
    noerror=0;
    err=glGetError();
  }
  return success;
}

#else
#define checkGlError() 
#endif

namespace smol
{
  ShaderProgram Renderer::defaultShader = {};
  static GLuint globalUbo = 0; // this is the global uniform buffer accessible from any shader program
  const size_t SMOL_UBO_MAT4_PROJ             = 0;
  const size_t SMOL_UBO_MAT4_VIEW             = (1 * sizeof(Mat4));
  const size_t SMOL_UBO_MAT4_MODEL            = (2 * sizeof(Mat4));
  const size_t SMOL_UBO_FLOAT_DELTA_TIME      = (3 * sizeof(Mat4));
  const size_t SMOL_UBO_FLOAT_RANDOM_01       = (3 * sizeof(Mat4) + sizeof(float));
  const size_t SMOL_UBO_FLOAT_ELAPSED_SECONDS = (3 * sizeof(Mat4) + sizeof(float) * 2);
  const size_t SMOL_UBO_SIZE                  = 4 * sizeof(Mat4) + 3 * sizeof(float);
  const GLuint SMOL_GLOBALUBO_BINDING_POINT = 0;

  void Renderer::setMaterial(const Material* material)
  {
    GLuint shaderProgramId = 0; 
    ResourceManager& resourceManager = ResourceManager::get();
    ShaderProgram& shader = resourceManager.getShader(material->shader);

    switch(material->depthTest)
    {
      case Material::DISABLE:
        glDisable(GL_DEPTH_TEST);
        break;
      case Material::LESS:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);  
        break;
      case Material::LESS_EQUAL:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);  
        break;
      case Material::EQUAL:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_EQUAL);  
        break;
      case Material::GREATER:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);  
        break;
      case Material::GREATER_EQUAL:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);  
        break;
      case Material::DIFFERENT:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_NOTEQUAL);  
        break;
      case Material::ALWAYS:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);  
        break;
      case Material::NEVER:
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_NEVER);  
        break;
    }

    switch(material->cullFace)
    {
      case Material::BACK:
        glEnable(GL_CULL_FACE); 
        glCullFace(GL_BACK);
        break;

      case Material::FRONT:
        glEnable(GL_CULL_FACE); 
        glCullFace(GL_FRONT);
        break;

      case Material::FRONT_AND_BACK:
        glEnable(GL_CULL_FACE); 
        glCullFace(GL_FRONT_AND_BACK);
        break;

      case Material::NONE:
        glDisable(GL_CULL_FACE);
        break;
    }

    if(shader.valid)
    {
      // use WHITE as default color for vertex attribute when using a valid shader
      shaderProgramId = shader.glProgramId;
      const Color& white = Color::WHITE;
      glVertexAttrib4f(Mesh::COLOR, white.r, white.g, white.b, 1.0f);
    }
    else
    {
      // use MAGENTA as default color for vertex attribute when using the default shader
      //
      shaderProgramId = Renderer::getDefaultShaderProgram().glProgramId;
      const Color& magenta = Color::MAGENTA;
      glVertexAttrib4f(Mesh::COLOR, magenta.r, magenta.g, magenta.b, 1.0f);
    }


    // Bind the global shader uniform buffer
    GLuint ubIndex = glGetUniformBlockIndex(shaderProgramId, "smol");
    glUniformBlockBinding(shaderProgramId, ubIndex,  SMOL_GLOBALUBO_BINDING_POINT); // globalUB will always be bound to index 0;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUbo);

    glUseProgram(shaderProgramId);

    // Apply uniform values from the material
    for (int i = 0; i < material->parameterCount; i++)
    {
      const MaterialParameter& parameter = material->parameter[i];
      switch(parameter.type)
      {
        case ShaderParameter::SAMPLER_2D:
          if (parameter.uintValue < (uint32) material->diffuseTextureCount)
          {
            int textureIndex = parameter.uintValue;
            Handle<Texture> hTexture = material->textureDiffuse[textureIndex];
            Texture& texture = resourceManager.getTexture(hTexture);
            glActiveTexture(GL_TEXTURE0 + textureIndex);
            GLuint textureId = texture.glTextureObject;
            glBindTexture(GL_TEXTURE_2D, textureId);
          }
          break;
        case ShaderParameter::FLOAT:
          glUniform1f(parameter.glUniformLocation, parameter.floatValue);
          break;

        case ShaderParameter::VECTOR2:
          glUniform2f(parameter.glUniformLocation, parameter.vec2Value.x, parameter.vec2Value.y);
          break;

        case ShaderParameter::VECTOR3:
          glUniform3f(parameter.glUniformLocation, parameter.vec3Value.x, parameter.vec3Value.y, parameter.vec3Value.z);
          break;

        case ShaderParameter::VECTOR4:
          glUniform4f(parameter.glUniformLocation, parameter.vec4Value.x, parameter.vec4Value.y, parameter.vec4Value.z, parameter.vec4Value.w);
          break;

        case ShaderParameter::INT:
          glUniform1i(parameter.glUniformLocation, parameter.intValue);
          break;

        case ShaderParameter::UNSIGNED_INT:
          glUniform1ui(parameter.glUniformLocation, parameter.uintValue);
          break;

        default:
          continue;
          break;
      }
    }
  }

  void Renderer::setMaterial(Handle<Material> handle)
  {
    const Material* material = handle.operator->();
    setMaterial(material);
  }

  void Renderer::setViewport(uint32 x, uint32 y, uint32 w, uint32 h)
  {
    glViewport(x, y, w, h);
  }

  Rect Renderer::getViewport()
  {
    GLint value[4];
    glGetIntegerv(GL_VIEWPORT, value);
    return Rect(value[0], value[1], value[2], value[3]);

  }

  void Renderer::clearBuffers(uint32 flag)
  {
    glClear(flag);
  }

  void Renderer::setClearColor(float r, float g, float b, float a)
  {
    glClearColor(r, g, b, a);
  }

  void Renderer::setClearColor(const Color& color)
  {
    glClearColor(color.r, color.g, color.b, color.a);
  }

  void Renderer::beginScissor(uint32 x, uint32 y, uint32 w, uint32 h)
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w, h);
  }

  void Renderer::endScissor()
  {
    glDisable(GL_SCISSOR_TEST);
  }

  void Renderer::setRenderMode(RenderMode mode)
  {
    if (mode == WIREFRAME)
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
  }

  void Renderer::updateGlobalShaderParams(const Mat4& proj, const Mat4& view, const Mat4& model, float deltaTime)
  {
    glBindBuffer(GL_UNIFORM_BUFFER, globalUbo);

    // proj
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_PROJ,
        sizeof(Mat4), proj.e);
    // view
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_VIEW,
        sizeof(Mat4), view.e);
    // model
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_MODEL,
        sizeof(Mat4), model.e);
    // delta time
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_FLOAT_DELTA_TIME,
        sizeof(float), &deltaTime);
    // random01
    float random01 = (float) smol::random01();
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_FLOAT_RANDOM_01,
        sizeof(float), &random01);
    // elapsed time
    float elapsedSeconds = Platform::getSecondsSinceStartup();
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_FLOAT_ELAPSED_SECONDS,
        sizeof(float), &elapsedSeconds);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  bool Renderer::createTextureRenderTarget(RenderTarget* out, int32 width, int32 height)
  {
    out->type = RenderTarget::TEXTURE;
    glCreateFramebuffers(1, &out->glFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, out->glFbo);

    Image dummyImage;
    dummyImage.width = width;
    dummyImage.height = height;
    dummyImage.bitsPerPixel = 24;   //RGB
    dummyImage.data = nullptr;
    Renderer::createTexture(&out->colorTexture, dummyImage);

    // bind the texture as color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out->colorTexture.glTextureObject, 0);

    // create a buffer attachment for depth and stencil
    glGenRenderbuffers(1, &out->glRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, out->glRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, out->glRbo);

    bool success = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    if (!success)
    {
      glDeleteFramebuffers(1, &out->glFbo);
      glDeleteRenderbuffers(1, &out->glRbo);
      glDeleteTextures(1, &out->colorTexture.glTextureObject);
      debugLogError("Failed to create a Texture Render Target");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return success;
  }

  void Renderer::resizeTextureRenderTarget(const RenderTarget& target, int32 width, int32 height)
  {
    glBindTexture(GL_TEXTURE_2D, target.colorTexture.glTextureObject);
    bool useSRGB = ConfigManager::get().rendererConfig().enableGammaCorrection;
    glTexImage2D(GL_TEXTURE_2D, 0, useSRGB ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, target.glRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  void Renderer::useRenderTarget(const RenderTarget& target)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, target.glFbo);
  }

  void Renderer::useDefaultRenderTarget()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void Renderer::initialize(const GlobalRendererConfig& config)
  {
    glGenBuffers(1, &globalUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, globalUbo);
    glBufferData(GL_UNIFORM_BUFFER, SMOL_UBO_SIZE, (void*) SMOL_GLOBALUBO_BINDING_POINT, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if (config.enableGammaCorrection)
    {
      glEnable(GL_FRAMEBUFFER_SRGB); 
    }

    if (config.enableMSAA)
    {
      glEnable(GL_MULTISAMPLE);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

  void Renderer::terminate()
  {
  }

  //
  // Texture resources
  //

  bool Renderer::createTexture(Texture* outTexture, const Image& image, Texture::Wrap wrap, Texture::Filter filter, Texture::Mipmap mipmap)
  {
    outTexture->width = image.width;
    outTexture->height = image.height;

    GLenum textureFormat;
    GLenum textureType;

    if (image.bitsPerPixel == 32)
    {
      textureFormat = GL_RGBA;
      textureType = GL_UNSIGNED_BYTE;
    }
    else if (image.bitsPerPixel == 24)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_BYTE;
    }
    else if (image.bitsPerPixel == 16)
    {
      if (image.format16 == Image::RGB_1_5_5_5)
      {
        textureFormat = GL_RGBA;
        textureType = GL_UNSIGNED_SHORT_5_5_5_1;
      }
      else
      {
        textureFormat = GL_RGB;
        textureType = GL_UNSIGNED_SHORT_5_6_5;
      }
    }
    else
    {
      debugLogError("Unsuported Image format. Colors might be wrong.");
      textureFormat = GL_RGBA;
      textureType = GL_UNSIGNED_BYTE;
    }

    glGenTextures(1, &outTexture->glTextureObject);
    glBindTexture(GL_TEXTURE_2D, outTexture->glTextureObject);

    // if the engine is set to use SRGB ?
    bool useSRGB = ConfigManager::get().rendererConfig().enableGammaCorrection;

    glTexImage2D(GL_TEXTURE_2D, 0, useSRGB ? GL_SRGB_ALPHA : GL_RGBA, image.width, image.height, 0, textureFormat, textureType, image.data);

    GLuint mode;
    switch (wrap)
    {
      case Texture::Wrap::REPEAT_MIRRORED:
        mode = GL_MIRRORED_REPEAT;
        break;
      case Texture::Wrap::CLAMP_TO_EDGE:
        mode = GL_CLAMP_TO_EDGE;
        break;
      case Texture::Wrap::REPEAT:
      default:
        mode = GL_REPEAT;
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

    switch (filter)
    {
      case Texture::Filter::NEAREST:
        mode = GL_NEAREST;
        break;

      case Texture::Filter::LINEAR:
      default:
        mode = GL_LINEAR;
        break;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);

    if (mipmap == Texture::Mipmap::NO_MIPMAP)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
    }
    else
    {
      switch(mipmap)
      {
        case Texture::Mipmap::LINEAR_MIPMAP_NEAREST:
          mode = GL_LINEAR_MIPMAP_NEAREST;
          break;
        case Texture::Mipmap::NEAREST_MIPMAP_LINEAR:
          mode = GL_NEAREST_MIPMAP_LINEAR;
          break;
        case Texture::Mipmap::NEAREST_MIPMAP_NEAREST:
          mode = GL_NEAREST_MIPMAP_NEAREST;
          break;
        case Texture::Mipmap::LINEAR_MIPMAP_LINEAR:
        default:
          mode = GL_LINEAR_MIPMAP_LINEAR;
          break;
      }
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return outTexture->glTextureObject != 0;
  }

  void Renderer::destroyTexture(Texture* texture)
  {
    glDeleteTextures(1, &texture->glTextureObject);
  }

  //
  // Shader resources
  //

  bool Renderer::createShaderProgram(ShaderProgram* outShader, const char* vsSource, const char* fsSource, const char* gsSource)
  {
    outShader->valid = false;
    outShader->glProgramId = 0;

    GLint status;
    const int errorLogSize = 1024;
    GLsizei errorBufferLen = 0;
    char errorBuffer[errorLogSize];

    // vertex shader
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vsSource, 0);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);

    if (! status)
    {
      glGetShaderInfoLog(vShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("Compiling VERTEX SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      return false;
    }

    // fragment shader
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fsSource, 0);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);

    if (! status)
    {
      glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      return false;
    }

    // geometry shader
    GLuint gShader = 0;

    if (gsSource)
    {
      gShader = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(gShader, 1, &gsSource, 0);
      glCompileShader(gShader);
      glGetShaderiv(gShader, GL_COMPILE_STATUS, &status);

      if (! status)
      {
        glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
        smol::Log::error("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        glDeleteShader(gShader);
        return false;
      }
    }

    // shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    if (gShader) glAttachShader(program, fShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (! status)
    {
      glGetProgramInfoLog(program, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("linking SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      if (gShader) glDeleteShader(gShader);
      glDeleteProgram(program);
      return false;
    }

    // find uniforms
    outShader->parameterCount = 0;
    int uniformCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    if (uniformCount > SMOL_MAX_SHADER_PARAMETERS)
    {
      Log::error("shader declares too many (%d) parameters. Maximum supported by material files is %d",
          uniformCount, SMOL_MAX_SHADER_PARAMETERS);
    }

    for (int i = 0; i < uniformCount && i < SMOL_MAX_SHADER_PARAMETERS; i++)
    {
      GLsizei length;
      int size;
      GLenum type;

      ShaderParameter& parameter = outShader->parameter[outShader->parameterCount];

      glGetActiveUniform(program, (GLuint)i, SMOL_MAX_SHADER_PARAMETER_NAME_LEN, &length, &size, &type, (GLchar*) &parameter.name);
      switch(type)
      {
        case GL_SAMPLER_2D:
          parameter.type = ShaderParameter::SAMPLER_2D;
          break;
        case GL_FLOAT:
          parameter.type = ShaderParameter::FLOAT;
          break;
        case GL_FLOAT_VEC2:
          parameter.type = ShaderParameter::VECTOR2;
          break;
        case GL_FLOAT_VEC3:
          parameter.type = ShaderParameter::VECTOR3;
          break;
        case GL_FLOAT_VEC4:
          parameter.type = ShaderParameter::VECTOR4;
          break;
        case GL_INT:
          parameter.type = ShaderParameter::INT;
          break;
        case GL_UNSIGNED_INT:
          parameter.type = ShaderParameter::UNSIGNED_INT;
          break;
        default:
          //TODO(marcio): We just ignore unsupported types for now. Is there a better approach ?
          continue;
          break;
      }

      parameter.glUniformLocation = glGetUniformLocation(program, parameter.name);
      outShader->parameterCount++;
    }


    glDeleteShader(vShader);
    glDeleteShader(fShader);
    if (gShader) glDeleteShader(gShader);

    outShader->glProgramId = program;
    outShader->valid = true;
    return true;
  }

  ShaderProgram Renderer::getDefaultShaderProgram()
  {
    if (defaultShader.valid)
    {
      return defaultShader;
    }

    const char* defaultVShader =
      "#version 330 core\n\
      layout (std140) uniform smol\n\
      {\n\
        mat4 proj;\n\
          mat4 view;\n\
          mat4 model;\n\
          float deltaTime;\n\
      };\n\
    layout (location = 0) in vec3 vertPos;\n\
      layout (location = 1) in vec2 vertUVIn;\n\
      out vec2 uv;\n\
      void main() { gl_Position = proj * view * model * vec4(vertPos, 1.0); uv = vertUVIn; }";

    const char* defaultFShader =
      "#version 330 core\n\
      out vec4 fragColor;\n\
      uniform sampler2D mainTex;\n\
      in vec2 uv;\n\
      void main(){ fragColor = texture(mainTex, uv) * vec4(1.0f, 0.0, 1.0, 1.0); }";

    bool success = createShaderProgram(&defaultShader, defaultVShader, defaultFShader, nullptr);
    SMOL_ASSERT(success == true, "Failed to create default shader program.");
    return defaultShader;
  }

  void Renderer::destroyShaderProgram(ShaderProgram* program)
  {
    glDeleteProgram(program->glProgramId);
    program->glProgramId = -1;
    program->valid = false;
  }

  //
  // Mesh resources
  //

  bool Renderer::createMesh(Mesh* outMesh,
      bool dynamic, Primitive primitive,
      const Vector3* vertices, int numVertices,
      const unsigned int* indices, int numIndices,
      const Color* color,
      const Vector2* uv0,
      const Vector2* uv1,
      const Vector3* normals)
  {
    if (!outMesh)
      return false;

    Mesh* mesh = outMesh;
    mesh->dynamic = dynamic;

    if (primitive == Primitive::TRIANGLE)
    {
      mesh->glPrimitive = GL_TRIANGLES;
    }
    else if (primitive == Primitive::LINE)
    {
      mesh->glPrimitive = GL_LINES;
    }
    else if (primitive == Primitive::POINT)
    {
      mesh->glPrimitive = GL_POINTS;
    }
    else
    {
      debugLogWarning("Unknown primitive. Defaulting to TRIANGLE.");
      mesh->glPrimitive = GL_TRIANGLES;
    }

    // VAO
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    GLenum bufferHint = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    if (numVertices)
    {
      mesh->numVertices = numVertices;
      mesh->verticesArraySize = numVertices * sizeof(Vector3);

      glGenBuffers(1, &mesh->vboPosition);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      glBufferData(GL_ARRAY_BUFFER, mesh->verticesArraySize, vertices, bufferHint);
      glVertexAttribPointer(Mesh::POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::POSITION);
    }

    mesh->ibo = 0;
    if (numIndices)
    {
      mesh->numIndices = numIndices;
      mesh->indicesArraySize = numIndices * sizeof(unsigned int);

      glGenBuffers(1, &mesh->ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesArraySize, indices, bufferHint);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    mesh->vboColor = 0;
    if(color)
    {
      glGenBuffers(1, &mesh->vboColor);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboColor);
      glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Color), color, bufferHint);
      glVertexAttribPointer(Mesh::COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::COLOR);
    }
    mesh->vboUV0 = 0;
    if(uv0)
    {
      glGenBuffers(1, &mesh->vboUV0);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV0);
      glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Vector2), uv0, bufferHint);
      glVertexAttribPointer(Mesh::UV0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::UV0);
    }

    mesh->vboUV1 = 0;
    if(uv1)
    {
      glGenBuffers(1, &mesh->vboUV1);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV1);
      glBufferData(GL_ARRAY_BUFFER,  mesh->numVertices * sizeof(Vector2), uv1, bufferHint);
      glVertexAttribPointer(Mesh::UV1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::UV1);
    }

    glBindVertexArray(0);
    return true;
  }

  void Renderer::updateMesh(Mesh* mesh, TriangleMesh* triangleMesh)
  {
    if (!mesh)
      return;

    if (!mesh->dynamic)
    {
      Log::warning("Unable to update a static mesh");
      return;
    }

    bool resizeBuffers = false;

    if (triangleMesh->positions)
    {
      size_t verticesArraySize = triangleMesh->numPositions * sizeof(Vector3);

      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      if (verticesArraySize > mesh->verticesArraySize)
      {
        resizeBuffers = true;
        mesh->verticesArraySize = verticesArraySize;
        glBufferData(GL_ARRAY_BUFFER, verticesArraySize, triangleMesh->positions, GL_DYNAMIC_DRAW);
      }
      else
      {
        glBufferSubData(GL_ARRAY_BUFFER, 0, verticesArraySize, triangleMesh->positions);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      mesh->numVertices = (unsigned int) (verticesArraySize / sizeof(Vector3));
    }

    if (triangleMesh->indices)
    {
      if (!mesh->ibo)
      {
        Log::warning("Unable to update indices for mesh created without index buffer.");
      }
      else
      {
        size_t indicesArraySize = triangleMesh->numIndices * sizeof(unsigned int);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
        if (indicesArraySize > mesh->indicesArraySize)
        {
          mesh->indicesArraySize = indicesArraySize;
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize, triangleMesh->indices, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indicesArraySize, triangleMesh->indices);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        mesh->numIndices = (unsigned int) (indicesArraySize / sizeof(unsigned int));
      }
    }

    if(triangleMesh->colors)
    {
      if (!mesh->vboColor)
      {
        Log::warning("Unable to update color for mesh created without color buffer.");
      }
      else
      {
        size_t size =  mesh->numVertices * sizeof(Color);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboColor);
        if (resizeBuffers)
        {
          glBufferData(GL_ARRAY_BUFFER, size, triangleMesh->colors, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, triangleMesh->colors);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(triangleMesh->uv0)
    {
      if (!mesh->vboUV0)
      {
        Log::warning("Unable to update UV0 for mesh created without UV0 buffer.");
      }
      else
      {
        size_t size = mesh->numVertices * sizeof(Vector2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV0);
        if(resizeBuffers)
        {
          glBufferData(GL_ARRAY_BUFFER, size, triangleMesh->uv0, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, triangleMesh->uv0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(triangleMesh->uv1)
    {
      if (!mesh->vboUV1)
      {
        Log::warning("Unable to update UV1 for mesh created without UV1 buffer.");
      }
      else
      {
        size_t size = mesh->numVertices * sizeof(Vector2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV1);
        if(resizeBuffers)
        {
          glBufferData(GL_ARRAY_BUFFER, size, triangleMesh->uv1, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, triangleMesh->uv1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }
  }

  void Renderer::destroyMesh(Mesh* mesh)
  {
    GLuint buffers[Mesh::MAX_BUFFERS_PER_MESH];
    int numBuffers = 0;

    if (mesh->ibo) buffers[numBuffers++] = mesh->ibo;
    if (mesh->vboPosition) buffers[numBuffers++] = mesh->vboPosition;
    if (mesh->vboNormal) buffers[numBuffers++] = mesh->vboNormal;
    if (mesh->vboUV0) buffers[numBuffers++] = mesh->vboUV0;
    if (mesh->vboUV1) buffers[numBuffers++] = mesh->vboUV1;

    glDeleteBuffers(numBuffers, (const GLuint*) buffers);
    glDeleteVertexArrays(1, (const GLuint*) &mesh->vao);
  }

  void drawTriangleMesh(const TriangleMesh& triangleMesh, uint32 triangleListIndex)
  {
    SMOL_ASSERT(triangleListIndex < triangleMesh.numTriangleLists, "The triangle list index passed to drawTriangleMesh() is out of bounds");

    const Mesh* mesh = triangleMesh.mesh.operator->();
    glBindVertexArray(mesh->vao);

    if (mesh->ibo != 0)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
      glDrawElements(mesh->glPrimitive, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
      glDrawArrays(mesh->glPrimitive, 0, mesh->numVertices);
    }
    glBindVertexArray(0);
  }

  //
  // StreamBuffer
  //

  bool Renderer::createStreamBuffer(StreamBuffer* out, uint32 capacity, StreamBuffer::Format format, uint32 indicesPerElement)
  {
    out->format = format;
    out->capacity = capacity;
    out->used = 0;
    out->bound = false;
    out->indicesPerElement = indicesPerElement;

    // VAO
    glGenVertexArrays(1, &out->vao);
    glBindVertexArray(out->vao);

    // IBO
    glGenBuffers(1, &out->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, out->capacity * indicesPerElement * sizeof(uint32), (void*) nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VBO
    glGenBuffers(1, &out->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, out->vbo);

    switch (format)
    {
      case StreamBuffer::POS_COLOR_UV:
        {
          out->elementSize = 9 * sizeof(float);
          glVertexAttribPointer(Mesh::POSITION, 3, GL_FLOAT, GL_FALSE, (GLsizei) out->elementSize, (const void*) 0);
          glVertexAttribPointer(Mesh::COLOR,    4, GL_FLOAT, GL_FALSE, (GLsizei) out->elementSize, (const void*) (3 * sizeof(float)));
          glVertexAttribPointer(Mesh::UV0,      2, GL_FLOAT, GL_FALSE, (GLsizei) out->elementSize, (const void*) (7 * sizeof(float)));

          glEnableVertexAttribArray(Mesh::POSITION);
          glEnableVertexAttribArray(Mesh::COLOR);
          glEnableVertexAttribArray(Mesh::UV0);
        }
        break;

      case StreamBuffer::POS_COLOR_UV_UV:
        {
          out->elementSize = 11 * sizeof(float);
          glVertexAttribPointer(Mesh::POSITION, 3, GL_FLOAT, GL_FALSE, (GLsizei)out->elementSize, (const void*) 0);
          glVertexAttribPointer(Mesh::COLOR,    4, GL_FLOAT, GL_FALSE, (GLsizei)out->elementSize, (const void*) (3 * sizeof(float)));
          glVertexAttribPointer(Mesh::UV0,      2, GL_FLOAT, GL_FALSE, (GLsizei)out->elementSize, (const void*) (7 * sizeof(float)));
          glVertexAttribPointer(Mesh::UV1,      2, GL_FLOAT, GL_FALSE, (GLsizei)out->elementSize, (const void*) (9 * sizeof(float)));

          glEnableVertexAttribArray(Mesh::POSITION);
          glEnableVertexAttribArray(Mesh::COLOR);
          glEnableVertexAttribArray(Mesh::UV0);
          glEnableVertexAttribArray(Mesh::UV1);
        }
        break;

      default:
        debugLogError("Unsuported Buffer format %d", (int)format);
        break;
    }

    glBufferData(GL_ARRAY_BUFFER, out->elementSize * out->capacity, (void*) nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    return true;
  }

  bool Renderer::resizeStreamBuffer(StreamBuffer& streamBuffer, uint32 capacity)
  {
    if (streamBuffer.format == StreamBuffer::UNINITIALIZED)
      return false;

    SMOL_ASSERT(streamBuffer.bound == true, "Can't resize and unbound StreamBuffer.");
    SMOL_ASSERT(streamBuffer.capacity < capacity, "Can't Shrinking a streamBuffer.");

    // resize VBO
    size_t size = streamBuffer.elementSize * capacity;
    streamBuffer.capacity = capacity;
    glBufferData(GL_ARRAY_BUFFER, size, (void*) nullptr, GL_DYNAMIC_DRAW);

    // resize IBO
    size = capacity * streamBuffer.indicesPerElement * sizeof(uint32);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, (void*) nullptr, GL_DYNAMIC_DRAW);
    return true;
  }

  void Renderer::bindStreamBuffer(StreamBuffer& streamBuffer)
  {
    if (streamBuffer.format == StreamBuffer::UNINITIALIZED)
      return;

    if (streamBuffer.bound)
      return;

    glBindVertexArray(streamBuffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, streamBuffer.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, streamBuffer.ibo);
    streamBuffer.bound = true;


    streamBuffer.vertexBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (!streamBuffer.vertexBuffer)
      debugLogError("Unable to map GPU memory for StreamBuffer");

    streamBuffer.indexBuffer = (uint32*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (!streamBuffer.indexBuffer)
      debugLogError("Unable to map GPU memory for StreamBuffer");
  }

  void Renderer::unbindStreamBuffer(StreamBuffer& streamBuffer)
  {
    if (streamBuffer.format == StreamBuffer::UNINITIALIZED)
      return;

    if (!streamBuffer.bound)
      return;


    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    streamBuffer.bound = false;
  }

  bool Renderer::destroyStreamBuffer(StreamBuffer& streamBuffer)
  {
    if (streamBuffer.format == StreamBuffer::UNINITIALIZED)
      return false;

    glDeleteBuffers(1, &streamBuffer.vbo);
    glDeleteVertexArrays(1, &streamBuffer.vao);

    streamBuffer.capacity = 0;
    streamBuffer.used     = 0;
    streamBuffer.vbo      = -1;
    streamBuffer.vao      = -1;
    return true;
  }

  void Renderer::begin(StreamBuffer& streamBuffer)
  {
    SMOL_ASSERT(streamBuffer.bound == false, "Cant begin() on a StreamBuffer that is already bound. Did you call begin() twice ?");

    bindStreamBuffer(streamBuffer);
    streamBuffer.flushCount = 0;
  }

  void Renderer::pushSprite(StreamBuffer& streamBuffer, const Vector3& position, const Vector2& size, const Rectf& uv, const Color& color)
  {
    pushSprite(streamBuffer, position, size, uv, color, color, color, color);
  }

  void Renderer::pushSprite(StreamBuffer& streamBuffer, const Vector3& position, const Vector2& size, const Rectf& uv, const Color& tlColor, const Color& trColor, const Color& blColor, const Color& brColor)
  {
    const int indicesPerSprite = 6;
    const int verticesPerSrprite = 4;

    SMOL_ASSERT(streamBuffer.bound == true, "Cant pushSprite() on a StreamBuffer that is not bound. Did forget to call begin() ?");
    SMOL_ASSERT(streamBuffer.indicesPerElement == 6,"The current StreamBuffer uses %d indices per element. Pushing a sprite assumes %d indices per element.", streamBuffer.indicesPerElement, indicesPerSprite);

    if (streamBuffer.used + 4 >= streamBuffer.capacity)
    {
      flush(streamBuffer);
    }

    const float y = -position.y;

    VertexPCU* pVertex = (VertexPCU*) (streamBuffer.used * streamBuffer.elementSize + (char*) streamBuffer.vertexBuffer);
    // Top left 
    pVertex->position = {position.x,  y, position.z};
    pVertex->color     = tlColor;
    pVertex->uv        = {uv.x, uv.y};
    pVertex++;
    // bottom right
    pVertex->position = {position.x + size.x,  y - size.y, position.z};
    pVertex->color     = brColor;
    pVertex->uv        = {uv.x + uv.w, uv.y - uv.h};
    pVertex++;
    // top right
    pVertex->position = {position.x + size.x,  y, position.z};
    pVertex->color     = trColor;
    pVertex->uv        = {uv.x + uv.w, uv.y};
    pVertex++;
    // bottom left
    pVertex->position = {position.x, y - size.y, position.z};
    pVertex->color     = blColor;
    pVertex->uv        = {uv.x, uv.y - uv.h};
    pVertex++;

    int numSprites = streamBuffer.used / verticesPerSrprite;
    int offset = numSprites * 4;
    uint32* pIndex = (uint32*) (numSprites * 6 * sizeof(uint32) + (char*) streamBuffer.indexBuffer);

    pIndex[0] = offset + 0;
    pIndex[1] = offset + 1;
    pIndex[2] = offset + 2;
    pIndex[3] = offset + 0;
    pIndex[4] = offset + 3;
    pIndex[5] = offset + 1;
    streamBuffer.used += 4;
  }

  void Renderer::pushLines(StreamBuffer& streamBuffer, const Vector2* points, int numPoints, const Color& color, float thickness)
  {
    const int indicesPerSprite = 6;
    const int verticesPerSrprite = 4;

    SMOL_ASSERT(streamBuffer.bound == true, "Cant pushSprite() on a StreamBuffer that is not bound. Did forget to call begin() ?");
    SMOL_ASSERT(streamBuffer.indicesPerElement == 6,"The current StreamBuffer uses %d indices per element. Pushing a sprite assumes %d indices per element.", streamBuffer.indicesPerElement, indicesPerSprite);

    if (streamBuffer.used + 4 >= streamBuffer.capacity)
    {
      flush(streamBuffer);
    }
    if (numPoints < 2)
    {
      debugLogWarning("Not enough points provided to pushLine()");
      return;
    }

    Vector2 p0 = points[0];
    p0.y = -p0.y;
    const float ht = thickness/2.0f;

    VertexPCU vertex[verticesPerSrprite];
    for(int i = 1; i < numPoints; i++)
    {
      Vector2 p1 = points[i];
      p1.y = -p1.y;

      VertexPCU* pVertex = (VertexPCU*) (streamBuffer.used * streamBuffer.elementSize + (char*) streamBuffer.vertexBuffer);

      if (abs(p0.x - p1.x) > abs(p0.y - p1.y))
      {
        // Top left 
        pVertex->position = {p0.x,  p0.y - ht, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
        // bottom right
        pVertex->position = {p1.x,  p1.y + ht, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
        // top right
        pVertex->position = {p1.x, p1.y - ht, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
        // bottom left
        pVertex->position = {p0.x, p0.y + ht, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
      }
      else 
      {
        // Top left 
        pVertex->position = {p0.x + ht,  p0.y, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
        // bottom right
        pVertex->position = {p1.x - ht,  p1.y, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
        // top right
        pVertex->position = {p1.x + ht, p1.y, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
        // bottom left
        pVertex->position = {p0.x - ht, p0.y, 0.0f};
        pVertex->color     = color;
        pVertex->uv        = Vector2(0.0f);
        pVertex++;
      }

      int numSprites = streamBuffer.used / verticesPerSrprite;
      uint32* pIndex = (uint32*) (numSprites * 6 * sizeof(uint32) + (char*) streamBuffer.indexBuffer);
      int offset = numSprites * 4;
      pIndex[0] = offset + 0;
      pIndex[1] = offset + 1;
      pIndex[2] = offset + 2;
      pIndex[3] = offset + 0;
      pIndex[4] = offset + 3;
      pIndex[5] = offset + 1;

      //TODO(marcio): Map GPU memory and write to it directly to void calling on gl driver so much
      //glBufferSubData(GL_ARRAY_BUFFER, streamBuffer.used * streamBuffer.elementSize, sizeof(vertex), (void*) vertex);
      //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, numSprites * 6 * sizeof(uint32), sizeof(index), (void*) index);
      streamBuffer.used += 4;
      p0 = p1;
    }

  }

  void Renderer::flush(StreamBuffer& streamBuffer)
  {
    int numSprites = streamBuffer.used / 4;
    int count = numSprites * 6;


    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    streamBuffer.vertexBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    streamBuffer.vertexBuffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    streamBuffer.flushCount++;
    streamBuffer.used = 0;
  }

  void Renderer::end(StreamBuffer& streamBuffer)
  {
    SMOL_ASSERT(streamBuffer.bound == true, "Cant end() on a StreamBuffer that is already bound. Did you call end() twice ?");
    flush(streamBuffer);

    // if flushed more than once before ending, it means we should enlarge the buffer so we don't flush so often
    uint32 flushCount = streamBuffer.flushCount - 1;
    if (flushCount > 0)
    {
      uint32 newCapacity = streamBuffer.capacity * flushCount * streamBuffer.capacity;
      resizeStreamBuffer(streamBuffer, newCapacity);
    }
    unbindStreamBuffer(streamBuffer);
  }
}
