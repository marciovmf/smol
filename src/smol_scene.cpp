#include <smol/smol_scene.h>
#include <smol/smol_platform.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector2.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_systems_root.h>
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

  Scene::Scene(ResourceManager& resourceManager):
    meshes(32 * sizeof(Mesh)),
    renderables(1024 * sizeof(Renderable)),
    nodes(1024 * sizeof(SceneNode)), 
    batchers(8 * sizeof(SpriteBatcher)),
    renderKeys(1024 * sizeof(uint64)),
    renderKeysSorted(1024 * sizeof(uint64)),
    clearColor(160/255.0f, 165/255.0f, 170/255.0f),
    clearOperation((ClearOperation)(COLOR_BUFFER | DEPTH_BUFFER))
  {
    viewMatrix = Mat4::initIdentity();

    // Creates a ROOT node
    nodes.add((const SceneNode&) SceneNode(this, SceneNode::SceneNodeType::ROOT, INVALID_HANDLE(SceneNode)));

    defaultShader   = resourceManager.getDefaultShader();
    defaultTexture  = resourceManager.getDefaultTexture();
    defaultMaterial = resourceManager.getDefaultMaterial();
  }

  //---------------------------------------------------------------------------
  // SceneNode
  //---------------------------------------------------------------------------
  SceneNode::SceneNode(Scene* scene, SceneNodeType type, Handle<SceneNode> parent) 
    : scene(*scene), active(true), dirty(true), type(type)
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


  //---------------------------------------------------------------------------
  // Scene
  //---------------------------------------------------------------------------

  void Scene::setNodeActive(Handle<SceneNode> handle, bool status)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return;
    node->setActive(status);
  }

  bool Scene::isNodeActiveInHierarchy(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return false;
    return node->isActiveInHierarchy();
  }

  bool Scene::isNodeActive(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if(!node) return false;
    return node->isActive();
  }
  
  // 
  // Resources: Textures, Materials, Meshes, Renderables
  //

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
