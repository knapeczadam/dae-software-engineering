#---------------------------------------------------------------------------------------------------

project(game_engine)                                                 # Declare the project name

#---------------------------------------------------------------------------------------------------
                                                                     # Determine the system architecture (x86 or x64)
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)                                  # Check if the system pointer size is 8 bytes (64-bit)
    set(SYSTEM_ARCHITECTURE x64)                                     # Set architecture to x64 for 64-bit systems
else ()
    set(SYSTEM_ARCHITECTURE x86)                                     # Set architecture to x86 for 32-bit systems
endif ()

#---------------------------------------------------------------------------------------------------

add_executable(${PROJECT_NAME}                                       # Define the executable target with its source files
        src/AbstractGame.cpp                                         # Core abstract game functionality
        src/Game.cpp                                                 # Implementation of the game logic
        src/GameEngine.cpp                                           # Game engine implementation
        src/GameWinMain.cpp                                          # Windows-specific entry point
        lua/game_breakout.lua                                        # Lua script included in the executable
)

#---------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PUBLIC inc)               # Include header files located in the 'inc' directory

target_link_libraries(${PROJECT_NAME}                                # Link external libraries to the project
        PRIVATE                                                      # Set the linkage to private
        sol2                                                         # Sol2: For Lua-C++ bindings
        lua::lua                                                     # Lua: The Lua interpreter library
)

#---------------------------------------------------------------------------------------------------
                                                                     # Define preprocessor macros
add_definitions(-DUNICODE -D_UNICODE)                                # UNICODE: Enforces the use of wide character strings in Windows


                                                                     # Set the output directory for the compiled executable
                                                                     # Creates a directory structure based on OS, architecture, and build type (e.g., Debug/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build/${CMAKE_SYSTEM_NAME}/${SYSTEM_ARCHITECTURE}/$<CONFIG>)

set_target_properties(${PROJECT_NAME}                                # Configure target properties
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}   # Use the custom output directory
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"                              # Set the Windows subsystem to 'Windows' for GUI applications
                                                                     # Alternatively, use WIN32 in add_executable() to set the subsystem
)

#---------------------------------------------------------------------------------------------------

add_custom_target(CopyScripts ALL                                    # Custom target for copying Lua scripts to the output directory
        COMMENT "Copying Lua scripts to output directory"
)

add_custom_command(TARGET CopyScripts POST_BUILD                     # Custom command to copy the Lua script to the runtime output directory after building
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/lua/game_breakout.lua            # Source Lua script
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/game_breakout.lua          # Destination in the output directory
)

add_dependencies(${PROJECT_NAME} CopyScripts)                        # Ensure the Lua script copying happens after the main executable is built

#---------------------------------------------------------------------------------------------------
