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

  unsigned int indices[] = {0, 3, 2, 2, 1, 0};
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
        nullptr, 0,
        uv, sizeof(uv)                  // uv0
        );

  auto texture = scene.createTexture("assets\\smol32.bmp");
  auto shader = scene.createShader("assets\\default.vs", "assets\\default.fs");
  auto material = scene.createMaterial(shader, &texture, 1);
  auto renderable = scene.createRenderable(material, mesh);

  node1 = scene.createNode(renderable, smol::Vector3{-0.5f, 0.0f, 0.0f});
  node2 = scene.createNode(renderable, smol::Vector3{0.5f, 0.0f, -0.2f}, smol::Vector3{0.5f, 0.5f, 0.5f}, 
      smol::Vector3{.0f, .0f, 1.0f}, 45.0f);
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
    smol::Log::info("%f, %f, %f", position.x, position.y, position.z);
  }
  
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

