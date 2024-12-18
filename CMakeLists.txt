cmake_minimum_required(VERSION 3.30)
project(se)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(project)

include(FetchContent)

FetchContent_Declare(
        lua
        GIT_REPOSITORY "https://github.com/marovira/lua"
        GIT_TAG "5.4.7"
)
FetchContent_MakeAvailable(lua)

FetchContent_Declare(
        sol2
        GIT_REPOSITORY "https://github.com/ThePhD/sol2"
        GIT_TAG "v3.3.1"
)
FetchContent_MakeAvailable(sol2)