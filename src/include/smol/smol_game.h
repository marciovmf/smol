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
  class GUI;

  const char* SMOL_CALLBACK_NAME_ONSTART = "onStart";
  const char* SMOL_CALLBACK_NAME_ONSTOP = "onStop";
  const char* SMOL_CALLBACK_NAME_ONUPDATE = "onUpdate";
  const char* SMOL_CALLBACK_NAME_ONGUI = "onGUI";

  typedef void (*SMOL_GAME_CALLBACK_ONSTART)();
  typedef void (*SMOL_GAME_CALLBACK_ONSTOP)();
  typedef void (*SMOL_GAME_CALLBACK_ONUPDATE)(float);
  typedef void (*SMOL_GAME_CALLBACK_ONGUI)(smol::GUI&);
}

extern "C"
{
  SMOL_GAME_API void onStart();
  SMOL_GAME_API void onUpdate(float deltaTime);
  SMOL_GAME_API void onStop();
  SMOL_GAME_API void onGUI(smol::GUI&);
}

#endif// SMOL_GAME_H



