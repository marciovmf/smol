#include <smol/smol_game.h>
#include <smol/smol_renderer.h>
#include <smol/smol_point.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <utility>
#include <time.h>

smol::SystemsRoot* root;
smol::Handle<smol::SceneNode> cameraNode;
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
float vpsize = 0.2f;

int shape = 0;

void onStart()
{
  smol::Log::info("Game started!");
  root = smol::SystemsRoot::get();
  smol::ResourceManager& resourceManager = root->resourceManager;
  smol::Scene& scene = root->loadedScene;

  // Read game specifig settings from variables.txt
  const smol::ConfigEntry* gameConfig = root->config.findEntry("game");
  uint32 seed = (uint32) gameConfig->getVariableNumber("seed", 0);
  smol::Log::info("seed = %d", seed);
  srand(seed);

  shader = resourceManager.loadShader("assets/default.shader");

  mesh = resourceManager.createMesh(true,  smol::MeshData::getPrimitiveCube());
  auto checkersTexture = resourceManager.createTexture(*smol::ResourceManager::createCheckersImage(600, 600, 100), 
      smol::Texture::Wrap::REPEAT, smol::Texture::Filter::NEAREST);
  checkersMaterial = resourceManager.createMaterial(shader, &checkersTexture,
      smol::Texture::Filter::NEAREST,
      smol::Texture::Mipmap::NEAREST_MIPMAP_NEAREST);

  resourceManager.getMaterial(checkersMaterial)
    .setVec4("color", (const smol::Vector4&) smol::Color::WHITE);

  // Manually create a material
  auto floorMaterial = resourceManager.createMaterial(shader, &checkersTexture, 1);
  resourceManager.getMaterial(floorMaterial)
    .setVec4("color",(const smol::Vector4&)  smol::Color::WHITE);


  auto floor = scene.createRenderable(floorMaterial,
      resourceManager.createMesh(false, smol::MeshData::getPrimitiveQuad())); 

  auto renderable2 = scene.createRenderable(checkersMaterial, mesh);

  // meshes
  floorNode = scene.createMeshNode(floor, 
      smol::Transform()
      .setPosition(0.0f, -5.0f, -0.0f)
      .setRotation(-90, 0.0f, 0.0f)
      .setScale(100.0f, 100.0f, 100.0f)
      );

  // center cube
  node1 = scene.createMeshNode(renderable2,
      smol::Transform()
      .setPosition(0.0f, -1.0f, 0.0f)
      .setRotation(0.0f, 0.0f, 0.0f)
      .setScale(2.0f, 2.0f, 2.0f)
      );

  // left cube
  node2 = scene.createMeshNode(renderable2, 
      smol::Transform()
      .setPosition(0.0f, 1.0f, -10.0f)
      .setRotation(1.0f, 1.0f, 1.0f)
      .setScale(0.8f, 0.8f, 0.8f)
      .setParent(node1)
      );

  // right cube
  scene.createMeshNode(renderable2, 
      smol::Transform()
      .setPosition(4.0f, 3.0f, -10.0f)
      .setRotation(0.8f, 0.8f, 0.8f)
      );

  // camera
  smol::Transform t;
  smol::Rect viewport = root->renderer.getViewport();
  cameraNode = scene.createPerspectiveCameraNode(60.0f, viewport.w/(float)viewport.h, 0.01f, 3000.0f, t);

  smol::SceneNode& leftCamera = scene.getNode(cameraNode);
  leftCamera.transform
    .setRotation(0.0f, 0.0f, 0.0f)
    .setPosition(0.0f, 0.0f, 0.5f);
  leftCamera.camera.setLayerMask((uint32)(smol::Layer::LAYER_0 | smol::Layer::LAYER_1 | smol::Layer::LAYER_2));


  //auto hCamera = scene.createPerspectiveCameraNode(60.0f, viewport.w/(float)viewport.h, 0.01f, 3000.0f, t);
  auto hCamera = scene.createOrthographicCameraNode(5.0f,  0.01f, 100.0f, t);
  smol::SceneNode& rightCamera = scene.getNode(hCamera);
  rightCamera.transform
    .setRotation(-30.0f, 0.0f, 0.0f)
    .setPosition(0.0f, 10.0f, 15.0f);
  rightCamera.camera.setViewportRect(smol::Rectf(0.75f, 0.0f, 0.25f, 0.25f));
  rightCamera.camera.setLayerMask((uint32)(smol::Layer::LAYER_0 | smol::Layer::LAYER_1));

  // Create a grass field
  auto grassRenderable1 = scene.createRenderable(
      resourceManager.loadMaterial("assets/grass_03.material"),
      resourceManager.createMesh(false, smol::MeshData::getPrimitiveQuad()));

  auto grassRenderable2 = scene.createRenderable(
      resourceManager.loadMaterial("assets/grass_02.material"),
      resourceManager.createMesh(false, smol::MeshData::getPrimitiveQuad()));

  const int changeLimit = 20;
  int nextChange = 0;

  for (int i = 0; i < 3000; i++)
  {
    float randX = (rand() % 100 - rand() % 100) * 1.0f;
    float randZ = (rand() % 100 - rand() % 100) * 1.0f;

    float randAngle = (rand() % 270 - rand() % 270) * 1.0f;
    float randScale = (rand() % 30) / 30.0f;

    smol::Transform t;
    nextChange = ++nextChange % changeLimit;

    bool isTallGrass;
    if (nextChange == 0)
    {
      isTallGrass = true;
      randScale *= 5.0f;
    }
    else
    {
      isTallGrass = false;
      randScale *= 3.0f;
    }

    t.setPosition(randX, -2.0f, randZ);
    //t.setRotation(0.0f, randAngle, 0.0f);
    t.setRotation(0.0f, 0.0f, 0.0f);
    t.setScale(randScale, randScale, randScale);

    auto handle = scene.createMeshNode( (isTallGrass) ? grassRenderable1 : grassRenderable2, t);
    scene.getNode(handle).setLayer(smol::Layer::LAYER_2);
  }

  // sprites
  auto texture = resourceManager.loadTexture("assets/smol.texture");
  auto smolMaterial = resourceManager.createMaterial(shader, &texture, 1);
  resourceManager.getMaterial(smolMaterial) .setVec4("color", smol::Vector4(1.0f, 1.0f, 1.0f, 1.0f));

  batcher = scene.createSpriteBatcher(smolMaterial);

  sprite1 = scene.createSpriteNode(batcher,
      (const smol::Rect&) smol::Rect(120, 580, 710, 200),
      (const smol::Vector3&) smol::Vector3(3.0f, 0.0f, 0.3f),
      4.0f, 2.0f, 
      (const smol::Color) smol::Color::WHITE);
  scene.getNode(sprite1).setLayer(smol::Layer::LAYER_1);

  sprite2 = scene.createSpriteNode(batcher, 
      smol::Rect(0, 0, 800, 800),
      smol::Vector3(2.0f, -2.0f, 0.4f),
      1.0f, 1.0f,
      smol::Color::WHITE);
  scene.getNode(sprite2).setLayer(smol::Layer::LAYER_1);

  auto sprite3 = scene.createSpriteNode(batcher, 
      smol::Rect(0, 0, 800, 800),
      smol::Vector3(-2.0f, 2.0f, 0.2f),
      1.0f, 1.0f, smol::Color::BLUE);
  scene.getNode(sprite3).setLayer(smol::Layer::LAYER_1);

  selectedNode = cameraNode;
}

