# lint_cmake: -readability/wonkycase
cmake_minimum_required(VERSION 3.14)
project(PureDataGD)

# ====== Fetch and set up dependencies ======
# Include FetchContent module
include(FetchContent)

# > ====== Godot ======
# Declare and fetch godot-cpp
FetchContent_Declare(
  godot-cpp
  GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
  GIT_TAG 4.3  # Use the appropriate branch or tag matching your Godot version
  GIT_SUBMODULES ""  # Ensure submodules are fetched
)

# Fetch and make godot-cpp available
FetchContent_MakeAvailable(godot-cpp)

# > ====== libpd ======
FetchContent_Declare(
  libpd
  GIT_REPOSITORY https://github.com/libpd/libpd.git
  GIT_TAG master # Use an appropriate tag or branch
)

FetchContent_MakeAvailable(libpd)

# ====== Setup build ======
# Specify the build type if necessary
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)

# List all source files
set(SOURCES
  src/puredatagd.cpp
  src/register_types.cpp
)

# Add PureDataGD library and specify its sources
add_library(PureDataGD SHARED ${SOURCES})

# Include directories for godot-cpp
target_include_directories(PureDataGD PRIVATE
  ${godot-cpp_SOURCE_DIR}/include
  ${godot-cpp_BINARY_DIR}/include
)

# Include directories for libpd
target_include_directories(PureDataGD PRIVATE
  ${libpd_SOURCE_DIR}/libpd_wrapper
  ${libpd_SOURCE_DIR}/pure-data/src
)

# Link against godot-cpp
target_link_libraries(PureDataGD PRIVATE godot-cpp)
# Add the `libpd` library to your target
target_link_libraries(PureDataGD PRIVATE libpd)

# Define export macro and set C++ standard
target_compile_definitions(PureDataGD PRIVATE GODOT_CPP_EXPORT)
target_compile_features(PureDataGD PRIVATE cxx_std_17)

# Ensure build order: PureDataGD depends on godot-cpp
add_dependencies(PureDataGD godot-cpp)
# Ensure build order: PureDataGD depends on libpd
add_dependencies(PureDataGD libpd)
