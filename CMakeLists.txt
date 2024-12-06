# lint_cmake: -readability/wonkycase
cmake_minimum_required(VERSION 3.14)
project(PureDataGD)

include(FetchContent)

FetchContent_Declare(
  godot-cpp
  GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
  GIT_TAG 4.3  # Use the appropriate branch or tag matching your Godot version
  GIT_SUBMODULES ""  # Ensure submodules are fetched
)

FetchContent_MakeAvailable(godot-cpp)

# Set options for godot-cpp
set(BINDINGS_GENERATOR_EXECUTABLE OFF CACHE INTERNAL "")
set(GODOT_CPP_CUSTOM_API_FILE "" CACHE STRING "")

# Specify the build type if necessary
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)

add_library(PureDataGD SHARED src/puredatagd.cpp src/register_types.cpp)

target_include_directories(PureDataGD PRIVATE
  ${godot-cpp_SOURCE_DIR}/include
  ${godot-cpp_BINARY_DIR}/include
)

target_link_libraries(PureDataGD PRIVATE godot-cpp)

target_compile_definitions(PureDataGD PRIVATE GODOT_CPP_EXPORT)
target_compile_features(PureDataGD PRIVATE cxx_std_17)

add_dependencies(PureDataGD godot-cpp)
