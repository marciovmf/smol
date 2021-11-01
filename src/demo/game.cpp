#include <smol/smol_game.h>

smol::SystemsRoot* root;
smol::Handle<smol::SceneNode> node1;
smol::Handle<smol::SceneNode> node2;
smol::Handle<smol::SceneNode> selectedNode;

smol::Handle<smol::Texture> texture;
smol::Handle<smol::ShaderProgram> shader;
smol::Handle<smol::Material> material;
smol::Handle<smol::Mesh> mesh;

void onStart(smol::SystemsRoot* systemsRoot)
{
  smol::Log::info("Game started!");
  root = systemsRoot;
  smol::Scene& scene = *(root->loadedScene);

  mesh = scene.createMesh(false, &(smol::MeshData::getPrimitiveQuad()));

  texture = scene.createTexture("assets\\smol32.bmp");
  shader = scene.createShader("assets\\default.vs", "assets\\default.fs");
  material = scene.createMaterial(shader, &texture, 1);
  auto renderable = scene.createRenderable(material, mesh);

  node1 = scene.createNode(renderable, smol::Vector3{-0.0f, 0.0f, 0.0f});
  node2 = scene.createNode(renderable,
      smol::Vector3{0.5f, 0.0f, -0.2f}, // position
      smol::Vector3{0.5f, 0.5f, 0.5f},  // scale
      smol::Vector3{0.0f, 0.0f, 1.0f}, 45.0f);// rotation axis + angle

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

  if (keyboard.getKeyDown(smol::KEYCODE_F5))
  {
    scene.destroyShader(shader);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F6))
  {
    scene.destroyTexture(texture);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F7))
  {
    scene.destroyMaterial(material);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE))
  {
    scene.clone(selectedNode);
    //scene.updateMeshAttribute(mesh, Mesh::POSITION, 0, 4 * 3 * sizeof(float))
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
    yDirection = 1;
  }
  else if (keyboard.getKey(smol::KEYCODE_S))
  {
    yDirection = -1;
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

    const smol::Vector3& position = transform->getPosition();
    transform->setPosition(
        amount * xDirection + position.x,
        amount * yDirection + position.y,
        amount * zDirection + position.z);

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

