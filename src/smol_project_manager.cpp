#include <smol/smol_project_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_platform.h>
#include <smol/smol_log.h>
#include <string.h>

#ifndef SMOL_GAME_MODULE_NAME
#ifdef SMOL_PLATFORM_WINDOWS
#define SMOL_GAME_MODULE_NAME "game.dll"
#else
#define SMOL_GAME_MODULE_NAME "runtree/game.so"
#endif
#endif

namespace smol
{
  bool ProjectManager::createProject(Project& project)
  {
    // Create output folder if id does not exist
    // copy template files
    // HOW TO PATHC the files with correct proejct name and other project specific info ?
    // copy engine assets
    // generate solution
    return false;
  }

  bool ProjectManager::loadFromProjectFile(const char* projectFilePath, Project& project)
  {
    const char* errorMsgTitle = "Error loading project.";

    project.valid = false;
    const size_t pathLen = strlen(projectFilePath);
    SMOL_ASSERT(pathLen < Platform::MAX_PATH_LEN, "Path to project exceeds maximum project path. Maximum path is %d",
        Platform::MAX_PATH_LEN);

    Config* config = new Config(projectFilePath);
    if (!config)
    {
      Platform::messageBoxError(errorMsgTitle, "Invalid project file.");
      return false;
    }

    const ConfigEntry* entry = config->findEntry("project");
    if (!entry)
    {
      Platform::messageBoxError(errorMsgTitle, "Invalid project file: project entry not found.");
      return false;
    }

    int versionMajor = (int) entry->getVariableNumber("engine_version_major");
    int versionMinor = (int) entry->getVariableNumber("engine_version_minor");
    const char* projectName = entry->getVariableString("name");

    size_t projectNameLen = strlen(projectName);
    SMOL_ASSERT(projectNameLen < Project::PROJECT_MAX_NAME_LEN, "Project name too long. Maximum name length is %d", Project::PROJECT_MAX_NAME_LEN);
    SMOL_ASSERT(projectNameLen > 0, "Project name is empty.");
    strncpy(project.name, projectName, Project::PROJECT_MAX_NAME_LEN);

    delete config;

    if (versionMajor != SMOL_VERSION_MAJOR || versionMinor != SMOL_VERSION_MINOR)
    {
      const size_t maxErrorMesageLen = 128;
      char errorMessage[maxErrorMesageLen];
      snprintf(errorMessage, maxErrorMesageLen, "Engine version mismatch. Project requries v%d.%d but engine is v%d.%d.",
          versionMajor, versionMinor, SMOL_VERSION_MAJOR, SMOL_VERSION_MINOR);
      Platform::messageBoxError(errorMsgTitle, errorMessage);
      return false;
    }

    // get the base path
    strncpy(project.path, projectFilePath, pathLen);
    char* ptr = project.path + pathLen - 1;
    while (ptr > project.path && *ptr != '\\' && *ptr != '/')
    {
      *ptr = 0;
      ptr--;
    }
    size_t basePathLen = strlen(project.path);
    const char* runtree = (const char*) "runtree";
    size_t runTreeLen = strlen(runtree);
    char* basePathEnd = project.path + basePathLen;
    strncpy(basePathEnd, (const char*) runtree, runTreeLen);
    basePathEnd[runTreeLen] = 0;


    project.modulePathRelative = SMOL_GAME_MODULE_NAME;
    project.valid = true;
    return true;
  }

  bool ProjectManager::unloadProject(Project& project)
  {
    return false;
  }
}
