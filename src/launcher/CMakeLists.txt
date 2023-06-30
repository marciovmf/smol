project("Launcher")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND SRC
    smol_launcher.cpp 
    smol_editor.h
    smol_editor.cpp
  )

if(WIN32)
  list(APPEND SRC smol_launcher_resource_win64.rc)
endif()

add_executable(slauncher ${SRC})

if (NOT SMOL_PROJECT_ROOT)
  message(FATAL_ERROR "SMOL_PROJECT_ROOT is not set.")
endif()

target_include_directories(slauncher INTERFACE "${SMOL_PROJECT_ROOT}/src/include")
target_include_directories(slauncher INTERFACE "${SMOL_PROJECT_ROOT}/src/include/smol")
target_link_libraries(slauncher PRIVATE smol ${OPENGL_LIBRARY})
target_compile_definitions(slauncher PRIVATE SMOL_MODULE_LAUNCHER)

# Make windows Debug builds to have a console subsystem
if(CMAKE_CONFIGURATION_TYPES)
  set_property(TARGET slauncher PROPERTY LINK_OPTIONS
    $<$<CXX_COMPILER_ID:MSVC>:$<IF:$<CONFIG:Release>,/SUBSYSTEM:WINDOWS,/SUBSYSTEM:CONSOLE>>)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release" )
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET slauncher PROPERTY LINK_OPTIONS /SUBSYSTEM:WINDOWS)
  endif()
endif()
