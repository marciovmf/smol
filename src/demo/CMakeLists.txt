cmake_minimum_required(VERSION 3.10)
project("smol_game")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Make sure we have a valid engine location
if (NOT DEFINED ENGINE_PATH)
  message(FATAL_ERROR "ENGINE_PATH not defined. Set it to the location of SMOL engine.")
elseif (NOT EXISTS "${ENGINE_PATH}")
  message(FATAL_ERROR "Invalid ENGINE_PATH location'${ENGINE_PATH}' does not exist")
endif()

message(STATUS "SMOL Engine location: ${ENGINE_PATH}")

# Make sure to generate the binaries where the engine expects them to be
set(RUN_TREE ${CMAKE_CURRENT_SOURCE_DIR}/runtree)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${RUN_TREE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${RUN_TREE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUN_TREE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${RUN_TREE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${RUN_TREE})

function(post_build_copy_engine_assets target)
  set(source_dir "${ENGINE_PATH}/bin/assets")
  set(destination_dir "${RUN_TREE}/assets")
  add_custom_command(
    TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${source_dir}
    ${destination_dir}
    COMMENT "Copying engine standard assets..."
    COMMAND_EXPAND_LISTS VERBATIM
    CONDITION NOT EXISTS "${RUN_TREE}/assets")
endfunction()

# Add targets
add_subdirectory(src)
add_subdirectory(src/editor)
add_subdirectory(src/launcher)
