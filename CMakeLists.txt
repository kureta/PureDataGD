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
)
# Fetch and make godot-cpp available
set(GODOT_PRECISION "double" CACHE STRING "Precision used in Godot" FORCE)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(GODOT_USE_HOT_RELOAD ON CACHE STRING "Enable hot  reloading" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
  set(GODOT_USE_HOT_RELOAD OFF CACHE STRING "Enable hot  reloading" FORCE)
endif()
FetchContent_MakeAvailable(godot-cpp)

# > ====== libpd ======
set(MULTI_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/libpd-multi.patch)
FetchContent_Declare(
  libpd
  GIT_REPOSITORY https://github.com/libpd/libpd.git
  GIT_TAG 39918e8 # Use an appropriate tag or branch
  PATCH_COMMAND ${MULTI_PATCH}
  UPDATE_DISCONNECTED 1
)
# LIBPD_STATIC does not compile on MSVC
set(LIBPD_SHARED ON CACHE STRING "Build libpd as a shared library" FORCE)
set(LIBPD_STATIC OFF CACHE STRING "Build libpd as a static library" FORCE)
set(PD_MULTI ON CACHE STRING "Multiple instances" FORCE)
FetchContent_MakeAvailable(libpd)

# ====== Define and setup the target ======
# Specify the source files
set(SOURCES
  src/puredatagd.cpp
  src/register_types.cpp
)
# Add PureDataGD target
add_library(PureDataGD SHARED ${SOURCES})
# Set the C++ standard to C++17
set_target_properties(PureDataGD PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

# ===== Include directories =====
# Include directories for godot-cpp
target_include_directories(PureDataGD SYSTEM PRIVATE
  ${godot-cpp_SOURCE_DIR}/include
  ${godot-cpp_BINARY_DIR}/include
)
# Include directories for libpd
target_include_directories(PureDataGD SYSTEM PRIVATE
  ${libpd_SOURCE_DIR}/cpp
  ${libpd_SOURCE_DIR}/libpd_wrapper
  ${libpd_SOURCE_DIR}/libpd_wrapper/util
  ${libpd_SOURCE_DIR}/pure-data/src
)

target_compile_options(PureDataGD PRIVATE
  -Wall -Wextra -Werror -pedantic -Wshadow -Wconversion
  -fsanitize=address -fsanitize=undefined -Weffc++
  -fstack-protector-strong -D_GLIBCXX_DEBUG -std=c++17
)

# ===== Link libraries =====
# Link against godot-cpp
target_link_libraries(PureDataGD PRIVATE godot-cpp)
# Add the `libpd` library to your target
target_link_libraries(PureDataGD PRIVATE libpd)

# Add install target
install(TARGETS PureDataGD libpd DESTINATION bin)

# ====== Configure the project defaults ======
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/demo
  CACHE STRING "Install prefix" FORCE)
