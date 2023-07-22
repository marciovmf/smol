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
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_mesh_data.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_material.h>
#include <smol/smol_renderer.h>
#include <smol/smol_point.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_scene_manager.h>
#include <smol/smol_input_manager.h>
#include <smol/smol_config_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_font.h>
#include <smol/smol_sprite_node.h>
#include <smol/smol_camera_node.h>
#include <smol/smol_gui.h>

#define SMOL_CALLBACK_NAME_ONSTART  "onStart"
#define SMOL_CALLBACK_NAME_ONSTOP   "onStop"
#define SMOL_CALLBACK_NAME_ONUPDATE "onUpdate"
#define SMOL_CALLBACK_NAME_ONGUI    "onGUI"

namespace smol
{
  struct SystemsRoot;
  struct Module;
  class GUI;

  typedef void (*SMOL_GAME_CALLBACK_ONSTART)();
  typedef void (*SMOL_GAME_CALLBACK_ONSTOP)();
  typedef void (*SMOL_GAME_CALLBACK_ONUPDATE)(float);
  typedef void (*SMOL_GAME_CALLBACK_ONGUI)(GUI&);

  struct GameModule
  {
    Module* module;
    SMOL_GAME_CALLBACK_ONSTART  onStart;
    SMOL_GAME_CALLBACK_ONSTOP   onStop;
    SMOL_GAME_CALLBACK_ONUPDATE onUpdate;
    SMOL_GAME_CALLBACK_ONGUI    onGUI;
  };
}

extern "C"
{
  SMOL_GAME_API void onStart();
  SMOL_GAME_API void onStop();
  SMOL_GAME_API void onGUI(smol::GUI&);
  SMOL_GAME_API void onUpdate(float deltaTime);
}

#endif// SMOL_GAME_H
