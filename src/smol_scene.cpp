#include <smol/smol_platform.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_renderer.h>

namespace smol
{

  const size_t SpriteBatcher::positionsSize = 4 * sizeof(Vector3);
  const size_t SpriteBatcher::colorsSize = 4 * sizeof(Color);
  const size_t SpriteBatcher::uvsSize = 4 * sizeof(Vector2);
  const size_t SpriteBatcher::indicesSize = 6 * sizeof(unsigned int);
  const size_t SpriteBatcher::totalSpriteSize = positionsSize + colorsSize + uvsSize + indicesSize;

  const Handle<SceneNode> Scene::ROOT = INVALID_HANDLE(SceneNode);


  Scene::Scene():
    shaders(32), textures(64), materials(32), meshes(32), renderables(32), nodes(128), 
    batchers(8), renderKeys((size_t)255), renderKeysSorted((size_t)255),
    clearColor(160/255.0f, 165/255.0f, 170/255.0f), clearOperation((ClearOperation)(COLOR_BUFFER | DEPTH_BUFFER))
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
    node->active = status;
  }

  bool Scene::isNodeActive(Handle<SceneNode> handle)
  {
    return true;
    SceneNode* node = nodes.lookup(handle);
    if(!node)
      return true;

    return isNodeActive(node->transform.getParent());
  }

  Transform* Scene::getTransform(Handle<SceneNode> node)
  {
    SceneNode* nodePtr = nodes.lookup(node);
    return nodePtr ? &nodePtr->transform : nullptr;
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
    Handle<Texture> texture = textures.reserve();
    Texture* texturePtr = textures.lookup(texture);
    if (texturePtr && Renderer::createTextureFromImage(texturePtr, image))
      return texture;

    return INVALID_HANDLE(Texture);
  }

  void Scene::destroyTexture(Texture* texture)
  {
    Renderer::destroyTexture(texture);
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
    Renderer::createMesh(mesh, dynamic, primitive, vertices, numVertices, indices, numIndices, color, uv0, uv1, normals);
    return handle;
  }

  void Scene::updateMesh(Handle<Mesh> handle, MeshData* meshData)
  {
    Mesh* mesh = meshes.lookup(handle);
    Renderer::updateMesh(mesh, meshData);
  }

  void Scene::destroyMesh(Mesh* mesh)
  {
    Renderer::destroyMesh(mesh);
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

  void Scene::destroySpriteBatcher(Handle<SpriteBatcher> handle)
  {
    SpriteBatcher* batcher = batchers.lookup(handle);
    if(!batcher)
    {
      warnInvalidHandle("SpriteBatcher");
    }
    else
    {
      destroyRenderable(batcher->renderable);
      batchers.remove(handle);
    }
  }

  // ##################################################################
  //  Shader resource handling 
  // ##################################################################

  Handle<ShaderProgram> Scene::createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource)
  {
    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);
    Renderer::createShaderProgram(shader, vsSource, fsSource, gsSource);
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
    Renderer::destroyShaderProgram(program);
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
      Vector3& rotation,
      Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.reserve();
    SceneNode* node = nodes.lookup(handle);

    node->type = SceneNode::MESH;
    node->transform.setParent(parent);
    node->transform.setPosition(position);
    node->transform.setRotation(rotation);
    node->transform.setScale(scale);
    node->transform.update(&nodes);
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
    node->transform.setParent(parent);
    node->transform.setPosition(position);
    node->transform.setRotation(0.0f, 0.0f, 1.0f);
    node->transform.setScale(1.0f, 1.0f, 1.0f);
    node->transform.update(&nodes);
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

  void Scene::destroyNode(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node)
    {
      return;
    }

    nodes.remove(handle);
  }

}

#undef INVALID_HANDLE
#undef warnInvalidHandle
