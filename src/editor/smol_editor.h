#ifndef SMOL_EDITOR_H
#define SMOL_EDITOR_H
#include <smol/smol_gui.h>

namespace smol
{
  struct Event;
  struct Window;
  struct Project;

  class Editor
  {
    private:
    GUI gui;
    Project* project;

    public:
    bool onEvent(const Event& event);
    void initialize(Window* window, Project& project);
    void render(int windowWidth, int windowHeight);
    void terminate();

  };
}
#endif  // SMOL_EDITOR_H
