
#include <smol/smol_gl.h>             // must be included first
#include <smol/smol_random.h>
#include <smol/smol_renderer.h>
#include <smol/smol_material.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_scene.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_platform.h>

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
  //
  // internal utility functions
  //

  static GLuint setMaterial(const Scene* scene, const Material* material, const SceneNode* cameraNode)
  {
    GLuint shaderProgramId = 0; 
    ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
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
      shaderProgramId = resourceManager.getShader(scene->defaultShader).glProgramId;
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

    return shaderProgramId;
  }

  static void updateGlobalShaderParams(SceneNode& cameraNode, float deltaTime)
  {
    glBindBuffer(GL_UNIFORM_BUFFER, globalUbo);
    // proj
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_PROJ,
        sizeof(Mat4), cameraNode.camera.getProjectionMatrix().e);
    // view
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_VIEW,
        sizeof(Mat4), cameraNode.transform.getMatrix().inverse().e);
    // model
    glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_MODEL,
        sizeof(Mat4), Mat4::initIdentity().e);
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

  //Radix sort 64bit values by the lower 32bit values.
  //param elements - pointer to 64bit integers to be sorted.
  //param elementCount - number of elements on elements array.
  //param dest - destination buffer where to put the sorted list. This buffer
  //must be large enough for storing elementCount elements.
  static void radixSort(uint64* elements, uint32 elementCount,  uint64* dest)
  {
    for(int shiftIndex = 0; shiftIndex < 32; shiftIndex+=8)
    {
      const uint32 bucketCount = 255;
      uint32 buckets[bucketCount] = {};

      // count key parts
      for(uint32 i = 0; i < elementCount; i++)
      {
        /// note we ignore the UPPER 32bit of the key
        uint32 element = (uint32) elements[i];
        int32 keySlice = (element >> shiftIndex) & 0xFF; // get lower part
        buckets[keySlice]++;
      }

      // calculate sorted positions
      uint32 startIndex = 0;
      for(uint32 i = 0; i < bucketCount; i++)
      {
        uint32 keyCount = buckets[i];
        buckets[i] = startIndex;
        startIndex += keyCount;
      }

      // move elements to their correct position
      for(uint32 i = 0; i < elementCount; i++)
      {
        uint64 element = elements[i];
        int32 keySlice = (element >> shiftIndex) & 0xFF; 
        uint32 destLocation = buckets[keySlice]++;
        // move the WHOLE 64bit key
        dest[destLocation] = element;
      }

      // swap buffers
      uint64* temp = elements;
      elements = dest;
      dest = temp;
    }
  }

  static inline uint64 encodeRenderKey(SceneNode::Type nodeType, uint16 materialIndex, uint8 queue, uint32 nodeIndex)
  {
    // Render key format
    // 64--------------------32---------------16-----------8---------------0
    // sceneNode index       | material index  | node type | render queue
    uint64 key = ((uint64) nodeIndex) << 32 | ((uint16) materialIndex) << 16 |  nodeType << 8 | (uint8) queue;
    return key;
  }

  static inline uint32 getNodeIndexFromRenderKey(uint64 key)
  {
    return (uint32) (key >> 32);
  }

  static inline uint32 getMaterialIndexFromRenderKey(uint64 key)
  {
    return ((uint32) key) >> 16;
  }

  static inline uint32 getNodeTypeFromRenderKey(uint64 key)
  {
    return (uint32) key >> 8;
  }

  //NOTE(marcio): We're probably not gonna need it
  //static inline uint32 getQueueFromRenderKey(uint64 key)
  //{
  //  return (uint32)((uint8)key);
  //}


  // This function assumes a material is already bound
  static void drawRenderable(Scene* scene, const Renderable* renderable, GLuint shaderProgramId)
  {
    Mesh* mesh = SystemsRoot::get()->resourceManager.getMesh(renderable->mesh);

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

  static int drawSpriteNodes(Scene* scene, SpriteBatcher* batcher, uint64* renderKeyList, uint32 cameraLayers)
  {
    const SceneNode* allNodes = scene->nodes.getArray();

    batcher->begin();
    for (int i = 0; i < batcher->spriteNodeCount; i++)
    {
      uint64 key = ((uint64*)renderKeyList)[i];
      SceneNode* sceneNode = (SceneNode*) &allNodes[getNodeIndexFromRenderKey(key)];

      // ignore sprites the current camera can't see
      if(!(cameraLayers & sceneNode->getLayer()))
        continue;
      batcher->pushSpriteNode(sceneNode);
    }
    batcher->end();
    return batcher->spriteNodeCount - 1;
  }

  //
  // Misc
  //

  Renderer::~Renderer()
  {
    debugLogInfo("Destroying Renderer");
  }

  Renderer::Renderer():
    scene(nullptr)
  {
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

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

  void Renderer::setScene(Scene& scene)
  {
    if (this->scene)
    {
      //TODO: Unbind and Unload all resources related to the current scene if any
      debugLogError("Replacing a loaded Scene is NOT IMPLEMENTED yet.");
    }

    this->scene = &scene;
  }

  Scene& Renderer::getLoadedScene()
  {
    return *scene;
  }

  Rect Renderer::getViewport() const
  {
    return viewport;
  }

  float Renderer::getViewportAspect() const
  {
    if (viewport.h <= 0 || viewport.w <= 0)
      return 0;
    return viewport.w / (float) viewport.h;
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
    bool useSRGB = SystemsRoot::get()->rendererConfig.enableGammaCorrection;

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

  void Renderer::updateMesh(Mesh* mesh, MeshData* meshData)
  {
    if (!mesh)
      return;

    if (!mesh->dynamic)
    {
      Log::warning("Unable to update a static mesh");
      return;
    }

    bool resizeBuffers = false;

    if (meshData->positions)
    {
      size_t verticesArraySize = meshData->numPositions * sizeof(Vector3);

      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      if (verticesArraySize > mesh->verticesArraySize)
      {
        resizeBuffers = true;
        mesh->verticesArraySize = verticesArraySize;
        glBufferData(GL_ARRAY_BUFFER, verticesArraySize, meshData->positions, GL_DYNAMIC_DRAW);
      }
      else
      {
        glBufferSubData(GL_ARRAY_BUFFER, 0, verticesArraySize, meshData->positions);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      mesh->numVertices = (unsigned int) (verticesArraySize / sizeof(Vector3));
    }

    if (meshData->indices)
    {
      if (!mesh->ibo)
      {
        Log::warning("Unable to update indices for mesh created without index buffer.");
      }
      else
      {
        size_t indicesArraySize = meshData->numIndices * sizeof(unsigned int);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
        if (indicesArraySize > mesh->indicesArraySize)
        {
          mesh->indicesArraySize = indicesArraySize;
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize, meshData->indices, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indicesArraySize, meshData->indices);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        mesh->numIndices = (unsigned int) (indicesArraySize / sizeof(unsigned int));
      }
    }

    if(meshData->colors)
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
          glBufferData(GL_ARRAY_BUFFER, size, meshData->colors, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, meshData->colors);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(meshData->uv0)
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
          glBufferData(GL_ARRAY_BUFFER, size, meshData->uv0, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, meshData->uv0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(meshData->uv1)
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
          glBufferData(GL_ARRAY_BUFFER, size, meshData->uv1, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, meshData->uv1);
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
  }

  void Renderer::unbindStreamBuffer(StreamBuffer& streamBuffer)
  {
    if (streamBuffer.format == StreamBuffer::UNINITIALIZED)
      return;

    if (!streamBuffer.bound)
      return;

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

    VertexPCU vertex[verticesPerSrprite];
    uint32 index[indicesPerSprite];

    // Top left 
    vertex[0].position = {position.x,  position.y, position.z};                         // top left
    vertex[0].color     = tlColor;
    vertex[0].uv        = {uv.x, uv.y};
    // bottom right
    vertex[1].position = {position.x + size.x,  position.y - size.y, position.z};       // bottom right
    vertex[1].color     = brColor;
    vertex[1].uv        = {uv.x + uv.w, uv.y - uv.h};
    // top right
    vertex[2].position = {position.x + size.x,  position.y, position.z};                // top right
    vertex[2].color     = trColor;
    vertex[2].uv        = {uv.x + uv.w, uv.y};
    // bottom left
    vertex[3].position = {position.x, position.y - size.y, position.z};                // bottom left
    vertex[3].color     = blColor;
    vertex[3].uv        = {uv.x, uv.y - uv.h};

    int numSprites = streamBuffer.used / verticesPerSrprite;
    int offset = numSprites * 4;
    index[0] = offset + 0;
    index[1] = offset + 1;
    index[2] = offset + 2;
    index[3] = offset + 0;
    index[4] = offset + 3;
    index[5] = offset + 1;

    //TODO(marcio): Map GPU memory and write to it directly to void calling on gl driver so much
    glBufferSubData(GL_ARRAY_BUFFER, streamBuffer.used * streamBuffer.elementSize, sizeof(vertex), (void*) vertex);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, numSprites * 6 * sizeof(uint32), sizeof(index), (void*) index);
    streamBuffer.used += 4;
  }

  void Renderer::flush(StreamBuffer& streamBuffer)
  {
    int numSprites = streamBuffer.used / 4;
    int count = numSprites * 6;

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
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

  //
  // Render
  //

  void Renderer::resize(int width, int height)
  {
    this->viewport.w = width;
    this->viewport.h = height;
    //OpenGL NDC coords are  LEFT-HANDED.
    //This is a RIGHT-HAND projection matrix.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    resized = true;
  }

  void Renderer::render(float deltaTime)
  {
    ResourceManager& resourceManager = SystemsRoot::get()->resourceManager;
    Scene& scene = *this->scene;
    const GLuint defaultShaderProgramId = resourceManager.getDefaultShader().glProgramId;
    const Material& defaultMaterial = resourceManager.getDefaultMaterial();

    const SceneNode* allNodes = scene.nodes.getArray();
    int numNodes = scene.nodes.count();

    scene.renderKeys.reset();
    scene.renderKeysSorted.reset();

    // ----------------------------------------------------------------------
    // Update sceneNodes and generate render keys
    int numCameras = 0;

    for(int i = 0; i < numNodes; i++)
    {
      SceneNode* node = (SceneNode*) &allNodes[i];
      Renderable* renderable = nullptr;
      uint64 key = 0;

      if (!node->isActiveInHierarchy())
        continue;

      switch(node->getType())
      {
        case SceneNode::CAMERA:
          {
            node->transform.update(scene);
            key = encodeRenderKey(node->getType(), 0, node->camera.getPriority(), i);
            numCameras++;
          }
          break;

        case SceneNode::MESH:
          {
            node->transform.update(scene);
            renderable = scene.renderables.lookup(node->mesh.renderable);
            Handle<Material> material = renderable->material;
            key = encodeRenderKey(node->getType(), (uint16)(material.slotIndex), material->renderQueue, i);
          }
          break;

        case SceneNode::TEXT:
          {
            node->transform.update(scene);
            if (node->transform.isDirty(scene) || node->isDirty())
            {
              SpriteBatcher* batcher = scene.batchers.lookup(node->text.batcher);
              batcher->dirty = true;
            }
            Handle<Material> material = node->text.batcher->material;
            key = encodeRenderKey(node->getType(), (uint16)(material.slotIndex), material->renderQueue, i);
          }
          break;
        case SceneNode::SPRITE:
          {
            node->transform.update(scene);
            if (node->transform.isDirty(scene) || node->isDirty())
            {
              SpriteBatcher* batcher = scene.batchers.lookup(node->sprite.batcher);
              batcher->dirty = true;
            }
            Handle<Material> material = node->sprite.batcher->material;
            key = encodeRenderKey(node->getType(), (uint16)(material.slotIndex), material->renderQueue, i);
          }
          break;

        default:
          continue;
          break;
      }

      // save the key if the node is active
      node->transform.update(scene);
      uint64* keyPtr = (uint64*) scene.renderKeys.pushSize(sizeof(uint64));
      *keyPtr = key;
    }

    // ----------------------------------------------------------------------
    // Sort keys
    const int32 numKeysToSort = (int32) (scene.renderKeys.getUsed() / sizeof(uint64));
    radixSort((uint64*) scene.renderKeys.getData(), numKeysToSort, (uint64*) scene.renderKeysSorted.pushSize(scene.renderKeys.getUsed()));

    // Cameras will be the first nodes on the sorted list. We use that to iterate all cameras
    uint64* allCameraKeys = (uint64*) scene.renderKeysSorted.getData();
    uint64* allRenderKeys = allCameraKeys + numCameras;
    const int32 numKeys = numKeysToSort - numCameras; // don't count with camera nodes;

    for(int cameraIndex = 0; cameraIndex < numCameras; cameraIndex++)
    {
      uint64 cameraKey = allCameraKeys[cameraIndex];
      SceneNode* cameraNode = (SceneNode*) &allNodes[getNodeIndexFromRenderKey(cameraKey)];
      SMOL_ASSERT(cameraNode->typeIs(SceneNode::Type::CAMERA), "SceneNode is CAMERA", cameraNode->getType());

      // If we resized the display, make sure to update camera projection
      if (resized)
        cameraNode->camera.update();

      // ----------------------------------------------------------------------
      // VIEWPORT

      const Rectf& cameraRect = cameraNode->camera.getViewportRect();
      Rect screenRect;
      screenRect.x = (size_t)(viewport.w * cameraRect.x);
      screenRect.y = (size_t)(viewport.h * cameraRect.y);
      screenRect.w = (size_t)(viewport.w * cameraRect.w);
      screenRect.h = (size_t)(viewport.h * cameraRect.h);

      glViewport((GLsizei) screenRect.x, (GLsizei) screenRect.y, (GLsizei) screenRect.w, (GLsizei) screenRect.h);

      // ----------------------------------------------------------------------
      // CLEAR
      const Color& clearColor = cameraNode->camera.getClearColor();
      glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);

      unsigned int clearOperation = cameraNode->camera.getClearOperation();
      if (clearOperation != Camera::ClearOperation::DONT_CLEAR)
      {
        GLuint glClearFlags = 0;

        if (clearOperation & Camera::ClearOperation::COLOR)
          glClearFlags |= GL_COLOR_BUFFER_BIT;

        if (clearOperation & Camera::ClearOperation::DEPTH)
          glClearFlags |= GL_DEPTH_BUFFER_BIT;

        //TODO(marcio): This hack will allow us to clear only the camera's viewport. Remove it when we have per camera Framebuffers working.
        glEnable(GL_SCISSOR_TEST);
        glScissor((GLsizei) screenRect.x, (GLsizei) screenRect.y, (GLsizei) screenRect.w, (GLsizei) screenRect.h);
        glClear(glClearFlags);
        glDisable(GL_SCISSOR_TEST);
      }

      // ----------------------------------------------------------------------
      // set uniform buffer matrices based on current camera

      updateGlobalShaderParams(*cameraNode, deltaTime);

      // ----------------------------------------------------------------------
      // Draw render keys
      int currentMaterialIndex = -1;
      GLuint shaderProgramId = 0; 
      uint32 cameraLayers = cameraNode->camera.getLayerMask();

      for(int i = 0; i < numKeys; i++)
      {
        uint64 key = allRenderKeys[i];
        SceneNode* node = (SceneNode*) &allNodes[getNodeIndexFromRenderKey(key)];
        SceneNode::Type nodeType = (SceneNode::Type) getNodeTypeFromRenderKey(key);
        int materialIndex = getMaterialIndexFromRenderKey(key);
        SMOL_ASSERT(node->typeIs(nodeType), "Node Type does not match the render key node type");

        node->setDirty(false);
        node->transform.setDirty(false); // reset transform dirty flag

        // Change material *if* necessary
        if (currentMaterialIndex != materialIndex)
        {
          currentMaterialIndex = materialIndex;
          Material& material = (resourceManager.getMaterials(nullptr))[materialIndex];
          shaderProgramId = setMaterial(&scene, &material, cameraNode);
        }

        if (node->typeIs(SceneNode::MESH)) 
        {
          if(!(cameraLayers & node->getLayer()))
            continue;

          glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_MODEL, sizeof(Mat4),
              node->transform.getMatrix().e);

          Renderable* renderable = scene.renderables.lookup(node->mesh.renderable);
          drawRenderable(&scene, renderable, shaderProgramId);
        }
        else if (node->typeIs(SceneNode::TEXT))
        {
          if(!(cameraLayers & node->getLayer()))
            continue;

          SpriteBatcher* batcher = scene.batchers.lookup(node->text.batcher);
          glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_MODEL, sizeof(Mat4),
              node->transform.getMatrix().e);

          batcher->begin();
          batcher->pushTextNode(node);
          batcher->end();
        }
        else if (node->typeIs(SceneNode::SPRITE))
        {
          SpriteBatcher* batcher = scene.batchers.lookup(node->sprite.batcher);
          glBufferSubData(GL_UNIFORM_BUFFER, SMOL_UBO_MAT4_MODEL, sizeof(Mat4),
              node->transform.getMatrix().e);

          drawSpriteNodes(&scene, batcher, allRenderKeys + i, cameraLayers);
          i+= (batcher->spriteNodeCount - 1);
        }
        else
        {
          //TODO(marcio): Implement scpecific render logic for each type of node
          continue; 
        }
      }
    }

    resized = false;

    // unbind the last shader and textures (material)
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glUseProgram(defaultShaderProgramId);
    for (int i = 0; i < defaultMaterial.diffuseTextureCount; i++)
    {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}
