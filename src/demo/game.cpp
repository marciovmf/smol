#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_renderer.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_resource_list.h>

smol::SystemsRoot* root;
smol::Handle<smol::SceneNode> node1;
smol::Handle<smol::SceneNode> node2;
smol::Handle<smol::SceneNode> selectedNode;

void onStart(smol::SystemsRoot* systemsRoot)
{
  smol::Log::info("Game started!");
  root = systemsRoot;
  smol::Scene& scene = *(root->loadedScene);

  unsigned int indices[] = {0, 1, 2, 2, 3, 0};
  smol::Vector3 vertices[] =
  {
    {0.5f,  0.5f, 0.0f},  // top right
    {0.5f, -0.5f, 0.0f},  // bottom right
    {-0.5f, -0.5f, 0.0f}, // bottom left
    {-0.5f,  0.5f, 0.0f}, // top left 
  };

  smol::Vector2 uv[] =
  {
    {1.0f, 1.0f},  // top right
    {1.0f, 0.0f},  // bottom right
    {0.0f, 0.0f},  // bottom left
    {0.0f, 1.0f}   // top left 
  };

  auto mesh = scene.createMesh(
        smol::Primitive::TRIANGLE,      // primitive
        vertices, sizeof(vertices),     // positions
        indices,  sizeof(indices),      // indices
        uv, sizeof(uv),                 // uv0
        nullptr, 0,                     // uv1
        nullptr, 0                      // normals
        );

  auto texture = scene.createTexture("assets\\smol32.bmp");
  auto shader = scene.createShader("assets\\default.vs", "assets\\default.fs");
  auto material = scene.createMaterial(shader, &texture, 1);
  auto renderable = scene.createRenderable(material, mesh);

  node1 = scene.createNode(renderable, smol::Vector3{-0.5f, 0.0f, 0.0f});
  node2 = scene.createNode(renderable, smol::Vector3{0.5f, 0.0f, 0.0f}, smol::Vector3{0.5f, 0.5f, 0.5f});
  selectedNode = node2;
}

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = *root->keyboard;
  smol::Scene& scene = *(root->loadedScene);

  int xDirection = 0;
  int yDirection = 0;
  int zDirection = 0;


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

  if (xDirection || yDirection || zDirection)
  {
    const float amount = 0.005f;
    smol::Transform* transform = scene.getTransform(selectedNode);
    transform->translate(xDirection * amount, yDirection * amount, zDirection * amount);
  }
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

