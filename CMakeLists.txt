cmake_minimum_required (VERSION 3.18 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Change Project details here
set(project_name DRPC)
set(project_version "1.0.0")

project(${project_name} VERSION "${project_version}")

# Fetch external dependencies here
include(FetchContent)

FetchContent_Declare (
    curl 
    GIT_REPOSITORY https://github.com/curl/curl.git
)

FetchContent_MakeAvailable(curl)

add_executable(${PROJECT_NAME})
add_subdirectory(src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type)
target_link_libraries(${PROJECT_NAME} libcurl)
