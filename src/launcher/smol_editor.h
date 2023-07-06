#ifndef SMOL_EDITOR_H
#define SMOL_EDITOR_H
#include <smol/smol_gui.h>
namespace smol
{
  struct Event;
  struct Window;

  class Editor
  {
    GUI gui;
    private:

    public:
    bool onEvent(const Event& event);
    void initialize(Window* window);
    void render(int windowWidth, int windowHeight);
    void terminate();

  };
}
#endif  // SMOL_EDITOR_H
