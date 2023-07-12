cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "game")

# Your game sources goes here...
set(SOURCES game.cpp) 

add_library(game OBJECT ${SOURCES})
target_include_directories(game PUBLIC
  "${ENGINE_PATH}/include" 
  "${ENGINE_PATH}/include/smol" )
