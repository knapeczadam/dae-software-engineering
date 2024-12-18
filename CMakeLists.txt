cmake_minimum_required(VERSION 3.30)
project(se)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(se
    AbstractGame.cpp
    AbstractGame.h
    Game.cpp
    Game.h
    GameDefines.h
    GameEngine.cpp
    GameEngine.h
    GameWinMain.cpp
    GameWinMain.h
    resource.h)

add_definitions(-DUNICODE -D_UNICODE)

set_target_properties(se PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")