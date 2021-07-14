#ifndef SMOL_RENDERER_H
#define SMOL_RENDERER_H

#include <vector>
#include <smol/smol_engine.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mat4.h>
#include <smol/smol_gl.h>
#include <smol/smol_resource_list.h>

namespace smol
{
  enum ClearOperation : char
  {
    DONT_CLEAR = 0,
    COLOR_BUFFER = 1,
    DEPTH_BUFFER = 1 << 1
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
    TRIANGLE_STRIP
  };

  enum SceneNodeType : char
  {
    EMPTY,
    MESH,
    SPRITE
  };

  struct Texture
  {
    GLuint textureObject;
  };

#define SMOL_MATERIAL_TEXTURE_DIFFUSE_MAX 6
  struct Material
  {
    GLuint shaderProgram;
    smol::Handle<Texture> textureDiffuse[SMOL_MATERIAL_TEXTURE_DIFFUSE_MAX];
    int renderQueue;
  };

  struct Mesh
  {
    Primitive primitive;
    GLuint vao;
    GLuint vboVertex;
    GLuint vboNormal;
    GLuint vboUV0;
    GLuint vboUV1;
    GLuint vboIndex;

    unsigned int numIndices;
    unsigned int numVertices;
  };

  struct Renderable
  {
    smol::Handle<Material> Material;
    smol::Handle<Mesh> Mesh;
  };

  struct Transform
  {
    Mat4 mat;
  };

  struct EmptySceneNode { };

  struct MeshSceneNode
  {
    smol::Handle<Renderable> renderable;
  };

  struct SpriteSceneNode
  {
    float x, y, w, h;
    smol::Handle<Renderable> renderable;
  };

  struct SceneNode
  {
    SceneNodeType type;
    Transform transform;
    smol::Handle<Transform> parentTransform;

    union
    {
      EmptySceneNode emptySceneNode;
      MeshSceneNode meshSceneNode;
      SpriteSceneNode spriteSceneNode;
    };
  };

  struct Scene
  {
    smol::ResourceList<Texture> Textures;
    smol::ResourceList<Material> Materials;
    smol::ResourceList<Mesh> Meshes;
    smol::ResourceList<Renderable> Renderables;
    smol::ResourceList<SceneNode> SceneNodes;

    Vector3 clearColor = {0.29f, 0.29f, 0.29f};
    ClearOperation clearOperation;

  };

  struct SMOL_ENGINE_API Renderer
  {
    int width;
    int height;
    Scene* scene;

    Renderer (Scene& scene, int width, int height);
    void setScene(Scene& scene);          // Unloads the current loaded scene, if any, and loads the given scene.
    void resize(int width, int height);   // Resizes the necessary resources to accomodathe the required dimentions.
    void render();                        // Called once per frame to render the scene.
  };

}
#endif  // SMOL_RENDERER_H

