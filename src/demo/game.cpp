#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_log.h>
#include <smol/smol_renderer.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_resource_list.h>

smol::SystemsRoot* root;

struct Foo
{
  unsigned int value;
};

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

  auto texture = scene.createTexture("assets\\smol24.bmp");
  auto shader = scene.createShader("assets\\default.vs", "assets\\default.fs");
  auto material = scene.createMaterial(shader, &texture, 1);
  auto renderable = scene.createRenderable(material, mesh);
}

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = *root->keyboard;
  if (keyboard.getKeyDown(smol::KEYCODE_J))
    smol::Log::info("Pressed this frame");
  else if (keyboard.getKeyUp(smol::KEYCODE_J))
    smol::Log::info("Released this frame");
  else if (keyboard.getKey(smol::KEYCODE_J))
    smol::Log::info("Holding...");
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

