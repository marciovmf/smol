#include <smol/smol_input_manager.h>
#include <smol/smol_log.h>

namespace smol
{
  InputManager::InputManager() { } 

  InputManager& InputManager::get()
  {
    static InputManager instance;
    return instance;
  }

  void InputManager::update()
  {
    mouse.update();
    keyboard.update();
  }
}
