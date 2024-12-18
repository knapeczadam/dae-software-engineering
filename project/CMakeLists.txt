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
    src/GameWinMain.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME} PRIVATE lua::lua sol2)

add_definitions(-DUNICODE -D_UNICODE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/${CMAKE_SYSTEM_NAME}/${SYSTEM_ARCHITECTURE}/$<CONFIG>)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")
