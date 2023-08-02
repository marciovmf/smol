#ifndef SMOL_INPUT_MANAGER_H
#define SMOL_INPUT_MANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_mouse.h>

namespace smol
{
 class SMOL_ENGINE_API InputManager final
 {
   InputManager();

   public:
   Keyboard keyboard; 
   Mouse mouse; 
   static InputManager& get();
   void update();

   InputManager(const InputManager& other) = delete;
   InputManager(const InputManager&& other) = delete;
   void operator=(const InputManager& other) = delete;
   void operator=(const InputManager&& other) = delete;
 };
}
#endif //SMOL_INPUT_MANAGER_H

