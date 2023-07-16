#include <smol/smol_project_manager.h>
#include <smol/smol_package.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_platform.h>
#include <smol/smol_log.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "editor/smol_editor_common.h"
#include "include/smol/smol_project_manager.h"

#ifndef SMOL_GAME_MODULE_NAME
#ifdef SMOL_PLATFORM_WINDOWS
#define SMOL_GAME_MODULE_NAME "game.dll"
#else
#define SMOL_GAME_MODULE_NAME "runtree/game.so"
#endif
#endif


#ifdef SMOL_PLATFORM_WINDOWS
#define popen _popen
#define pclose _pclose
#endif

namespace smol
{
  bool ProjectManager::createProject(const char* projectFilePath, const char* projectName, Project::CMakeGenerator generator)
  {
    const char* msgErrorTitle = "Error creating new project";
    char buffer[Platform::MAX_PATH_LEN];
    char templatePackageFile[Platform::MAX_PATH_LEN];
    char settingsFile[Platform::MAX_PATH_LEN];
    char settingsFileNew[Platform::MAX_PATH_LEN];

    // get the project base path
    char* projectFolder = buffer;
    size_t pathLen = strlen(projectFilePath);
    strncpy(projectFolder, projectFilePath, pathLen);
    char* ptr = projectFolder + pathLen - 1;
    while (ptr > projectFolder && *ptr != '\\' && *ptr != '/')
    {
      *ptr = 0;
      ptr--;
    }

    //
    // Template package file
    //

    // Get the full path of the template package. it's relative to the editor binary path.
    int32 result = snprintf(templatePackageFile, Platform::MAX_PATH_LEN, "%s%ctemplate_project.package", Platform::getBinaryPath(), Platform::pathSeparator());
    if (result < 0 || result > Platform::MAX_PATH_LEN)
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Engine path is too long.");
      return false;
    }

