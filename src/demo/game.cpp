#include <smol/smol_game.h>
#include <smol/smol_renderer.h>
#include <smol/smol_point.h>
#include <smol/smol_assetmanager.h>

smol::SystemsRoot* root;
smol::Handle<smol::SceneNode> node1;
smol::Handle<smol::SceneNode> node2;
smol::Handle<smol::SceneNode> sprite1;
smol::Handle<smol::SceneNode> sprite2;
smol::Handle<smol::SceneNode> selectedNode;
smol::Handle<smol::Texture> texture2;
smol::Handle<smol::ShaderProgram> shader;
smol::Handle<smol::Material> material2;
smol::Handle<smol::Mesh> mesh;
smol::Handle<smol::SpriteBatcher> batcher;

int shape = 0;

void onStart(smol::SystemsRoot* systemsRoot)
{
  smol::Log::info("Game started!");
  root = systemsRoot;
  smol::Scene& scene = *(root->loadedScene);

  mesh = scene.createMesh(true, &(smol::MeshData::getPrimitiveCube()));

  auto texture = scene.loadTexture("assets\\smol.texture");
  texture2 = scene.createTexture("assets\\smol16.bmp");

  shader = scene.loadShader("assets\\default.shader");
  auto material = scene.createMaterial(shader, &texture, 1);
  material2 = scene.createMaterial(shader, &texture2, 1);

  auto renderable = scene.createRenderable(material, mesh);
  auto renderable2 = scene.createRenderable(material2, mesh);
  auto floor = scene.createRenderable(
      scene.createMaterial(shader, &(scene.createTexture(*smol::AssetManager::createCheckersImage(600, 600, 100))) ,1),
      scene.createMesh(false, &(smol::MeshData::getPrimitiveQuad()))
      );

  // meshes

  scene.createMeshNode(floor, 
      smol::Vector3{0.0f, -5.0f, -5.0f},
      smol::Vector3{100.0f, 100.0f, 100.0f},
      smol::Vector3{-90, 0.0f, 0.0f});

  // center cube
  node1 = scene.createMeshNode(renderable2, smol::Vector3{0.0f, -1.0f, -10.0f});

  // left cube
  node2 = scene.createMeshNode(renderable2, 
      smol::Vector3{0.0f, 1.0f, 0.0f},
      smol::Vector3{0.8f, 0.8f, 0.8f},
      smol::Vector3{1.0f, 1.0f, 1.0f},
      node1);

  // right cube
  scene.createMeshNode(renderable2, 
      smol::Vector3{4.0f, 0.0f, -10.0f},
      smol::Vector3{0.8f, 0.8f, 0.8f},
      smol::Vector3{0.0f, 0.0f, 0.0f}
      );

  // sprites
  scene.createMeshNode(renderable, smol::Vector3{0.0f, 0.0f, 0.0f});

  batcher = scene.createSpriteBatcher(material);
  sprite1 = scene.createSpriteNode(batcher,
      smol::Rect{120, 580, 710, 200},
      smol::Vector3{1.0f, 1.0f, 0.0f},
      350.0f, 100.0f, smol::Color::WHITE);

  sprite2 = scene.createSpriteNode(batcher, 
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{200.0f, 200.0f, 0.0f},
      100.0f, 100.0f, smol::Color::GREEN);

  scene.createSpriteNode(batcher, 
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{400.0f, 200.0f, 0.0f},
      100.0f, 100.0f, smol::Color::BLUE);
  selectedNode = node2;
}

unsigned int angle = 0;
bool once = true;

