#include <smol/smol_scene.h>
#include <smol/smol_platform.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector2.h>
#include <smol/smol_cfg_parser.h>
#include <string.h>

namespace smol
{
  const size_t SpriteBatcher::positionsSize = 4 * sizeof(Vector3);
  const size_t SpriteBatcher::colorsSize = 4 * sizeof(Color);
  const size_t SpriteBatcher::uvsSize = 4 * sizeof(Vector2);
  const size_t SpriteBatcher::indicesSize = 6 * sizeof(unsigned int);
  const size_t SpriteBatcher::totalSpriteSize = positionsSize + colorsSize + uvsSize + indicesSize;


  // First node handle always points to the ROOT scene node
  const Handle<SceneNode> Scene::ROOT = (Handle<SceneNode>{ (int) 0, (int) 0 });

  Scene::Scene():
    shaders(32), textures(64), materials(32), meshes(32), renderables(32), nodes(128), 
    batchers(8), renderKeys((size_t)255), renderKeysSorted((size_t)255),
    clearColor(160/255.0f, 165/255.0f, 170/255.0f), clearOperation((ClearOperation)(COLOR_BUFFER | DEPTH_BUFFER))
  {
    viewMatrix = Mat4::initIdentity();
    Image* img = AssetManager::createCheckersImage(800, 600, 32);
    defaultTexture = createTexture(*img);
    AssetManager::unloadImage(img);

    // Creates a ROOT node
    nodes.add((const SceneNode&) SceneNode(this, SceneNode::Type::ROOT, INVALID_HANDLE(SceneNode)));

    // store the default shader program in the scene
    ShaderProgram program = Renderer::getDefaultShaderProgram();
    defaultShader = shaders.add(program);

    defaultMaterial = createMaterial(defaultShader, &defaultTexture, 1);
  }

  //---------------------------------------------------------------------------
  // SceneNode
  //---------------------------------------------------------------------------
  SceneNode::SceneNode(Scene* scene, SceneNode::Type type, Handle<SceneNode> parent) 
    : scene(*scene), active(true), dirty(true), type(type), layer(Layer::LAYER_0)
  { 
    transform.setParent(parent);
  }

  bool SceneNode::isActive()
  {
    return active;
  }

  bool SceneNode::isActiveInHierarchy()
  {
    if (!active)
      return false;

    Handle<SceneNode> parent = transform.getParent();

    if (parent == Scene::ROOT)
      return true;

    SceneNode* parentPtr = scene.getNode(parent);
    if (!parentPtr)
      return false;
    
    return parentPtr->isActiveInHierarchy();
  }

  void SceneNode::setParent(Handle<SceneNode> parent)
  {
    // root nodes have no parent
    if (type == SceneNode::ROOT)
      return;

    if (!scene.getNode(parent))
    {
      Log::error("Trying to set parent of node with an invalid parent node handle");
      transform.setParent(Scene::ROOT);
      return;
    }
    transform.setParent(parent);
  }

  void SceneNode::setActive(bool status)
  {
    active = status;
    dirty = true;
  }

  void SceneNode::setLayer(Layer layer)
  {
    this->layer = layer;
  }


  //---------------------------------------------------------------------------
  // Scene
  //---------------------------------------------------------------------------

