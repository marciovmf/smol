#include <smol/smol_platform.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_mesh_data.h>
#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h>
#undef SMOL_GL_DEFINE_EXTERN


namespace smol
{

  const size_t SpriteBatcher::positionsSize = 4 * sizeof(Vector3);
  const size_t SpriteBatcher::colorsSize = 4 * sizeof(Color);
  const size_t SpriteBatcher::uvsSize = 4 * sizeof(Vector2);
  const size_t SpriteBatcher::indicesSize = 6 * sizeof(unsigned int);
  const size_t SpriteBatcher::totalSpriteSize = positionsSize + colorsSize + uvsSize + indicesSize;

  const Handle<SceneNode> Scene::ROOT = INVALID_HANDLE(SceneNode);

  Transform::Transform():
    position({.0f, .0f, .0f}), rotation(.0f, .0f, .0f),
    scale({1.0f, 1.0f, 1.0f}), angle(0.0f), dirty(false)
    { }

  const Mat4& Transform::getMatrix() const
  {
    return  model; 
  }

  void Transform::setPosition(float x, float y, float z) 
  { 
    position.x = x;
    position.y = y;
    position.z = z;
    dirty = true;
  }

  void Transform::setScale(float x, float y, float z)
  { 
    scale.x = x;
    scale.y = y;
    scale.z = z;
    dirty = true;
  }

  void Transform::setRotation(float x, float y, float z, float angle) 
  {
    rotation.x = x;
    rotation.y = y;
    rotation.z = z;
    this->angle = angle;
    dirty = true;
  };

  const Vector3& Transform::getPosition() const { return position; }

  const Vector3& Transform::getScale() const { return scale; }

  bool Transform::isDirty() const { return dirty; }

  bool Transform::update()
  {
    if(!dirty)
      return false;

    // scale
    Mat4 scaleMatrix = Mat4::initScale(scale.x, scale.y, scale.z);
    Mat4 transformed = Mat4::mul(scaleMatrix, Mat4::initIdentity());

    // rotation
    Mat4 rotationMatrix = Mat4::initRotation(rotation.x, rotation.y, rotation.z, angle);
    transformed = Mat4::mul(rotationMatrix, transformed);

    // translation
    Mat4 translationMatrix = Mat4::initTranslation(position.x, position.y, position.z);
    transformed = Mat4::mul(translationMatrix, transformed);

    model = transformed;
    dirty = false;
    return true;
  }

  Scene::Scene():
    shaders(32), textures(64), materials(32), meshes(32), renderables(32), nodes(128), 
    batchers(8), renderKeys((size_t)255), renderKeysSorted((size_t)255),
    clearColor(160/255.0f, 165/255.0f, 170/255.0f),
    clearOperation((ClearOperation)(COLOR_BUFFER | DEPTH_BUFFER))
  {
    viewMatrix = Mat4::initIdentity();
    Image* img = AssetManager::createCheckersImage(800, 600, 32);
    defaultTexture = createTexture(*img);
    AssetManager::unloadImage(img);

    const char* defaultVShader =
      "#version 330 core\n\
      layout (location = 0) in vec3 vertPos;\n\
      layout (location = 1) in vec2 vertUVIn;\n\
      uniform mat4 proj;\n\
      out vec2 uv;\n\
      void main() { gl_Position = proj * vec4(vertPos, 1.0); uv = vertUVIn; }";

    const char* defaultFShader =
      "#version 330 core\n\
      out vec4 fragColor;\n\
      uniform sampler2D mainTex;\n\
      in vec2 uv;\n\
      void main(){ fragColor = texture(mainTex, uv) * vec4(1.0f, 0.0, 1.0, 1.0);}";

    defaultShader = createShaderFromSource(defaultVShader, defaultFShader, nullptr);
    defaultMaterial = createMaterial(defaultShader, &defaultTexture, 1);
  }

