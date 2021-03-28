#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_log.h>

void onStart()
{
  smol::Log::info("Game started!");
}

void onUpdate(float deltaTime)
{
  if (smol::Keyboard::getKeyDown(smol::KEYCODE_J))
    smol::Log::info("Pressed this frame\n");
  else if (smol::Keyboard::getKeyUp(smol::KEYCODE_J))
    smol::Log::info("Released this frame\n");
  else if (smol::Keyboard::getKey(smol::KEYCODE_J))
    smol::Log::info("Holding...\n");
}

void onStop()
{
  smol::Log::info("Game Stopped!");
}