    // make sure the template package exists
    if (!Platform::pathExists(templatePackageFile))
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Missing template_project.package.");
      return false;
    }

    // extract template project package
    Packer::extractPackage(templatePackageFile, projectFolder);

    //
    // Settings file
    //

    //Get the full path of the settings file. it's relative to the editor binary path.
    result = snprintf(settingsFile, Platform::MAX_PATH_LEN, "%s%c%s", Platform::getBinaryPath(), Platform::pathSeparator(), SMOL_VARIABLES_FILE);
    if (result < 0 || result > Platform::MAX_PATH_LEN)
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Engine path is too long.");
      return false;
    }

    // make sure the settings file exists
    if (!Platform::pathExists(settingsFile))
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Missing settings file!");
      return false;
    }

    result = snprintf(settingsFileNew, Platform::MAX_PATH_LEN, "%s%cruntree%c%s", projectFolder, Platform::pathSeparator(), Platform::pathSeparator(), SMOL_VARIABLES_FILE);
    if (result < 0 || result > Platform::MAX_PATH_LEN)
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Project path is too long.");
      return false;
    }

    if (!Platform::copyFile(settingsFile, settingsFileNew, false))
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Unable to copy settings file to the project folder.");
      return false;
    }


    //
    // Proejct file
    //

    // write the project file
    const char* projectFileContent = "@project: name: \"%s\", engine_version_major: %d, engine_version_minor: %d, generator: %d";
    snprintf(buffer, Platform::MAX_PATH_LEN, projectFileContent, projectName,
        SMOL_VERSION_MAJOR, SMOL_VERSION_MINOR, (int32) generator);
    FILE* fd = fopen(projectFilePath, "wb");

    if (!fd)
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Unable to create project file");
      return false;
    }

    if (fwrite(buffer, strlen(buffer), 1, fd) != 1)
    {
      Platform::messageBoxError(msgErrorTitle, (const char*) "Unable to write to project file");
      return false;
    }

    fflush(fd);
    fclose(fd);
    return true;
  }

  bool ProjectManager::loadProject(const char* projectFilePath, Project& project)
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
    project.generator = (Project::CMakeGenerator) entry->getVariableNumber("generator");
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

    // Change directory to the project path
    Platform::setWorkingDirectory(project.path);
    // Generates the project if it wasn't already
    if (! Platform::fileExists("../workspace/CMakeCache.txt"))
    {
      project.state = Project::CREATED;
      project.cmdOutputBuffer[0] = 0;
      return ProjectManager::generateProject(project);
    }
        //return false;

    //TODO(marcio): We should NOT do it here. We should instead do from a "Run"
    //option in the editor. There must be a way for the editor to run without a
    //valid project built so it remains open skipping the game callbacks or
    //calling dummy callbacks untill we RUN the game. At that point we try to
    //build it and if successfull, we reload the callback pointers
    //return ProjectManager::buildProjectModule(project);
    return true;
  }

  // This function assumes the project directory is the current directory.
  // Ideally there is no reason to generate a project that is not loaded.
  bool ProjectManager::generateProject(Project &project)
  {
    if (!project.valid)
      return false;

    const char* generatorName = ProjectManager::getGeneratorName(project.generator);
    char cmdLine[Platform::MAX_PATH_LEN];
    const char separator = Platform::pathSeparator();
    // For the editor, the project working directory is runtree. So we pass relative paths to cmake
    snprintf(cmdLine, Platform::MAX_PATH_LEN, "%s%ccmake%cbin%ccmake.exe -S .. -B ..%cworkspace -G \"%s\" -DENGINE_PATH=%s%c..",
        Platform::getBinaryPath(),
        separator,
        separator,
        separator,
        separator,
        generatorName,
        Platform::getBinaryPath(),
        separator);
    Log::info("Generating project solution...:\n\t%s\n", cmdLine);

    FILE* pipe = popen(cmdLine, "r");
    if (!pipe) {
      return false;
    }

    project.externalCommandPipe = (void*) pipe;
    project.state = Project::GENERATING;
    return true;
  }

  bool ProjectManager::buildProjectModule(Project &project)
  {
    if (!project.valid)
      return false;

    char cmdLine[Platform::MAX_PATH_LEN];
    const char separator = Platform::pathSeparator();
    // For the editor, the project working directory is runtree. So we pass relative paths to cmake
    snprintf(cmdLine, Platform::MAX_PATH_LEN, "%s%ccmake%cbin%ccmake.exe --build ..%cworkspace",
        Platform::getBinaryPath(),
        separator,
        separator,
        separator,
        separator);
    Log::info("Building project ...:\n\t%s\n", cmdLine);

    FILE* pipe = popen(cmdLine, "r");
    if (!pipe) {
      return false;
    }

    project.externalCommandPipe = (void*) pipe;
    project.state = Project::BUILDING;
    return true;
  }

  //returns true when the command is finished
  bool ProjectManager::waitForExternalCommand(Project& project, int* exitCode)
  {
    if (!project.valid)
    {
      *exitCode = -1;
      return true;
    }

    FILE* pipe = (FILE*) project.externalCommandPipe;
    project.externalCommandPipe = pipe;
    if (fgets(project.cmdOutputBuffer, sizeof(project.cmdOutputBuffer), pipe) != NULL)
    {
      printf("%s", project.cmdOutputBuffer);  // Print the output
    }

    if (feof(pipe))
    {
      int32 result = pclose(pipe);

      if (result == 0)
      {
        if (project.state == Project::GENERATING)
          project.state = Project::GENERATED;
        else if (project.state == Project::BUILDING)
          project.state = Project::READY;
      }
      else
      {
        if (project.state == Project::GENERATING)
          project.state = Project::FAIL_GENERATE;
        else if (project.state == Project::BUILDING)
          project.state = Project::FAIL_BUILD;
      }

      *exitCode = result;
      project.externalCommandPipe = nullptr;
      return true;
    }
    return false;
  }


  bool ProjectManager::unloadProject(Project& project)
  {
    return false;
  }

  const char* ProjectManager::getGeneratorName(Project::CMakeGenerator generator)
  {
    static const char* generatorNames[] = {
      (const char*) "Visual Studio 16 2019",
      (const char*) "Visual Studio 15 2017",
      (const char*) "Visual Studio 14 2015",
      (const char*) "Unix Makefiles",
      (const char*) "NMake Makefiles",
      (const char*) "Ninja",
      // Unknown
      (const char*) "UNKNOWN"
    };

    const char* name;

    switch (generator)
    {
      case Project::VISUAL_STUDIO2019:
      case Project::VISUAL_STUDIO2017:
      case Project::VISUAL_STUDIO2015:
      case Project::UNIX_MAKEFILES:
      case Project::NMAKE_MAKEFILES:
      case Project::NINJA:
        name = generatorNames[(int32)generator];
        break;
      default:
        name = generatorNames[(int32)Project::GENERATOR_COUNT];
        break;
    }
    return name;
  }
}