  void Scene::setNodeActive(Handle<SceneNode> handle, bool status)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return;
    node->active = status;
    node->dirty = true;
  }

  bool Scene::isNodeActive(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return false;

    return node->active;
  }


  Transform* Scene::getTransform(Handle<SceneNode> handle)
  {
    SceneNode* sceneNode = nodes.lookup(handle);

    if (!sceneNode)
      return nullptr;

    return &sceneNode->transform;
  }

  // ##################################################################
  //  Texture resource handling 
  // ##################################################################

  Handle<Texture> Scene::createTexture(const char* path)
  {
    Image* image = AssetManager::loadImageBitmap(path);
    Handle<Texture> texture = createTexture(*image);
    AssetManager::unloadImage(image);
    return texture;
  }

  Handle<Texture> Scene::createTexture(const Image& image)
  {
    Texture texture;
    texture.width = image.width;
    texture.height = image.height;

    GLenum textureFormat = GL_RGBA;
    GLenum textureType = GL_UNSIGNED_BYTE;

    if (image.bitsPerPixel == 24)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_BYTE;
    }
    else if (image.bitsPerPixel == 16)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_SHORT_5_6_5;
    }


    glGenTextures(1, &texture.textureObject);
    glBindTexture(GL_TEXTURE_2D, texture.textureObject);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, textureFormat, textureType, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return textures.add(texture);
  }

  void Scene::destroyTexture(Texture* texture)
  {
    glDeleteTextures(1, &texture->textureObject);
  }

  void Scene::destroyTexture(Handle<Texture> handle)
  {
    Texture* texture = textures.lookup(handle);
    if (!texture)
    {
      warnInvalidHandle("Texture");
    }
    else
    {
      destroyTexture(texture);
      textures.remove(handle);
    }
  }

  // ##################################################################
  //  Material resource handling 
  // ##################################################################

  Handle<Material> Scene::createMaterial(Handle<ShaderProgram> shader,
      Handle<Texture>* diffuseTextures, int diffuseTextureCount)
  {
    SMOL_ASSERT(diffuseTextureCount <= SMOL_MATERIAL_MAX_TEXTURES, "Exceeded Maximum diffuse textures per material");

    Handle<Material> handle = materials.reserve();
    Material* material = materials.lookup(handle);

    if (diffuseTextureCount)
    {
      size_t copySize = diffuseTextureCount * sizeof(Handle<Texture>);
      material->shader = shader;
      material->diffuseTextureCount = diffuseTextureCount;
      memcpy(material->textureDiffuse, diffuseTextures, copySize);
    }
    return handle;
  }

  void Scene::destroyMaterial(Handle<Material> handle)
  {
    Material* material = materials.lookup(handle);
    if(!material)
    {
      warnInvalidHandle("Material");
    }
    else
    {
      materials.remove(handle);
    }
  }

  // ##################################################################
  //  Mesh resource handling 
  // ##################################################################

  Handle<Mesh> Scene::createMesh(bool dynamic, const MeshData* meshData)
  {
    const size_t numPositions = meshData->numPositions;
    const size_t numIndices = meshData->numIndices;
    const size_t vec3BufferSize = numPositions * sizeof(Vector3);

    return createMesh(dynamic,
        Primitive::TRIANGLE,
        meshData->positions, meshData->numPositions,
        meshData->indices, meshData->numIndices,
        meshData->colors, meshData->uv0, meshData->uv1, meshData->normals);
  }

  Handle<Mesh> Scene::createMesh(bool dynamic, Primitive primitive,
      const Vector3* vertices, int numVertices,
      const unsigned int* indices, int numIndices,
      const Color* color,
      const Vector2* uv0,
      const Vector2* uv1,
      const Vector3* normals)
  {
    Handle<Mesh> handle = meshes.reserve();
    Mesh* mesh = meshes.lookup(handle);
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
    return handle;
  }

  void Scene::updateMesh(Handle<Mesh> handle,
      const Vector3* vertices, int numVertices,
      const unsigned int* indices, int numIndices,
      const Color* color,
      const Vector2* uv0,
      const Vector2* uv1,
      const Vector3* normals)
  {

    Mesh* mesh = meshes.lookup(handle);
    if (!mesh->dynamic)
    {
      Log::warning("Unable to update a static mesh");
      return;
    }

    bool resizeBuffers = false;

    if (vertices)
    {
      size_t verticesArraySize = numVertices * sizeof(Vector3);

      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      if (verticesArraySize > mesh->verticesArraySize)
      {
        resizeBuffers = true;
        mesh->verticesArraySize = verticesArraySize;
        glBufferData(GL_ARRAY_BUFFER, verticesArraySize, vertices, GL_DYNAMIC_DRAW);
      }
      else
      {
        glBufferSubData(GL_ARRAY_BUFFER, 0, verticesArraySize, vertices);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      mesh->numVertices = (unsigned int) (verticesArraySize / sizeof(Vector3));
    }

    if (indices)
    {
      if (!mesh->ibo)
      {
        Log::warning("Unable to update indices for mesh created without index buffer.");
      }
      else
      {
        size_t indicesArraySize = numIndices * sizeof(unsigned int);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
        if (indicesArraySize > mesh->indicesArraySize)
        {
          mesh->indicesArraySize = indicesArraySize;
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize, indices, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indicesArraySize, indices);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        mesh->numIndices = (unsigned int) (indicesArraySize / sizeof(unsigned int));
      }
    }

    if(color)
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
          glBufferData(GL_ARRAY_BUFFER, size, color, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, color);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(uv0)
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
          glBufferData(GL_ARRAY_BUFFER, size, uv0, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, uv0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(uv1)
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
          glBufferData(GL_ARRAY_BUFFER, size, uv1, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, uv1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }
  }

  void Scene::destroyMesh(Mesh* mesh)
  {
    GLuint buffers[SMOL_MAX_BUFFERS_PER_MESH];
    int numBuffers = 0;

    if (mesh->ibo) buffers[numBuffers++] = mesh->ibo;
    if (mesh->vboPosition) buffers[numBuffers++] = mesh->vboPosition;
    if (mesh->vboNormal) buffers[numBuffers++] = mesh->vboNormal;
    if (mesh->vboUV0) buffers[numBuffers++] = mesh->vboUV0;
    if (mesh->vboUV1) buffers[numBuffers++] = mesh->vboUV1;

    glDeleteBuffers(numBuffers, (const GLuint*) buffers);
    glDeleteVertexArrays(1, (const GLuint*) &mesh->vao);
  }

  void Scene::destroyMesh(Handle<Mesh> handle)
  {
    Mesh* mesh = meshes.lookup(handle);
    if(!mesh)
    {
      warnInvalidHandle("Mesh");
    }
    else
    {
      destroyMesh(mesh);
      meshes.remove(handle);
    }
  }

  // ##################################################################
  //  SpriteBatcher handling 
  // ##################################################################
  Handle<SpriteBatcher> Scene::createSpriteBatcher(Handle<Material> material, int capacity)
  {
    Handle<SpriteBatcher> handle = batchers.reserve();
    SpriteBatcher* batcher =  batchers.lookup(handle);
    batcher->arena.initialize(capacity * SpriteBatcher::totalSpriteSize + 1);

    // It doesn't matter the contents of memory. Nothing is read from this pointer. It's just necessary to create a valid MeshData;
    char* memory = batcher->arena.pushSize(capacity * SpriteBatcher::totalSpriteSize);
    MeshData meshData((Vector3*)memory, capacity, 
        (unsigned int*)memory, capacity * 6,
        (Color*) memory, nullptr,
        (Vector2*) memory, nullptr);
    Handle<Renderable> renderable = createRenderable(material, createMesh(true, &meshData));

    batcher->renderable = renderable;
    batcher->spriteCount = 0;
    batcher->spriteCapacity = capacity;
    batcher->dirty = false;

    return handle;
  }

  // ##################################################################
  //  Renderable resource handling 
  // ##################################################################

  Handle<Renderable> Scene::createRenderable(Handle<Material> material, Handle<Mesh> mesh)
  {
    Handle<Renderable> handle = renderables.reserve();
    Renderable* renderable = renderables.lookup(handle);
    renderable->mesh = mesh;
    renderable->material = material;
    return handle;
  }

  void Scene::destroyRenderable(Handle<Renderable> handle)
  {
    Renderable* renderable = renderables.lookup(handle);
    if(!renderable)
    {
      warnInvalidHandle("Renderable");
    }
    else
    {
      renderables.remove(handle);
    }
  }

  // ##################################################################
  //  Shader resource handling 
  // ##################################################################

  Handle<ShaderProgram> Scene::createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource)
  {
    //TODO(marcio): There must be a "default" shader we might use in case we fail to build a shader program.

    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);
    shader->valid = false;
    shader->programId = 0;

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
      return handle;
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
      return handle;
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
        return handle;
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
      return handle;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    if (gShader) glDeleteShader(gShader);

    shader->programId = program;
    shader->valid = true;
    return handle;
  }

  Handle<ShaderProgram> Scene::createShader(const char* vsFilePath,
      const char* fsFilePath,
      const char* gsFilePath)
  {
    //TODO(marcio): There must be a "default" shader we might use in case we fail to build a shader program.

    char* vertexSource = vsFilePath ? Platform::loadFileToBufferNullTerminated(vsFilePath) : nullptr;
    char* fragmentSource = fsFilePath ? Platform::loadFileToBufferNullTerminated(fsFilePath) : nullptr;
    char* geometrySource = gsFilePath ? Platform::loadFileToBufferNullTerminated(gsFilePath) : nullptr;

    Handle<ShaderProgram> handle = createShaderFromSource(vertexSource, fragmentSource, geometrySource);

    if (vertexSource) Platform::unloadFileBuffer(vertexSource);
    if (fragmentSource) Platform::unloadFileBuffer(fragmentSource);
    if (geometrySource) Platform::unloadFileBuffer(geometrySource);
    return handle;
  }

  void Scene::destroyShader(ShaderProgram* program)
  {
    glDeleteProgram(program->programId);
  }

  void Scene::destroyShader(Handle<ShaderProgram> handle)
  {
    ShaderProgram* program = shaders.lookup(handle);
    if (!program)
    {
      warnInvalidHandle("ShaderProgram");
    }
    else
    {
      destroyShader(program);
      shaders.remove(handle);
    }
  }

  // ##################################################################
  //  Node resource handling 
  // ##################################################################

  Handle<SceneNode> Scene::createMeshNode(
      Handle<Renderable> renderable,
      Vector3& position,
      Vector3& scale,
      Vector3& rotationAxis,
      float rotationAngle,
      Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.reserve();
    SceneNode* node = nodes.lookup(handle);

    node->type = SceneNode::MESH;
    node->parent = parent;
    node->transform.setPosition(position.x, position.y, position.z);
    node->transform.setRotation(rotationAxis.x, rotationAxis.y, rotationAxis.z, rotationAngle);
    node->transform.setScale(scale.x, scale.y, scale.z);
    node->transform.update();

    node->meshNode.renderable = renderable;

    return handle;
  }

  Handle<SceneNode> Scene::createSpriteNode(
      Handle<SpriteBatcher> batcher,
      Rect& rect,
      Vector3& position,
      float width,
      float height,
      const Color& color,
      int angle,
      Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.reserve();
    SceneNode* node = nodes.lookup(handle);

    node->type = SceneNode::SPRITE;
    node->parent = parent;
    node->transform.setPosition(position.x, position.y, position.z);
    node->transform.setRotation(0.0f, 0.0f, 1.0f, (float)angle);
    node->transform.setScale(1.0f, 1.0f, 1.0f);
    node->transform.update();
    node->spriteNode.rect = rect;
    node->spriteNode.batcher = batcher;
    node->spriteNode.width = width;
    node->spriteNode.height = height;
    node->spriteNode.angle = angle;
    node->spriteNode.color = color;

    SpriteBatcher* batcherPtr = batchers.lookup(batcher);
    if (batcherPtr)
    {
      batcherPtr->spriteCount++;
      batcherPtr->dirty = true;
    }

    return handle;
  }

  Handle<SceneNode> Scene::clone(Handle<SceneNode> handle)
  {
    Handle<SceneNode> newHandle = nodes.reserve();
    SceneNode* newNode = nodes.lookup(newHandle);
    SceneNode* original = nodes.lookup(handle);
    memcpy(newNode, original, sizeof(SceneNode));

    //TODO(marcio): this won't work for sprites because it does not update spriteCount on the spriteBatcher. Fix it!
    return newHandle;
  }
}

#undef INVALID_HANDLE
#undef warnInvalidHandle

