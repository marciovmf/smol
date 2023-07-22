#ifndef SMOL_PROJECT_MANAGER_H
#define SMOL_PROJECT_MANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_platform.h>
#include <xerrc.h>

namespace smol
{
  struct SMOL_ENGINE_API Project
  {
    enum
    {
      PROJECT_MAX_NAME_LEN = 255,
    };

    enum CMakeGenerator
    {
      VISUAL_STUDIO2019 = 0,
      VISUAL_STUDIO2017 = 1,
      VISUAL_STUDIO2015 = 2,
      UNIX_MAKEFILES    = 3,
      NMAKE_MAKEFILES   = 4,
      NINJA             = 5,
      GENERATOR_COUNT
    };

    enum State
    {
      INVALID,
      CREATED,
      GENERATING,
      GENERATED,
      BUILDING,
      FAIL_GENERATE,
      FAIL_BUILD,
      READY
    };

    bool valid;
    char name[PROJECT_MAX_NAME_LEN];
    char path[Platform::MAX_PATH_LEN];
    const char* modulePathRelative;
    CMakeGenerator generator;
    State state;
    void* externalCommandPipe;
    char cmdOutputBuffer[128];
  };

  struct SMOL_ENGINE_API ProjectManager
  {
    static bool createProject(const char* projectFilePath, const char* projectName, Project::CMakeGenerator generator);
    static bool loadProject(const char* projectFilePath, Project& project);
    static bool unloadProject(Project& project);
    static bool generateProject(Project& project);
    static const char* getGeneratorName(Project::CMakeGenerator generator);
    static bool buildProjectModule(Project &project);
    static bool waitForExternalCommand(Project& project, int* exitCode);
  };
}
#endif //SMOL_PROJECT_MANAGER_H

