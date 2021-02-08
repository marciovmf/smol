#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_keyboard.h>

void onStart()
{
  LogInfo("Game started!");
}

void onUpdate(float deltaTime)
{
  if (smol::Keyboard::getKeyDown(smol::KEYCODE_J))
    LogInfo("Pressed this frame");
  else if (smol::Keyboard::getKeyUp(smol::KEYCODE_J))
    LogInfo("Released this frame");
  else if (smol::Keyboard::getKey(smol::KEYCODE_J))
    LogInfo("Holding...");
}

void onStop()
{
  LogInfo("Game Stopped!");
}

