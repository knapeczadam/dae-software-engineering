#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.30)                       # Minimum CMake version required for this project
#---------------------------------------------------------------------------------------------------

project(se)                                                # Declare the project name

#---------------------------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)                                 # Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD_REQUIRED ON)                        # Set the C++ standard as required

#---------------------------------------------------------------------------------------------------

include(FetchContent)                                      # Include the FetchContent module, which allows for downloading and using external dependencies

FetchContent_Declare(                                      # FetchContent for the Lua library
        lua
        GIT_REPOSITORY "https://github.com/marovira/lua"   # The repository URL
        GIT_TAG "5.4.7"                                    # Specific version/tag of Lua to fetch
        GIT_SHALLOW TRUE                                   # Shallow copy of the repository
)
FetchContent_MakeAvailable(lua)                            # Make the Lua library available to the project

FetchContent_Declare(                                      # FetchContent for the Sol2 library
        sol2
        GIT_REPOSITORY "https://github.com/ThePhD/sol2"    # The repository URL
        GIT_TAG "v3.3.1"                                   # Specific version/tag of Sol2 to fetch
        GIT_SHALLOW TRUE                                   # Shallow copy of the repository
)
FetchContent_MakeAvailable(sol2)                           # Make the Sol2 library available to the project

#---------------------------------------------------------------------------------------------------
                                                           # Add the 'project' directory as a subdirectory
add_subdirectory(project)                                  # This assumes there is a CMakeLists.txt in the 'project' directory for further configurations.

#---------------------------------------------------------------------------------------------------
