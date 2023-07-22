project("editor")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND SRC
    smol_editor_launcher.cpp 
    smol_editor_common.h
    smol_editor.h
    smol_editor.cpp
  )

LIST(APPEND LIBS smol ${OPENGL_LIBRARY})
if(WIN32)
  list(APPEND SRC
    resource.h
    resource.rc)

  list(APPEND LIBS kernel32.lib)
endif()

add_executable(editor ${SRC})

# Retrieve the current configuration name
if(CMAKE_CONFIGURATION_TYPES)
  set_target_properties(editor PROPERTIES 
    OUTPUT_NAME smol$<$<NOT:$<CONFIG:Release>>:$<CONFIG>>
    WIN32_EXECUTABLE $<CONFIG:Release>
    )
else()
  set(LAUNCHER_NAME "smol")
  if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(RELEASEBUILD TRUE)
  else()
    set(RELEASEBUILD FALSE)
    set(LAUNCHER_NAME "${LAUNCHER_NAME}${CMAKE_BUILD_TYPE}")
  endif()

  set_target_properties(editor PROPERTIES 
    OUTPUT_NAME "${LAUNCHER_NAME}"
    WIN32_EXECUTABLE ${RELEASEBUILD})
endif()


target_include_directories(editor INTERFACE "${SOURCE_PATH}/include")
target_include_directories(editor INTERFACE "${SOURCE_PATH}/include/smol")
target_compile_definitions(editor PRIVATE SMOL_MODULE_LAUNCHER)
target_link_libraries(editor PRIVATE ${LIBS})
