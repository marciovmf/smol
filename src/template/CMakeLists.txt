cmake_minimum_required(VERSION 3.13)
project(template)
set(CMAKE_CXX_STANDARD 14)

# Generates a package with the template proejct source code and place it in the
# data folder

set(PACKAGE_NAME "../../../data/template_project.package")
add_custom_target(template 
  COMMAND "${OUTPUT_DIR}/bin/packer.exe" ${PACKAGE_NAME}
  CMakeLists.txt runtree/.smolfolder
  src/CMakeLists.txt src/game.cpp
  src/editor/CMakeLists.txt src/launcher/CMakeLists.txt
  src/launcher/game_icon.ico src/launcher/game_launcher.cpp
  src/launcher/game_launcher_resource.h src/launcher/game_launcher_resource.rc
  workspace/.smolfolder
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/project
  COMMENT "Packing template project ${PACKAGE_NAME}..."
  ALL)
