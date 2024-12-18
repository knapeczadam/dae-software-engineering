project(game_engine)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(SYSTEM_ARCHITECTURE x64)
else ()
    set(SYSTEM_ARCHITECTURE x86)
endif ()

add_executable(${PROJECT_NAME}
    src/AbstractGame.cpp
    src/Game.cpp
    src/GameEngine.cpp
    src/GameWinMain.cpp
    lua/game_breakout.lua
)

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME} PRIVATE sol2 lua::lua)

add_definitions(-DUNICODE -D_UNICODE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/${CMAKE_SYSTEM_NAME}/${SYSTEM_ARCHITECTURE}/$<CONFIG>)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")

add_custom_target(CopyScripts ALL
    COMMENT "Copying Lua scripts to output directory"
)

add_custom_command(TARGET CopyScripts POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/game_breakout.lua
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/game_breakout.lua)

add_dependencies(${PROJECT_NAME} CopyScripts)