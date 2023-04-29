#include <smol/smol_game.h>
#include <smol/smol_renderer.h>
#include <smol/smol_point.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_font.h>
#include <utility>
#include <time.h>

float drawGliph(smol::Glyph& g, float x, float y, 
    smol::Kerning* kernings, uint16 kerningCount,
    smol::Handle<smol::SpriteBatcher> batcher, smol::Scene& scene, smol::Color color)
{
  const float scale = 0.01f;
  const float width   = g.rect.w * scale;
  const float height  = g.rect.h * scale;
  const float yOffset = g.yOffset * scale;

  // offset by half height because our pivots are at center
  const float xOffset = g.xOffset * scale + width/2;
  const float xAdvance = g.xAdvance * scale;
 // offset by half height because our pivots are at center
  y -= height/2.0f + yOffset;

  // apply kerning
  float kerning = 0.0f;
  for (int i = 0; i < kerningCount; i++)
  {
    smol::Kerning& k = kernings[i];
    if(k.second == g.id)
    {
      kerning = k.amount * scale;
      break;
    }
  }

  scene.createSpriteNode(batcher, g.rect,
      smol::Vector3(x + xOffset + kerning, y, 0.0f),
      width, height, color);

  return x + xAdvance + kerning;
}

void drawString(const char* str, smol::Font* font, float x, float y, smol::Handle<smol::SpriteBatcher> batcher, smol::Scene& scene, smol::Color color)
{
  float advance = x;
  smol::Kerning* kerning = nullptr;
  uint16 kerningCount = 0;

  while (*str != 0)
  {
    for (int i = 0; i < font->glyphCount; i++)
    {
      uint16 id = (uint16) *str;

      smol::Glyph& glyph = font->glyph[i];
      if (glyph.id == id)
      {
        if ((char)id == '\n')
        {
          y -= font->lineHeight / 100.0f;
          advance = x;
        }
        advance = drawGliph(glyph, advance, y, kerning, kerningCount, batcher, scene, color);
        // kerning information for the next character
        kerning = &font->kerning[glyph.kerningStart];
        kerningCount = glyph.kerningCount;
        break;
      }
    }
    str++;
  }
}

smol::SystemsRoot* root;
smol::Font* font;
smol::Handle<smol::SceneNode> cameraNode;
smol::Handle<smol::SceneNode> sideCamera;
smol::Handle<smol::SceneNode> floorNode;
smol::Handle<smol::SceneNode> node1;
smol::Handle<smol::SceneNode> node2;
smol::Handle<smol::SceneNode> sprite1;
smol::Handle<smol::SceneNode> selectedNode;
smol::Handle<smol::Texture> texture2;
smol::Handle<smol::ShaderProgram> shader;
smol::Handle<smol::Material> checkersMaterial;
smol::Handle<smol::Mesh> mesh;
smol::Handle<smol::SpriteBatcher> batcher;
smol::Handle<smol::SpriteBatcher> textBatcher;
bool sideCameraActive = false;
float vpsize = 0.2f;

int shape = 0;

void onStart()
{
  smol::Log::info("Game started!");
  root = smol::SystemsRoot::get();
  smol::ResourceManager& resourceManager = root->resourceManager;
  smol::Scene& scene = root->sceneManager.getLoadedScene();

  font = resourceManager.loadFont("assets/font/segoeui.font");

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
  cameraNode = scene.createPerspectiveCameraNode(60.0f, 0.01f, 3000.0f, t);

  cameraNode->transform
    .setRotation(0.0f, .0f, 0.0f)
    .setPosition(0.0f, 0.0f, 0.5f);
  cameraNode->camera.setLayerMask((uint32)(smol::Layer::LAYER_0 | smol::Layer::LAYER_1 | smol::Layer::LAYER_2));

  sideCamera = scene.createPerspectiveCameraNode(60.0f, 0.01f, 3000.0f, t);
  sideCamera->transform
    .setRotation(-30.0f, 0.0f, 0.0f)
    .setPosition(0.0f, 10.0f, 15.0f);
  sideCamera->camera.setViewportRect(smol::Rectf(0.75f, 0.0f, 0.25f, 0.25f));
  sideCamera->camera.setLayerMask((uint32)(smol::Layer::LAYER_0 | smol::Layer::LAYER_1));
  sideCamera->setActive(sideCameraActive);

#if 1
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
    t.setRotation(0.0f, 0.0f, 0.0f);
    t.setScale(randScale, randScale, randScale);

    auto handle = scene.createMeshNode( (isTallGrass) ? grassRenderable1 : grassRenderable2, t);
    handle->setLayer(smol::Layer::LAYER_2);
  }
