#include <smol/smol_game.h>

smol::SystemsRoot* root;
smol::Handle<smol::SceneNode> node1;
smol::Handle<smol::SceneNode> node2;
smol::Handle<smol::SceneNode> node3;
smol::Handle<smol::SceneNode> selectedNode;

smol::Handle<smol::Texture> texture;
smol::Handle<smol::Texture> texture2;
smol::Handle<smol::ShaderProgram> shader;
smol::Handle<smol::Material> material;
smol::Handle<smol::Material> material2;
smol::Handle<smol::Material> materialTest;
smol::Handle<smol::Mesh> mesh;

void onStart(smol::SystemsRoot* systemsRoot)
{
  smol::Log::info("Game started!");
  root = systemsRoot;
  smol::Scene& scene = *(root->loadedScene);

  mesh = scene.createMesh(true, &(smol::MeshData::getPrimitiveCube()));

  texture = scene.createTexture("assets\\smol32.bmp");
  texture2 = scene.createTexture("assets\\ldk.bmp");
  shader = scene.createShader("assets\\default.vs", "assets\\default.fs");
  material = scene.createMaterial(shader, &texture, 1);
  material2 = scene.createMaterial(shader, &texture2, 1);

  auto renderable = scene.createRenderable(material, mesh);
  auto renderable2 = scene.createRenderable(material2, mesh);
  auto batcher = scene.createSpriteBatcher(material);

  //node1 = scene.createMeshNode(renderable, smol::Vector3{0.0f, 0.0f, 0.0f});
  //
  node1 = scene.createSpriteNode(batcher,
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{0.0f, 200.0f, 0.0f},
      100.0f, 100.0f);

  node2 = scene.createMeshNode(renderable2, 
      smol::Vector3{0.0f, 0.0f, -2.0f},
      smol::Vector3{0.4f, 0.4f, 0.4f},
      smol::Vector3{0.0f, 0.5f, 1.0f}, 45.0f);

   scene.createSpriteNode(batcher, 
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{200.0f, 200.0f, 0.0f},
      100.0f, 100.0f);

   scene.createMeshNode(renderable2, 
      smol::Vector3{-1.0f, 0.0f, -2.0f},
      smol::Vector3{0.2f, 0.2f, 0.2f},
      smol::Vector3{0.0f, 0.5f, 1.0f}, 30.0f);

   scene.createSpriteNode(batcher, 
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{400.0f, 200.0f, 0.0f},
      100.0f, 100.0f);

   scene.createMeshNode(renderable2, 
      smol::Vector3{1.6f, 0.0f, -4.0f},
      smol::Vector3{0.2f, 0.2f, 0.2f},
      smol::Vector3{0.5f, 1.0f, 0.0f}, 30.0f);

  selectedNode = node2;
}

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = *root->keyboard;
  smol::Scene& scene = *(root->loadedScene);

  int xDirection = 0;
  int yDirection = 0;
  int zDirection = 0;
  int scaleAmount = 0;

  if (keyboard.getKeyDown(smol::KEYCODE_T))
  {
    const smol::MeshData* cone = &(smol::MeshData::getPrimitiveCone());
    scene.updateMesh(mesh,
        cone->positions, cone->numPositions,
        cone->indices, cone->numIndices,
        cone->colors, cone->uv0, cone->uv1, cone->normals);
  }
 
  if (keyboard.getKeyDown(smol::KEYCODE_Y))
  {
    const smol::MeshData* arrow = &(smol::MeshData::getPrimitiveCube());

    scene.updateMesh(mesh,
        arrow->positions, arrow->numPositions,
        arrow->indices, arrow->numIndices,
        arrow->colors,
        arrow->uv0,
        arrow->uv1,
        arrow->normals);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_U))
  {
    const smol::MeshData* sphere = &(smol::MeshData::getPrimitiveSphere());

    scene.updateMesh(mesh,
        sphere->positions, sphere->numPositions,
        sphere->indices, sphere->numIndices,
        sphere->colors, sphere->uv0, sphere->uv1, sphere->normals);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F5))
  {
    scene.destroyShader(shader);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F6))
  {
    scene.destroyTexture(texture2);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F7))
  {
    scene.destroyMaterial(material);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE))
  {
    scene.clone(selectedNode);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_TAB))
  {
    selectedNode = (selectedNode == node1) ? node2 : node1;
  }

  // left/right
  if (keyboard.getKey(smol::KEYCODE_A))
  {
    xDirection = -1;
  }
  else if (keyboard.getKey(smol::KEYCODE_D))
  {
    xDirection = 1;
  }

  // up/down movement
  if (keyboard.getKey(smol::KEYCODE_W))
  {
    yDirection = -1;
  }
  else if (keyboard.getKey(smol::KEYCODE_S))
  {
    yDirection = 1;
  }

  // back/forth movement
  if (keyboard.getKey(smol::KEYCODE_Q))
  {
    zDirection = -1;
  }
  else if (keyboard.getKey(smol::KEYCODE_E))
  {
    zDirection = 1;
  }

  if (keyboard.getKey(smol::KEYCODE_J))
  {
    scaleAmount = -1;
  }
  if (keyboard.getKey(smol::KEYCODE_K))
  {
    scaleAmount = 1;
  }

  smol::Transform* transform = scene.getTransform(selectedNode);

  if (xDirection || yDirection || zDirection || scaleAmount)
  {
    const float amount = 0.01f;
    //const float moveAmount = selectedNode == node1 ? 1.0f : amount;
    const float moveAmount = 1.0f;

    const smol::Vector3& position = transform->getPosition();
    transform->setPosition(
        moveAmount * xDirection + position.x,
        moveAmount * yDirection + position.y,
        moveAmount * zDirection + position.z);

    const smol::Vector3& scale = transform->getScale();
    transform->setScale(
        amount * scaleAmount + scale.x,
        amount * scaleAmount + scale.y,
        amount * scaleAmount + scale.z);

  }

}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

