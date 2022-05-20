#include <smol/smol_game.h>
#include <smol/smol_renderer.h>
#include <smol/smol_assetmanager.h>

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
smol::Handle<smol::SpriteBatcher> batcher;

int shape = 0;

void onStart(smol::SystemsRoot* systemsRoot)
{
  smol::Log::info("Game started!");
  root = systemsRoot;
  smol::Scene& scene = *(root->loadedScene);

  mesh = scene.createMesh(true, &(smol::MeshData::getPrimitiveCube()));

  texture = scene.createTexture("assets\\smol32.bmp");
  texture2 = scene.createTexture("assets\\smol16.bmp");

  shader = scene.createShader("assets\\default.vs", "assets\\default.fs");
  material = scene.createMaterial(shader, &texture, 1);
  material2 = scene.createMaterial(shader, &texture2, 1);

  auto renderable = scene.createRenderable(material, mesh);
  auto renderable2 = scene.createRenderable(material2, mesh);
  auto floor = scene.createRenderable(
      scene.createMaterial(shader, &(scene.createTexture(*smol::AssetManager::createCheckersImage(600, 600, 100))) ,1),
      scene.createMesh(false, &(smol::MeshData::getPrimitiveQuad()))
      );

  batcher = scene.createSpriteBatcher(material);

  scene.createMeshNode(floor, 
      smol::Vector3{0.0f, -5.0f, -5.0f},
      smol::Vector3{100.0f, 100.0f, 100.0f},
      smol::Vector3{-90, 0.0f, 0.0f});

  scene.createSpriteNode(batcher,
      smol::Rect{120, 580, 710, 200},
      smol::Vector3{0.0f, 0.0f, 0.0f},
      350.0f, 100.0f, smol::Color::WHITE);

  scene.createSpriteNode(batcher, 
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{200.0f, 200.0f, 0.0f},
      100.0f, 100.0f, smol::Color::GREEN);

  scene.createSpriteNode(batcher, 
      smol::Rect{0, 0, 800, 800},
      smol::Vector3{400.0f, 200.0f, 0.0f},
      100.0f, 100.0f, smol::Color::BLUE);

  scene.createMeshNode(renderable2, 
      smol::Vector3{-2.0f, 0.0f, -5.0f},
      smol::Vector3{0.5f, 0.5f, 0.5f});

  node1 = scene.createMeshNode(renderable2,
      smol::Vector3{0.0f, 0.0f, -5.0f},
      smol::Vector3{1.0f, 1.0f, 1.0f});

  node2 = scene.createMeshNode(renderable2, 
      smol::Vector3{0.0f, 3.0f, 0.0f},
      smol::Vector3{1.0f, 1.0f, 1.0f},
      smol::Vector3{0.0f, 0.0f, 0.0f},
      node1);

  selectedNode = node2;
}

unsigned int angle = 0;
bool once = true;

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

    const smol::MeshData* m;

    shape++;
    if (shape > 4)
      shape = 0;

    switch(shape)
    {
      case 0:
        m = &(smol::MeshData::getPrimitiveCylinder());
        break;
      case 1:
        m = &(smol::MeshData::getPrimitiveCube());
        break;
      case 2:
        m = &(smol::MeshData::getPrimitiveSphere());
        break;
      case 3:
        m = &(smol::MeshData::getPrimitiveQuad());
        break;
      case 4:
        m = &(smol::MeshData::getPrimitiveCone());
        break;
    }

    scene.updateMesh(mesh, (smol::MeshData*) m);
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
            smol::Vector3{x * spriteWidth, y * spriteHeight, 0.0f},
            spriteWidth, spriteHeight,
            smol::Color(rand() % 256, rand() % 256, rand() % 256));
      }
    }
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
    scene.destroyMaterial(material2);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE))
  {
    scene.clone(selectedNode);
    //scene.setNodeActive(selectedNode, !scene.isNodeActive(selectedNode));
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

  if (xDirection || yDirection || zDirection || scaleAmount)
  {
    smol::Transform* transform = scene.getTransform(selectedNode);
    if (transform)
    {
      const float amount = 0.04f;
      const float moveAmount = amount;// selectedNode == node1 ? 1.0f : amount;

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

  smol::Transform* transform = scene.getTransform(node1);
  if (transform)
  {
    float a = (float) ++angle;
    transform->setRotation(0, a, 0);
  }

}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

