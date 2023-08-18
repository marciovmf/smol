#ifndef SMOL_EDITOR_H
#define SMOL_EDITOR_H

#include <smol/smol_gui.h>
#include <smol/smol_game.h>
#include <smol/smol_platform.h>

namespace smol
{
  struct Event;
  struct Window;
  struct Project;

  class Editor
  {
    enum Mode
    {
      MODE_EDIT     = 0,      // in edit mode
      MODE_PRERUN   = 1,      // Building the game module before running
      MODE_RUNNING  = 2       // Game module is built and loaded
    };

    private:
    GUI gui;
    Project* project;
    bool closeFlag;
    char reopenProjectFilePath[Platform::MAX_PATH_LEN];
    GameModule gameModule;
    Mode mode;

    void drawMainMenu(int windowWidth, int windowHeight);
    void drawProjectDialog(int screenWidth, int screenHeight);
    bool loadGameModule(const char* modulePath);
    bool unloadGameModule();
    void toggleMode();


    public:
    bool onEvent(const Event& event);
    void initialize(Window* window, Project& project);
    void update(float deltaTime, int windowWidth, int windowHeight);
    void terminate();
    bool getCloseFlag();
    void updateGame(float delta);
    const char* shouldReopenWithProject();
  };
}
#endif  // SMOL_EDITOR_H
