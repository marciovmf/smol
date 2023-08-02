#include <smol/smol_scene.h>
#include <smol/smol_scene_node.h>
#include <smol/smol_platform.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_renderer.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector2.h>
#include <smol/smol_cfg_parser.h>
#include <string.h>
#include <utility>

#define warnInvalidHandle(typeName) debugLogWarning("Attempting to reference a '%s' resource from an invalid handle", (typeName))
namespace smol
{
  Scene::Scene():
    renderables(32),
    nodes(32), 
    batchers(8),
    renderKeys(1024 * sizeof(uint64)),
    renderKeysSorted(1024 * sizeof(uint64))
  {
    viewMatrix = Mat4::initIdentity();
  }

  Scene::~Scene()
  {
    debugLogInfo("Scene Released Renderable x%d, SpriteBatcher x%d, SceneNode x%d.", 
        renderables.count(),
        batchers.count(),
        nodes.count());
  }

  //
  // Create / Destroy scene resources
  //

  Handle<Renderable> Scene::createRenderable(Handle<Material> material, Handle<Mesh> mesh)
  {
    Handle<Renderable> handle = renderables.add(Renderable(material, mesh));
    Renderable* renderable = renderables.lookup(handle);
    if (!renderable)
    {
      return INVALID_HANDLE(Renderable);
    }
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

  Handle<SpriteBatcher> Scene::createSpriteBatcher(Handle<Material> material, int capacity)
  {
    return batchers.add(SpriteBatcher(material, capacity));
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
      batchers.remove(handle);
    }
  }

#ifndef SMOL_MODULE_GAME
  Handle<SceneNode> Scene::createNode(SceneNode::Type type, const Transform& transform)
  {
    Handle<SceneNode> handle = nodes.add(SceneNode(this, type, transform));
    handle->setDirty(true);
    return handle;
  }
#endif

#ifndef SMOL_MODULE_GAME
  void Scene::destroyNode(Handle<SceneNode> handle)
  {
    nodes.remove(handle);
  }
#endif

  int Scene::getNodeCount() const
  {
    return nodes.count();
  }

  const SceneNode* Scene::getNodes(uint32* count) const
  {
    if (count)
      *count = nodes.count();
    return nodes.getArray();
  }

  //
  // Internal rendering utility functions
  //

  /**
   * param elements - pointer to 64bit integers to be sorted.
   * param elementCount - number of elements on elements array.
   * param dest - destination buffer where to put the sorted list. This buffer
   * must be large enough for storing elementCount elements.
   */
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

