#ifndef SMOL_EDITOR_H
#define SMOL_EDITOR_H
#include <smol/smol_gui.h>
namespace smol
{
  class Editor
  {
    GUI gui;
    public:
    void initialize();
    void render(int windowWidth, int windowHeight);
    void terminate();

  };
}
#endif  // SMOL_EDITOR_H