#endif

  // sprites
  auto smolMaterial = resourceManager.loadMaterial("assets/default.material");
  batcher = scene.createSpriteBatcher(smolMaterial, smol::SpriteBatcher::SCREEN);

  auto fontMaterial = resourceManager.loadMaterial("assets/font.material");
  fontMaterial->setSampler2D("mainTex", font->texture);
  textBatcher = scene.createSpriteBatcher(fontMaterial, smol::SpriteBatcher::SCREEN);

  char* p = "Testing string drawing!\nThis is another text line :)";
  drawString(p, font, -5.0f, 0.0f, textBatcher, scene, smol::Color::BLUE);

  scene.createSpriteNode(textBatcher,
      //smol::Rect(378, 507, 409, 481), SMOL LOGO
      smol::Rect(339, 479, 32, 32), // FOLDER
      //smol::Rect(270, 476, 32, 32), // CUBE
      //smol::Rect(303, 476, 32, 32), // DOCUMENT
      smol::Vector3(0.0f, 3.0f, 0.3f),
      0.32f, 0.32f,
      smol::Color::YELLOW);

  selectedNode = cameraNode;
}

unsigned int angle = 0;
bool once = true;

int spriteXDirection = 1;
int spriteYDirection = 1;
smol::Vector3 direction;

inline float animateToZero(float value, float deltaTime)
{
  //value = value * 0.95f;
  value = value * (0.95f - deltaTime);
  if (fabs(value) <= 0.02f)
    value = 0.0f;
  return value;
}

bool isSideCameraActive = false;

