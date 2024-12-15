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
FetchContent_MakeAvailable(godot-cpp)

# > ====== libpd ======
FetchContent_Declare(
  libpd
  GIT_REPOSITORY https://github.com/libpd/libpd.git
  GIT_TAG master # Use an appropriate tag or branch
)
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
target_include_directories(PureDataGD PRIVATE
  ${godot-cpp_SOURCE_DIR}/include
  ${godot-cpp_BINARY_DIR}/include
)
# Include directories for libpd
target_include_directories(PureDataGD PRIVATE
  ${libpd_SOURCE_DIR}/cpp
  ${libpd_SOURCE_DIR}/libpd_wrapper
  ${libpd_SOURCE_DIR}/libpd_wrapper/util
  ${libpd_SOURCE_DIR}/pure-data/src
)

# ===== Link libraries =====
# Link against godot-cpp
target_link_libraries(PureDataGD PRIVATE godot-cpp)
# Add the `libpd` library to your target
target_link_libraries(PureDataGD PRIVATE libpd)

# Add install target
install(TARGETS PureDataGD libpd DESTINATION bin)

# ====== Configure the project defaults ======
set(GODOT_PRECISION "double" CACHE STRING "Precision used in Godot" FORCE)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/demo
  CACHE STRING "Install prefix" FORCE)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(GODOT_USE_HOT_RELOAD ON CACHE STRING "Enable hot  reloading" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
  set(GODOT_USE_HOT_RELOAD OFF CACHE STRING "Enable hot  reloading" FORCE)

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3")
  endif()
endif()
