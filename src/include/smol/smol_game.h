#ifndef SMOL_GAME_H
#define SMOL_GAME_H

#include <smol/smol_engine.h>
#include <smol/smol_platform.h>

#ifdef SMOL_PLATFORM_WINDOWS
#ifdef SMOL_ENGINE_IMPLEMENTATION
#define SMOL_GAME_API __declspec(dllimport)
#else
#define SMOL_GAME_API __declspec(dllexport)
#endif //SMOL_ENGINE_IMPLEMENTATION
#else
#define SMOL_GAME_API
#endif // SMOL_PLATFORM_WINDOWS

#include <smol/smol_keyboard.h>
#include <smol/smol_scene.h>
#include <smol/smol_systems_root.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_material.h>

namespace smol
{
  struct SystemsRoot;

  const char* SMOL_CALLBACK_NAME_ONSTART = "onStart";
  const char* SMOL_CALLBACK_NAME_ONSTOP = "onStop";
  const char* SMOL_CALLBACK_NAME_ONUPDATE = "onUpdate";

  typedef void (*SMOL_GAME_CALLBACK_ONSTART)();
  typedef void (*SMOL_GAME_CALLBACK_ONSTOP)();
  typedef void (*SMOL_GAME_CALLBACK_ONUPDATE)(float);


  // Specialize Handle<SceneNode> so it's more convenient to call on game side
  inline Material* Handle<Material>::operator->()
  {
    static ResourceManager& resourceManager = smol::SystemsRoot::get()->resourceManager;
    Material& material = resourceManager.getMaterial((Handle<Material>)*this);
    return &material;
  }

  // Specialize Handle<SceneNode> so it's more convenient to call on game side
  inline SceneNode* Handle<SceneNode>::operator->()
  {
    static SceneManager& sceneManager = smol::SystemsRoot::get()->sceneManager;
    SceneNode& node = sceneManager.getLoadedScene().getNode((Handle<SceneNode>)*this);
    return &node;
  }


}

extern "C"
{
  SMOL_GAME_API void onStart();
  SMOL_GAME_API void onUpdate(float deltaTime);
  SMOL_GAME_API void onStop();
}

#endif// SMOL_GAME_H



