project("Launcher")

if(WIN32)
  set(SMOL_WINDOWS_RESOURCE_FILE "smol_launcher_resource_win64.rc")
endif()

add_executable(slauncher smol_launcher.cpp "${SMOL_WINDOWS_RESOURCE_FILE}")
target_include_directories(slauncher INTERFACE "${SOURCE_PATH}/include")
target_include_directories(slauncher INTERFACE "${SOURCE_PATH}/include/smol")
target_link_libraries(slauncher PRIVATE smol ${OPENGL_LIBRARY})

# Make windows Debug builds to have a console subsystem
if(CMAKE_CONFIGURATION_TYPES)
  set_property(TARGET slauncher PROPERTY LINK_OPTIONS
    $<$<CXX_COMPILER_ID:MSVC>:$<IF:$<CONFIG:Release>,/SUBSYSTEM:WINDOWS,/SUBSYSTEM:CONSOLE>>)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release" )
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET slauncher PROPERTY LINK_OPTIONS /SUBSYSTEM:WINDOWS)
  endif()
endif()
