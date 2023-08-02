# This is a template for games built with the SMOL engine.
cmake_minimum_required(VERSION 3.10)
project("Launcher")

set(SOURCES
  game_launcher.cpp
  game_launcher_resource.h
  game_launcher_resource.rc)

add_executable(launcher EXCLUDE_FROM_ALL ${SOURCES})

# Retrieve the current configuration name
if(CMAKE_CONFIGURATION_TYPES)
  set_target_properties(launcher PROPERTIES 
    OUTPUT_NAME launcher$<$<NOT:$<CONFIG:Release>>:$<CONFIG>>
    WIN32_EXECUTABLE $<CONFIG:Release>
    )
else()
  set(LAUNCHER_NAME "launcher")
  if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(RELEASEBUILD TRUE)
  else()
    set(RELEASEBUILD FALSE)
    set(LAUNCHER_NAME "${LAUNCHER_NAME}${CMAKE_BUILD_TYPE}")
  endif()

  set_target_properties(launcher PROPERTIES 
    OUTPUT_NAME "${LAUNCHER_NAME}"
    WIN32_EXECUTABLE ${RELEASEBUILD})
endif()

target_compile_definitions(launcher PRIVATE SMOL_MODULE_LAUNCHER)
target_link_libraries(launcher PUBLIC "${ENGINE_PATH}/lib/smol.lib" PRIVATE game)

add_custom_command(TARGET launcher POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ENGINE_PATH}/bin/smol.dll ${RUN_TREE})

post_build_copy_engine_assets(launcher) 
