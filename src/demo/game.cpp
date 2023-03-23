#include <smol/smol_game.h>
#include <smol/smol_renderer.h>
#include <smol/smol_point.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <utility>
#include <time.h>

smol::SystemsRoot* root;
smol::Handle<smol::SceneNode> floorNode;
smol::Handle<smol::SceneNode> node1;
smol::Handle<smol::SceneNode> node2;
smol::Handle<smol::SceneNode> sprite1;
smol::Handle<smol::SceneNode> sprite2;
smol::Handle<smol::SceneNode> selectedNode;
smol::Handle<smol::Texture> texture2;
smol::Handle<smol::ShaderProgram> shader;
smol::Handle<smol::Material> checkersMaterial;
smol::Handle<smol::Mesh> mesh;
smol::Handle<smol::SpriteBatcher> batcher;

int shape = 0;

void onStart()
{
  smol::Log::info("Game started!");
  root = smol::SystemsRoot::get();
  smol::ResourceManager& resourceManager = root->resourceManager;

  smol::ConfigEntry* gameConfig = root->config.findEntry("game");
  uint32 seed = (uint32) gameConfig->getVariableNumber("seed", 0);
  smol::Log::info("seed = %d", seed);
  srand(seed);

  smol::Scene& scene = root->loadedScene;

  mesh = scene.createMesh(true,  smol::MeshData::getPrimitiveCube());
  shader = resourceManager.loadShader("assets/default.shader");
  auto checkersTexture = resourceManager.createTexture(*smol::ResourceManager::createCheckersImage(600, 600, 100));
  checkersMaterial = resourceManager.createMaterial(shader, &checkersTexture, 1);

  resourceManager.getMaterial(checkersMaterial)
    ->setVec4("color", smol::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

  // Manually create a material
  auto floorMaterial = resourceManager.createMaterial(shader, &checkersTexture, 1);
  auto floor = scene.createRenderable(floorMaterial,
      scene.createMesh(false, smol::MeshData::getPrimitiveQuad())); 

  resourceManager.getMaterial(floorMaterial)
    ->setVec4("color", smol::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

  auto renderable2 = scene.createRenderable(checkersMaterial, mesh);

  // meshes
  floorNode = scene.createMeshNode(floor, 
      smol::Transform(
        smol::Vector3(0.0f, -5.0f, -5.0f),
        smol::Vector3(-90, 0.0f, 0.0f),
        smol::Vector3(100.0f, 100.0f, 100.0f)));

  // center cube
  node1 = scene.createMeshNode(renderable2,
      smol::Transform(
        smol::Vector3{0.0f, -1.0f, -15.0f},
        smol::Vector3(0.0f, 0.0f, 0.0f),
        smol::Vector3{2.0f, 2.0f, 2.0f}));

  // left cube
  node2 = scene.createMeshNode(renderable2, 
      smol::Transform(
        smol::Vector3(0.0f, 1.0f, 0.0f),
        smol::Vector3(1.0f, 1.0f, 1.0f),
        smol::Vector3(0.8f, 0.8f, 0.8f),
        node1));

  // right cube
  scene.createMeshNode(renderable2, 
      smol::Transform(
        smol::Vector3(4.0f, 3.0f, -10.0f),
        smol::Vector3(0.8f, 0.8f, 0.8f)));

  scene.getNode(floorNode).setLayer(smol::Layer::LAYER_1);


  // camera
  smol::Transform t;
  t.setParent(node2);
  smol::Rect viewport =
    root->renderer.getViewport();
  auto camera = scene.createPerspectiveCameraNode(60.0f, viewport.w/(float)viewport.h, 0.01f, 100.0f, t);
  scene.getNode(camera).cameraNode.camera.setLayerMask((uint32)(smol::Layer::LAYER_0 | smol::Layer::LAYER_1));
  scene.setMainCamera(camera);

  // Create a grass field

  auto grassRenderable1 = scene.createRenderable(
      resourceManager.loadMaterial("assets/grass_03.material"),
      scene.createMesh(false, smol::MeshData::getPrimitiveQuad()));

  auto grassRenderable2 = scene.createRenderable(
      resourceManager.loadMaterial("assets/grass_02.material"),
      scene.createMesh(false, smol::MeshData::getPrimitiveQuad()));

  float minZ = -90.0f;
  const int changeLimit = 3;
  int nextChange = 0;

  for (int i = 0; i < 5000; i++)
  {
    float randX = (rand() % 1000 - rand() % 1000) / 1000.0f;
    float randZ = minZ + ((rand() % 1000) / 1000.0f) * 0.5f;
    minZ = randZ;

    float randAngle = (rand() % 270 - rand() % 270) * 1.0f;
    float randScale = (rand() % 30 - rand() % 30) / 30.0f;
    randScale *= 1.5f;

    smol::Transform t;
    nextChange = ++nextChange % changeLimit;
    t.setPosition(100 * randX, -2.0f, randZ);
    t.setRotation(0.0f, randAngle, 0.0f);
    t.setScale(2.0f + randScale, 2.0f + randScale, 2.0f + randScale);

    scene.createMeshNode(
        (nextChange == 0) ? grassRenderable1 : grassRenderable2, t);
  }

  // sprites
  auto texture = resourceManager.loadTexture("assets/smol.texture");
  auto smolMaterial = resourceManager.createMaterial(shader, &texture, 1);

  resourceManager.getMaterial(smolMaterial)
    ->setVec4("color", smol::Vector4(0.0f, 0.5f, 0.3f, 0.8f));

  batcher = scene.createSpriteBatcher(smolMaterial);
  sprite1 = scene.createSpriteNode(batcher,
      (const smol::Rect&) smol::Rect(120, 580, 710, 200),
      (const smol::Vector3&) smol::Vector3(1.0f, 1.0f, 0.0f),
      350.0f, 100.0f, 
      (const smol::Color) smol::Color::WHITE);

  sprite2 = scene.createSpriteNode(batcher, 
      smol::Rect(0, 0, 800, 800),
      smol::Vector3(200.0f, 200.0f, 0.0f),
      100.0f, 100.0f, smol::Color::GREEN);

  scene.createSpriteNode(batcher, 
      smol::Rect(0, 0, 800, 800),
      smol::Vector3(400.0f, 200.0f, 0.0f),
      100.0f, 100.0f, smol::Color::BLUE);

  selectedNode = node2;
}

unsigned int angle = 0;
bool once = true;

int spriteXDirection = 1;
int spriteYDirection = 1;

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = root->keyboard;
  smol::Mouse& mouse = root->mouse;
  smol::Scene& scene = root->loadedScene;
  smol::Renderer& renderer = root->renderer;

  int xDirection = 0;
  int yDirection = 0;
  int zDirection = 0;
  int scaleAmount = 0;
  if (mouse.getButton(smol::MOUSE_BUTTON_LEFT))
  {
    smol::Point2 p = mouse.getCursorPosition();
    scene.getNode(sprite2).transform.setPosition((float) p.x, (float) p.y, 0.2f);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_T))
  {
    smol::MeshData m;
    shape++;
    if (shape > 4)
      shape = 0;

    switch(shape)
    {
      case 0:
        m = smol::MeshData::getPrimitiveCylinder();
        break;
      case 1:
        m = smol::MeshData::getPrimitiveSphere();
        break;
      case 2:
        m = smol::MeshData::getPrimitiveCone();
        break;
      case 3:
        m = smol::MeshData::getPrimitiveQuad();
        break;
      case 4:
        m = smol::MeshData::getPrimitiveCube();
        break;
    }

    scene.updateMesh(mesh, &m);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F4) && once)
  {
    once = false;
    int numHSprites = 20;
    int numVSprites = 20;
    smol::ConfigEntry* entry = root->config.findEntry("game");
    if (entry)
    {
      numHSprites = (int) entry->getVariableNumber("numHSprites", (float) numHSprites);
      numVSprites = (int) entry->getVariableNumber("numVSprites", (float) numVSprites);
    }

    smol::Rect viewport =
      root->renderer.getViewport();
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

  if (keyboard.getKeyDown(smol::KEYCODE_F5)) { root->resourceManager.destroyShader(shader); }

  if (keyboard.getKeyDown(smol::KEYCODE_F7)) { root->resourceManager.destroyMaterial(checkersMaterial); }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE)) 
  { 
    smol::SceneNode& node = scene.getNode(selectedNode);
    node.setActive(!node.isActive());
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
    smol::Transform* transform = &scene.getNode(selectedNode).transform;
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

  smol::Transform* transform = &scene.getNode(node1).transform;
  if (transform)
  {
    float a = transform->getRotation().y + 30 * deltaTime;
    transform->setRotation(0, a, 0);
  }

  // bounce sprite1 across the screen borders
  transform = &scene.getNode(sprite1).transform;

  smol::Vector3 position = transform->getPosition();
  smol::Rect viewport = renderer.getViewport();

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