  static void drawRenderable(const Renderable* renderable)
  {
    const Mesh* mesh = renderable->mesh.operator->();
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
    const SceneNode* allNodes = scene->getNodes();

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

  void Scene::render(float deltaTime)
  {
    ResourceManager& resourceManager = ResourceManager::get();
    const GLuint defaultShaderProgramId = resourceManager.getDefaultShader()->glProgramId;
    const Material& defaultMaterial = resourceManager.getDefaultMaterial();

    const SceneNode* allNodes = nodes.getArray();
    int numNodes = nodes.count();

    renderKeys.reset();
    renderKeysSorted.reset();

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
            node->transform.update(*this);
            key = encodeRenderKey(node->getType(), 0, node->camera.getPriority(), i);
            numCameras++;
          }
          break;

        case SceneNode::MESH:
          {
            node->transform.update(*this);
            renderable = renderables.lookup(node->mesh.renderable);
            Handle<Material> material = renderable->material;
            key = encodeRenderKey(node->getType(), (uint16)(material.slotIndex), material->renderQueue, i);
          }
          break;

        case SceneNode::TEXT:
          {
            node->transform.update(*this);
            if (node->transform.isDirty(*this) || node->isDirty())
            {
              SpriteBatcher* batcher = batchers.lookup(node->text.batcher);
              batcher->dirty = true;
            }
            Handle<Material> material = node->text.batcher->material;
            key = encodeRenderKey(node->getType(), (uint16)(material.slotIndex), material->renderQueue, i);
          }
          break;
        case SceneNode::SPRITE:
          {
            node->transform.update(*this);
            if (node->transform.isDirty(*this) || node->isDirty())
            {
              SpriteBatcher* batcher = batchers.lookup(node->sprite.batcher);
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
      node->transform.update(*this);
      uint64* keyPtr = (uint64*) renderKeys.pushSize(sizeof(uint64));
      *keyPtr = key;
    }

    // ----------------------------------------------------------------------
    // Sort keys
    const int32 numKeysToSort = (int32) (renderKeys.getUsed() / sizeof(uint64));
    radixSort((uint64*) renderKeys.getData(), numKeysToSort, (uint64*) renderKeysSorted.pushSize(renderKeys.getUsed()));

    // Cameras will be the first nodes on the sorted list. We use that to iterate all cameras
    uint64* allCameraKeys = (uint64*) renderKeysSorted.getData();
    uint64* allRenderKeys = allCameraKeys + numCameras;
    const int32 numKeys = numKeysToSort - numCameras; // don't count with camera nodes;

    for(int cameraIndex = 0; cameraIndex < numCameras; cameraIndex++)
    {
      uint64 cameraKey = allCameraKeys[cameraIndex];
      SceneNode* cameraNode = (SceneNode*) &allNodes[getNodeIndexFromRenderKey(cameraKey)];
      SMOL_ASSERT(cameraNode->typeIs(SceneNode::Type::CAMERA), "SceneNode is CAMERA", cameraNode->getType());

      //TODO(marcio): When we have an event system, linsten for display resized event and only update camera matrices in case of an event.
      cameraNode->camera.update();

      // ----------------------------------------------------------------------
      // VIEWPORT

      const Rect viewport = Renderer::getViewport();
      const Rectf& cameraRect = cameraNode->camera.getViewportRect();
      Rect screenRect;
      screenRect.x = (size_t)(viewport.w * cameraRect.x);
      screenRect.y = (size_t)(viewport.h * cameraRect.y);
      screenRect.w = (size_t)(viewport.w * cameraRect.w);
      screenRect.h = (size_t)(viewport.h * cameraRect.h);

      Renderer::setViewport((GLsizei) screenRect.x, (GLsizei) screenRect.y, (GLsizei) screenRect.w, (GLsizei) screenRect.h);

      // ----------------------------------------------------------------------
      // CLEAR
      const Color& clearColor = cameraNode->camera.getClearColor();
      Renderer::setClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);

      unsigned int clearOperation = cameraNode->camera.getClearOperation();
      if (clearOperation != Renderer::ClearBufferFlag::CLEAR_NONE)
      {
        Renderer::clearBuffers(clearOperation);
      }

      // ----------------------------------------------------------------------
      // set uniform buffer matrices based on current camera

      Renderer::updateGlobalShaderParams(
          cameraNode->camera.getProjectionMatrix(),
          cameraNode->transform.getMatrix().inverse(),
          Mat4::initIdentity(),
          deltaTime);

      // ----------------------------------------------------------------------
      // Draw render keys
      int currentMaterialIndex = -1;
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
          Renderer::setMaterial(&material);
        }

        if (node->typeIs(SceneNode::MESH)) 
        {
          if(!(cameraLayers & node->getLayer()))
            continue;

          Renderer::updateGlobalShaderParams(
              cameraNode->camera.getProjectionMatrix(),
              cameraNode->transform.getMatrix().inverse(),
              node->transform.getMatrix(),
              deltaTime);

          Renderable* renderable = renderables.lookup(node->mesh.renderable);
          drawRenderable(renderable);
        }
        else if (node->typeIs(SceneNode::TEXT))
        {
          if(!(cameraLayers & node->getLayer()))
            continue;

          Renderer::updateGlobalShaderParams(
              cameraNode->camera.getProjectionMatrix(),
              cameraNode->transform.getMatrix().inverse(),
              node->transform.getMatrix(),
              deltaTime);

          SpriteBatcher* batcher = batchers.lookup(node->text.batcher);
          batcher->begin();
          batcher->pushTextNode(node);
          batcher->end();
        }
        else if (node->typeIs(SceneNode::SPRITE))
        {

          Renderer::updateGlobalShaderParams(
              cameraNode->camera.getProjectionMatrix(),
              cameraNode->transform.getMatrix().inverse(),
              node->transform.getMatrix(),
              deltaTime);

          SpriteBatcher* batcher = batchers.lookup(node->sprite.batcher);
          drawSpriteNodes(this, batcher, allRenderKeys + i, cameraLayers);
          i+= (batcher->spriteNodeCount - 1);
        }
        else
        {
          //TODO(marcio): Implement scpecific render logic for each type of node
          continue; 
        }
      }
    }

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

#undef INVALID_HANDLE
#undef warnInvalidHandle
