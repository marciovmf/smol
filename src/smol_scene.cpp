#include <smol/smol_scene.h>
#include <smol/smol_scene_nodes.h>
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
    mainCamera(INVALID_HANDLE(SceneNode)),
    clearOperation((ClearOperation)(COLOR_BUFFER | DEPTH_BUFFER)),
    nullSceneNode(SceneNode(this, SceneNode::Type::INVALID))
  {
    viewMatrix = Mat4::initIdentity();

    // Creates a ROOT node
    nodes.add((const SceneNode&) SceneNode(this, SceneNode::Type::ROOT));

    defaultShader   = resourceManager.getDefaultShader();
    defaultTexture  = resourceManager.getDefaultTexture();
    defaultMaterial = resourceManager.getDefaultMaterial();
  }

  //---------------------------------------------------------------------------
  // Scene
  //---------------------------------------------------------------------------

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
  Handle<SceneNode> Scene::createMeshNode(Handle<Renderable> renderable, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::MESH, transform));
    nodes.lookup(handle)->meshNode.renderable = renderable;
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
    const Transform& t = Transform(
        position,
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        parent);

    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::SPRITE, t));
    SceneNode* node = nodes.lookup(handle);

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

  Handle<SceneNode> Scene::createPerspectiveCameraNode(float fov, float aspect, float zNear, float zFar, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::CAMERA, transform));
    SceneNode* node = nodes.lookup(handle);
    node->camera.setPerspective(fov, aspect, zNear, zFar);
    node->camera.setLayerMask((uint32) Layer::LAYER_0);
    return handle;
  }

  Handle<SceneNode> Scene::createOrthographicCameraNode(float left, float right, float top, float bottom, float zNear, float zFar, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, SceneNode::CAMERA, transform));
    SceneNode* node = nodes.lookup(handle);
    node->camera.setOrthographic(left, right, top, bottom, zNear, zFar);
    node->camera.setLayerMask((uint32) Layer::LAYER_0);
    return handle;
  }

  void Scene::setMainCamera(Handle<SceneNode> handle)
  {
    SceneNode* node = nodes.lookup(handle);
    if (!node || !node->typeIs(SceneNode::Type::CAMERA))
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

  SceneNode& Scene::getNode(Handle<SceneNode> handle) const
  {
    SceneNode* node = nodes.lookup(handle);
    if(node)
      return *node;

    return (SceneNode&) nullSceneNode;
  }
}

#undef INVALID_HANDLE
#undef warnInvalidHandle
