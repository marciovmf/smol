#ifndef SMOL_GAME_H
#define SMOL_GAME_H

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

namespace smol
{
  const char* SMOL_CALLBACK_NAME_ONSTART = "onStart";
  const char* SMOL_CALLBACK_NAME_ONSTOP = "onStop";
  const char* SMOL_CALLBACK_NAME_ONUPDATE = "onUpdate";

  typedef void (*SMOL_GAME_CALLBACK_ONSTART)();
  typedef void (*SMOL_GAME_CALLBACK_ONSTOP)();
  typedef void (*SMOL_GAME_CALLBACK_ONUPDATE)(float);

}

extern "C"
{
  SMOL_GAME_API void onStart();
  SMOL_GAME_API void onUpdate(float deltaTime);
  SMOL_GAME_API void onStop();
}

#endif// SMOL_GAME_H



