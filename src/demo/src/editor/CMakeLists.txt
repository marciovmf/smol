cmake_minimum_required(VERSION 3.10)
project("editor_module")

add_library(editor_module SHARED)
target_link_libraries(editor_module PUBLIC "${ENGINE_PATH}/lib/smol.lib" PRIVATE game)
set_target_properties(editor_module PROPERTIES OUTPUT_NAME game)
post_build_copy_engine_assets(editor_module) 