void onUpdate(float deltaTime)
{
  smol::Keyboard& keyboard = root->keyboard;
  smol::Mouse& mouse = root->mouse;
  smol::Renderer& renderer = root->renderer;
  smol::ResourceManager& resourceManager = root->resourceManager;
  smol::Scene& scene = root->sceneManager.getLoadedScene();

  float scaleAmount = 0.0f;

  if (keyboard.getKeyDown(smol::KEYCODE_M))
  {
    scene.setSpriteBatcherMode(textBatcher, scene.getSpriteBatcherMode(textBatcher) ?
        smol::SpriteBatcher::SCREEN : smol::SpriteBatcher::CAMERA );
  }

  if (keyboard.getKeyDown(smol::KEYCODE_C))
  {
    sideCameraActive = !sideCameraActive;
    sideCamera->setActive(sideCameraActive);
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

    smol::Rect viewport = root->renderer.getViewport();

    // we assume the screen camera size is 5.0
    float screenSize = 5.0f * 0.7f; // we use a smaller portion to keep things centered
    float spriteWidth = (2 * screenSize) / numHSprites;
    float spriteHeight = (2 * screenSize) / numVSprites;

    float xPos = -screenSize;
    float yPos = -screenSize;

    for (int x = 0; x < numVSprites; x++)
    {
      for (int y = 0; y < numHSprites; y++)
      {
        auto hNode = scene.createSpriteNode(batcher, smol::Rect{0, 0, 800, 800},
            smol::Vector3{xPos + spriteWidth/2, yPos + spriteHeight/2, 0.3f },
            spriteWidth, spriteHeight,
            smol::Color(rand() % 256, rand() % 256, rand() % 256));

        xPos += spriteWidth;
        hNode->setLayer(smol::Layer::LAYER_1);
      }

      xPos = -screenSize;
      yPos += spriteHeight;
    }
  }


  if (keyboard.getKeyDown(smol::KEYCODE_V) )
  {
    vpsize += 0.2f;
    if (vpsize > 1.0f) vpsize = 0.2f;
    cameraNode->camera.setViewportRect(smol::Rectf(0.0f, 0.0f, vpsize, vpsize));
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F2)) {
    cameraNode->camera
      .setClearOperation((smol::Camera::ClearOperation::DEPTH | smol::Camera::ClearOperation::COLOR));
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F3)) {
    cameraNode->camera
      .setClearOperation((unsigned int)smol::Camera::ClearOperation::DEPTH);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_F5)) { root->resourceManager.destroyShader(shader); }

  if (keyboard.getKeyDown(smol::KEYCODE_F7)) { root->resourceManager.destroyMaterial(checkersMaterial); }

  if (keyboard.getKeyDown(smol::KEYCODE_SPACE)) 
  { 
    selectedNode->setActive(!selectedNode->isActive());
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
    smol::Color color = cameraNode->camera.getClearColor();
    color.r+=0.2f;
    if (color.r > 1.0f) color.r = 0.0f;
    cameraNode->camera.setClearColor(color);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_G))
  {
    smol::Color color = cameraNode->camera.getClearColor();
    color.g+=0.2f;
    if (color.g > 1.0f) color.g = 0.0f;
    cameraNode->camera.setClearColor(color);
  }

  if (keyboard.getKeyDown(smol::KEYCODE_B))
  {
    smol::Color color = cameraNode->camera.getClearColor();
    color.b+=0.2f;
    if (color.b > 1.0f) color.b = 0.0f;
    cameraNode->camera.setClearColor(color);
  }

  // left/right
  if (keyboard.getKey(smol::KEYCODE_A)) { direction.x += -0.1f; }
  else if (keyboard.getKey(smol::KEYCODE_D)) { direction.x += 0.1f; }
  else
  {
    direction.x = animateToZero(direction.x, deltaTime);
  }

  // up/down movement
  if (keyboard.getKey(smol::KEYCODE_S)) { direction.y += -0.1f; }
  else if (keyboard.getKey(smol::KEYCODE_W)) { direction.y += 0.1f; }
  else
  {
    direction.y = animateToZero(direction.y, deltaTime);
  }

  // back/forth movement
  if (keyboard.getKey(smol::KEYCODE_E)) { direction.z += -0.1f; }
  else if (keyboard.getKey(smol::KEYCODE_Q)) { direction.z += 0.1f; }
  else
  {
    direction.z = animateToZero(direction.z, deltaTime);
  }

  if (keyboard.getKey(smol::KEYCODE_J)) { scaleAmount = -1; }
  if (keyboard.getKey(smol::KEYCODE_K)) { scaleAmount = 1; }

  if (direction.x || direction.y || direction.z || scaleAmount)
  {
    smol::Transform* transform = &selectedNode->transform;
    if (transform)
    {
      const float amount = 15.0f * deltaTime;

      const smol::Vector3& position = transform->getPosition();
      smol::Vector3 updatedPos(
          amount * direction.x + position.x,
          amount * direction.y + position.y,
          amount * direction.z + position.z);

      transform->setPosition(updatedPos.x, updatedPos.y, updatedPos.z);
      const smol::Vector3& scale = transform->getScale();
      transform->setScale(
          amount * scaleAmount + scale.x,
          amount * scaleAmount + scale.y,
          amount * scaleAmount + scale.z);
    }
  }

  // Prevent the camera from going through the floor
  const smol::Vector3& floorPos = floorNode->transform.getPosition();
  smol::Vector3 cameraPos = cameraNode->transform.getPosition();
  if (cameraPos.y < (floorPos.y + 1.0f))
  {
    cameraPos.y = floorPos.y + 1.0f;
    cameraNode->transform.setPosition(cameraPos);
    direction.y = -direction.y/2.0f; // invert the direction with lower speed
  }

  smol::Transform* transform = &node1->transform;
  if (transform)
  {
    float a = transform->getRotation().y + 30 * deltaTime;
    transform->setRotation(0, a, 0);
  }
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