int spriteXDirection = 1;
int spriteYDirection = 1;

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = *root->keyboard;
  smol::Mouse& mouse = *root->mouse;
  smol::Scene& scene = *(root->loadedScene);
  smol::Renderer& renderer = *(root->renderer);

  int xDirection = 0;
  int yDirection = 0;
  int zDirection = 0;
  int scaleAmount = 0;
  if (mouse.getButton(smol::MOUSE_BUTTON_LEFT))
  {
    smol::Point2 p = mouse.getCursorPosition();
    smol::Transform* transform = scene.getTransform(sprite2);
    transform->setPosition((float) p.x, (float) p.y, 0.2f);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_T))
  {
    smol::MeshData* m;
    shape++;
    if (shape > 4)
      shape = 0;

    switch(shape)
    {
      case 0:
        m = (smol::MeshData*) &(smol::MeshData::getPrimitiveCylinder());
        break;
      case 1:
        m = (smol::MeshData*) &(smol::MeshData::getPrimitiveSphere());
        break;
      case 2:
        m = (smol::MeshData*) &(smol::MeshData::getPrimitiveCone());
        break;
      case 3:
        m = (smol::MeshData*) &(smol::MeshData::getPrimitiveQuad());
        break;
      case 4:
        m = (smol::MeshData*) &(smol::MeshData::getPrimitiveCube());
        break;
    }

    scene.updateMesh(mesh, m);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F4) && once)
  {
    once = false;
    const int numHSprites = 20;
    const int numVSprites = 20;
    smol::Rect viewport =
      root->renderer->getViewport();
    float spriteWidth = viewport.w /(float) numHSprites;
    float spriteHeight = viewport.h /(float) numVSprites;

    for (int x = 0; x < numHSprites; x++)
    {
      for (int y = 0; y < numVSprites; y++)
      {
        scene.createSpriteNode(batcher, 
            smol::Rect{0, 0, 800, 800},
            smol::Vector3{x * spriteWidth, y * spriteHeight, 0.1f},
            spriteWidth, spriteHeight,
            smol::Color(rand() % 256, rand() % 256, rand() % 256));
      }
    }
  }


  if (keyboard.getKeyDown(smol::KEYCODE_F5)) { scene.destroyShader(shader); }

  if (keyboard.getKeyDown(smol::KEYCODE_F6)) { scene.destroyTexture(texture2); }

  if (keyboard.getKeyDown(smol::KEYCODE_F7)) { scene.destroyMaterial(material2); }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE)) 
  { 
    bool active = scene.isNodeActive(selectedNode);
    scene.setNodeActive(selectedNode, !active);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_TAB)) { selectedNode = (selectedNode == node1) ? node2 : node1; }

  // left/right
  if (keyboard.getKey(smol::KEYCODE_A)) { xDirection = -1; }
  else if (keyboard.getKey(smol::KEYCODE_D)) { xDirection = 1; }

  // up/down movement
  if (keyboard.getKey(smol::KEYCODE_W)) { yDirection = -1; }
  else if (keyboard.getKey(smol::KEYCODE_S)) { yDirection = 1; }

  // back/forth movement
  if (keyboard.getKey(smol::KEYCODE_Q)) { zDirection = -1; }
  else if (keyboard.getKey(smol::KEYCODE_E)) { zDirection = 1; }

  if (keyboard.getKey(smol::KEYCODE_J)) { scaleAmount = -1; }
  if (keyboard.getKey(smol::KEYCODE_K)) { scaleAmount = 1; }

  if (xDirection || yDirection || zDirection || scaleAmount)
  {
    smol::Transform* transform = scene.getTransform(selectedNode);
    if (transform)
    {
      const float amount = 3.0f * deltaTime;

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

  smol::Transform* transform = scene.getTransform(node1);
  if (transform)
  {
    float a = transform->getRotation().y + 30 * deltaTime;
    transform->setRotation(0, a, 0);
  }

  // bounce sprite1 across the screen borders
  transform = scene.getTransform(sprite1);

  smol::Vector3& position = (smol::Vector3&) transform->getPosition();
  smol::Rect viewport = (smol::Rect&) renderer.getViewport();

  if (position.x + 250 > viewport.w || position.x < 0)
    spriteXDirection *= -1;

  if (position.y + 100 > viewport.h || position.y < 0)
    spriteYDirection *= -1;

  transform->setPosition(
      position.x + (100 * spriteXDirection) * deltaTime,
      position.y + (100 * spriteYDirection) * deltaTime,
      position.z);
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

