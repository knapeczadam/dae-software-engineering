project(game_engine)


add_executable(${PROJECT_NAME}
    src/AbstractGame.cpp
    src/Game.cpp
    src/GameEngine.cpp
    src/GameWinMain.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC inc)

add_definitions(-DUNICODE -D_UNICODE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/$<CONFIG>)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")