  void Scene::setNodeActive(Handle<SceneNode> handle, bool status)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return;
    node->setActive(status);
  }

  bool Scene::isNodeActive(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return false;
    return node->isActive();
  }

  bool Scene::isNodeActiveInHierarchy(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return false;
    return node->isActiveInHierarchy();
  }

  void Scene::setLayer(Handle<SceneNode> handle, Layer layer)
  {
    SceneNode* node = nodes.lookup(handle);
    if (node)
    {
     node->setLayer(layer);
    }
  }

  // 
  // Resources: Textures, Materials, Meshes, Renderables
  //


  Handle<Texture> Scene::loadTexture(const char* path)
  {
    debugLogInfo("Loading texture %s", path);
    if (!path)
      return INVALID_HANDLE(Texture);

    Config config(path);
    ConfigEntry* entry = config.findEntry((const char*) "texture");

    if (!entry)
    {
      Log::error("Unable to load texture '%s'", path);
      return INVALID_HANDLE(Texture);
    }

    const char* imagePath = entry->getVariableString((const char*) "image", nullptr);
    unsigned int wrap = (unsigned int) entry->getVariableNumber((const char*) "wrap", 0.0f);
    unsigned int filter = (unsigned int) entry->getVariableNumber((const char*) "filter", 0.0f);
    unsigned int mipmap = (unsigned int) entry->getVariableNumber((const char*) "mipmap", 0.0f);

    if (wrap >= Texture::Wrap::MAX_WRAP_OPTIONS)
    {
      wrap = 0;
      Log::error("Invalid wrap value in Texture file '%s'");
    }

    if (filter >= Texture::Filter::MAX_FILTER_OPTIONS)
    {
      filter = 0;
      Log::error("Invalid filter value in Texture file '%s'");
    }

    if (mipmap >= Texture::Mipmap::MAX_MIPMAP_OPTIONS)
    {
      mipmap = 0;
      Log::error("Invalid mipmap value in Texture file '%s'");
    }

    return createTexture(imagePath, (Texture::Wrap) wrap, (Texture::Filter) filter, (Texture::Mipmap) mipmap);
  }

  Handle<Texture> Scene::createTexture(const char* path, Texture::Wrap wrap, Texture::Filter filter, Texture::Mipmap mipmap)
  {
    Image* image = AssetManager::loadImageBitmap(path);
    Handle<Texture> texture = createTexture(*image, wrap, filter, mipmap);
    AssetManager::unloadImage(image);
    return texture;
  }

  Handle<Texture> Scene::createTexture(const Image& image, Texture::Wrap wrap, Texture::Filter filter, Texture::Mipmap mipmap)
  {
    Handle<Texture> texture = textures.reserve();
    Texture* texturePtr = textures.lookup(texture);
    bool success = Renderer::createTexture(texturePtr, image, wrap, filter, mipmap);

    if (texturePtr && success)
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

  Handle<Material> Scene::loadMaterial(const char* path)
  {
    debugLogInfo("Loading material %s", path);
    if (!path)
      return INVALID_HANDLE(Material);

    Config config(path);
    ConfigEntry* materialEntry = config.findEntry((const char*)"material");

    if (!materialEntry)
      return INVALID_HANDLE(Material);

    int numDiffuseTextures = 0;
    Handle<Texture> diffuseTextures[SMOL_MATERIAL_MAX_TEXTURES];

    const char* shaderPath = materialEntry->getVariableString((const char*) "shader", nullptr);
    if (!shaderPath)
    {
      Log::error("Invalid material file '%s'. First entry must be 'shader'.", path);
      return INVALID_HANDLE(Material);
    }

    //TODO(marcio): We must be able to know if the required shader is already loaded. If it is we should use it instead of loading it again!
    Handle<ShaderProgram> shader = loadShader(shaderPath);
    int renderQueue = (int) materialEntry->getVariableNumber((const char*)"queue", (float) RenderQueue::QUEUE_OPAQUE);

    const char* STR_TEXTURE = (const char*) "texture";
    ConfigEntry *textureEntry = config.findEntry(STR_TEXTURE);
    while(textureEntry)
    {
      // Diffuse texture
      const char* diffuseTexture = textureEntry->getVariableString((const char*) "diffuse");
      if (diffuseTexture)
      {
        if (numDiffuseTextures >= SMOL_MATERIAL_MAX_TEXTURES)
        {
          Log::error("Material file '%s' exceeded the maximum of %d diffuse textures. The texture '%s' will be ignored.",
              path, SMOL_MATERIAL_MAX_TEXTURES, diffuseTexture);
        }
        else
        {
          diffuseTextures[numDiffuseTextures++] = loadTexture(diffuseTexture);
        }
      }
      textureEntry = config.findEntry(STR_TEXTURE, textureEntry);
    }

    Handle<Material> handle = createMaterial(shader, diffuseTextures, numDiffuseTextures, renderQueue);
    Material* material = materials.lookup(handle);

    //set values for material parameters
    for(int i = 0; i < material->parameterCount; i++)
    {
      MaterialParameter& param = material->parameter[i];
      switch(param.type)
      {
        case ShaderParameter::SAMPLER_2D:
          {
            // The sampler_2d is an index for the material's texture list
            uint32 textureIndex = (uint32) materialEntry->getVariableNumber(param.name);
            if (textureIndex >= (uint32) numDiffuseTextures)
            {
              Log::error("Material parameter '%s' references an out of bounds texture index %d",
                  param.name, textureIndex);
              textureIndex = 0;
            }
            param.uintValue = textureIndex;
          }
          break;
        case ShaderParameter::VECTOR2:
          param.vec2Value = materialEntry->getVariableVec2(param.name);
          break;
        case ShaderParameter::VECTOR3:
          param.vec3Value = materialEntry->getVariableVec3(param.name);
          break;
        case ShaderParameter::VECTOR4:
          param.vec4Value = materialEntry->getVariableVec4(param.name);
          break;
        case ShaderParameter::FLOAT:
          param.floatValue = materialEntry->getVariableNumber(param.name);
          break;
        case ShaderParameter::INT:
          param.intValue = (int) materialEntry->getVariableNumber(param.name);
          break;
        case ShaderParameter::UNSIGNED_INT:
          param.uintValue = (uint32) materialEntry->getVariableNumber(param.name);
          break;
        case ShaderParameter::INVALID:
          break;
      }
    }

    return handle;
  }

  Handle<Material> Scene::createMaterial(Handle<ShaderProgram> shader,
      Handle<Texture>* diffuseTextures, int diffuseTextureCount, int renderQueue)
  {
    SMOL_ASSERT(diffuseTextureCount <= SMOL_MATERIAL_MAX_TEXTURES, "Exceeded Maximum diffuse textures per material");

    Handle<Material> handle = materials.reserve();
    Material* material = materials.lookup(handle);
    memset(material, 0, sizeof(Material));

    if (diffuseTextureCount)
    {
      size_t copySize = diffuseTextureCount * sizeof(Handle<Texture>);
      material->renderQueue = renderQueue;
      material->shader = shader;
      material->diffuseTextureCount = diffuseTextureCount;
      memcpy(material->textureDiffuse, diffuseTextures, copySize);
    }

    ShaderProgram* shaderPtr = shaders.lookup(shader);
    if (shaderPtr)
    {
      material->parameterCount = shaderPtr->parameterCount;
      uint32 texturesAssigned = 0;

      for(int i = 0; i < shaderPtr->parameterCount; i++)
      {
        MaterialParameter& materialParam = material->parameter[i];
        const ShaderParameter& shaderParam = shaderPtr->parameter[i];
        //We copy sizeof(ShaderParameter) to the MaterialParameter so the values remain untouched.
        //This is intentional since we memset(0) the whole material after allocating it.
        memcpy(&materialParam, &shaderParam, sizeof(ShaderParameter));

        // for robustness, if the parameter is a texture and there are textures, we try to assing them as they apper
        if (materialParam.type == ShaderParameter::SAMPLER_2D && diffuseTextureCount)
        {
          if (texturesAssigned >= (uint32) diffuseTextureCount)
            texturesAssigned = 0;

          materialParam.uintValue = texturesAssigned++;
        }
      }
    }

    return handle;
  }

  Material* Scene::getMaterial(Handle<Material> handle)
  {
    return materials.lookup(handle);
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

  Handle<Mesh> Scene::createMesh(bool dynamic, const MeshData& meshData)
  {
    return createMesh(dynamic,
        Primitive::TRIANGLE,
        meshData.positions, meshData.numPositions,
        meshData.indices, meshData.numIndices,
        meshData.colors, meshData.uv0, meshData.uv1, meshData.normals);
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

  Handle<Renderable> Scene::createRenderable(Handle<Material> material, Handle<Mesh> mesh)
  {
    Handle<Renderable> handle = renderables.reserve();
    Renderable* renderable = renderables.lookup(handle);
    if (!renderable)
    {
      return INVALID_HANDLE(Renderable);
    }

    renderable->material = material;
    renderable->mesh = mesh;
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
    Handle<Renderable> renderable = createRenderable(material, createMesh(true, meshData));

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

  Handle<ShaderProgram> Scene::createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource)
  {
    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);
    Renderer::createShaderProgram(shader, vsSource, fsSource, gsSource);
    return handle;
  }

  Handle<ShaderProgram> Scene::loadShader(const char* filePath)
  {
    if (!filePath)
    {
      return INVALID_HANDLE(ShaderProgram);
    }

    Config config(filePath);
    ConfigEntry* entry = config.entries;

    const char* STR_VERTEX_SHADER = "vertexShader";
    const char* STR_FRAGMENT_SHADER = "fragmentShader";
    const char* STR_GEOMETRY_SHADER = "geometryShader";

    const char* vsSource = entry->getVariableString(STR_VERTEX_SHADER, nullptr);
    const char* fsSource = entry->getVariableString(STR_FRAGMENT_SHADER, nullptr);
    const char* gsSource = nullptr;

    if (entry->variableCount == 3)
      gsSource = entry->getVariableString(STR_GEOMETRY_SHADER, nullptr);

    if (vsSource == nullptr || fsSource == nullptr)
    {
      Log::error("Invalid shader source file '%s'. First entry must be 'vertexShader', then 'fragmentShader', and an optional 'geometryShader'.", filePath);
      return INVALID_HANDLE(ShaderProgram);
    }

    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);

    Renderer::createShaderProgram(shader, vsSource, fsSource, gsSource);
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

  //
  // Scene Node utility functions
  //
  Handle<SceneNode> Scene::createMeshNode(
      Handle<Renderable> renderable,
      const Vector3& position,
      const Vector3& scale,
      const Vector3& rotation,
      Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::MESH, parent));
    SceneNode* node = nodes.lookup(handle);

    node->transform.setPosition(position.x, position.y, position.z);
    node->transform.setRotation(rotation.x, rotation.y, rotation.z);
    node->transform.setScale(scale.x, scale.y, scale.z);
    node->transform.update(&nodes);

    node->meshNode.renderable = renderable;
    return handle;
  }

  Handle<SceneNode> Scene::createSpriteNode(
      Handle<SpriteBatcher> batcher,
      const Rect& rect,
      const Vector3& position,
      float width,
      float height,
      const Color& color,
      int angle,
      Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::SPRITE, parent));
    SceneNode* node = nodes.lookup(handle);

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
      // copy the renreable handle to the node level
      node->spriteNode.renderable = batcherPtr->renderable;

      batcherPtr->spriteCount++;
      batcherPtr->dirty = true;
    }

    return handle;
  }

  Handle<SceneNode> Scene::createPerspectiveCameraNode(float fov, float aspect, float zNear, float zFar, const Transform& transform, Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::CAMERA, parent));
    SceneNode* node = nodes.lookup(handle);

    node->cameraNode.camera.setPerspective(fov, aspect, zNear, zFar);
    node->cameraNode.camera.setLayers((uint32) Layer::LAYER_0);

    const Vector3& p = transform.getPosition();
    const Vector3& s = transform.getScale();
    const Vector3& r = transform.getRotation();
    node->transform.setPosition(p.x, p.y, p.z);
    node->transform.setRotation(r.x, r.y, r.z);
    node->transform.setScale(s.x, s.y, s.z);
    node->transform.update(&nodes);

    return handle;
  }

  Handle<SceneNode> Scene::createOrthographicCameraNode(float left, float right, float top, float bottom, float zNear, float zFar, const Transform& transform, Handle<SceneNode> parent)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::CAMERA, parent));
    SceneNode* node = nodes.lookup(handle);

    node->cameraNode.camera.setOrthographic(left, right, top, bottom, zNear, zFar);
    node->cameraNode.camera.setLayers((uint32) Layer::LAYER_0);

    const Vector3& p = transform.getPosition();
    const Vector3& s = transform.getScale();
    const Vector3& r = transform.getRotation();
    node->transform.setPosition(p.x, p.y, p.z);
    node->transform.setRotation(r.x, r.y, r.z);
    node->transform.setScale(s.x, s.y, s.z);
    node->transform.update(&nodes);

    return handle;
  }

  void Scene::setMainCamera(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if (!node || node->type != SceneNode::Type::CAMERA)
      return;

    mainCamera = handle;
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

  SceneNode* Scene::getNode(Handle<SceneNode> handle)
  {
    return nodes.lookup(handle);
  }

  Transform* Scene::getTransform(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if (!node)
      return nullptr;

    return &node->transform;
  }
}

#undef INVALID_HANDLE
#undef warnInvalidHandle