unsigned int angle = 0;
bool once = true;

int spriteXDirection = 1;
int spriteYDirection = 1;

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = root->keyboard;
  smol::Mouse& mouse = root->mouse;
  smol::Renderer& renderer = root->renderer;
  smol::ResourceManager& resourceManager = root->resourceManager;
  smol::Scene& scene = root->loadedScene;

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

    smol::SystemsRoot::get()->resourceManager.updateMesh(mesh, &m);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F4) && once)
  {
    once = false;
    int numHSprites = 5;
    int numVSprites = 5;
    const smol::ConfigEntry* entry = root->config.findEntry("game");
    if (entry)
    {
      numHSprites = (int) entry->getVariableNumber("numHSprites", (float) numHSprites);
      numVSprites = (int) entry->getVariableNumber("numVSprites", (float) numVSprites);
    }

    smol::Rect viewport =
      root->renderer.getViewport();
    float spriteWidth = 30.0f / numHSprites;
    float spriteHeight = 30.0f / numVSprites;

    for (int x = 0; x < numHSprites; x++)
    {
      for (int y = 0; y < numVSprites; y++)
      {
        auto hNode = scene.createSpriteNode(batcher, 
            smol::Rect{0, 0, 800, 800},
            smol::Vector3{x * spriteWidth, y * spriteHeight, 0.3f },
            spriteWidth, spriteHeight,
            smol::Color(rand() % 256, rand() % 256, rand() % 256));

        scene.getNode(hNode).setLayer(smol::Layer::LAYER_1);
      }
    }
  }


  if (keyboard.getKeyDown(smol::KEYCODE_V) )
  {
    vpsize += 0.2f;
    if (vpsize > 1.0f) vpsize = 0.2f;
    scene.getNode(cameraNode).camera.setViewportRect(smol::Rectf(0.0f, 0.0f, vpsize, vpsize));
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F2)) {
    scene.getNode(cameraNode).camera
      .setClearOperation((smol::Camera::ClearOperation::DEPTH | smol::Camera::ClearOperation::COLOR));
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F3)) {
    scene.getNode(cameraNode).camera
      .setClearOperation((unsigned int)smol::Camera::ClearOperation::DEPTH);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F5)) { root->resourceManager.destroyShader(shader); }

  if (keyboard.getKeyDown(smol::KEYCODE_F7)) { root->resourceManager.destroyMaterial(checkersMaterial); }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE)) 
  { 
    smol::SceneNode& node = scene.getNode(selectedNode);
    node.setActive(!node.isActive());
  }

  if (keyboard.getKeyDown(smol::KEYCODE_TAB)) 
  { 
    if (selectedNode == node1)
      selectedNode = node2;
    else if (selectedNode == node2)
      selectedNode = cameraNode;
    else if (selectedNode == cameraNode)
      selectedNode = node1;
  }


  if (keyboard.getKeyDown(smol::KEYCODE_R)) 
  {
    smol::SceneNode& camera = scene.getNode(cameraNode);
    smol::Color color = camera.camera.getClearColor();
    color.r+=0.2f;
    if (color.r > 1.0f) color.r = 0.0f;
    camera.camera.setClearColor(color);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_G))
  {
    smol::SceneNode& camera = scene.getNode(cameraNode);
    smol::Color color = camera.camera.getClearColor();
    color.g+=0.2f;
    if (color.g > 1.0f) color.g = 0.0f;
    camera.camera.setClearColor(color);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_B))
  {
    smol::SceneNode& camera = scene.getNode(cameraNode);
    smol::Color color = camera.camera.getClearColor();
    color.b+=0.2f;
    if (color.b > 1.0f) color.b = 0.0f;
    camera.camera.setClearColor(color);
  }

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
      const float amount = 15.0f * deltaTime;

      const smol::Vector3& position = transform->getPosition();
      smol::Vector3 updatedPos(
          amount * xDirection + position.x,
          amount * yDirection + position.y,
          amount * zDirection + position.z);

      transform->setPosition(updatedPos.x, updatedPos.y, updatedPos.z);
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

  //// bounce sprite1 across the screen borders
  //transform = &scene.getNode(sprite1).transform;
  //smol::Vector3 position = transform->getPosition();
  //smol::Rect viewport = renderer.getViewport();

  //if (position.x + 250 > viewport.w || position.x < 0)
  //  spriteXDirection *= -1;

  //if (position.y + 100 > viewport.h || position.y < 0)
  //  spriteYDirection *= -1;

  //transform->setPosition(
  //    position.x + (100 * spriteXDirection) * deltaTime,
  //    position.y + (100 * spriteYDirection) * deltaTime,
  //    position.z);
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

