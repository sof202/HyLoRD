cmake_minimum_required(VERSION 3.14)
project(HyLoRD)

include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  # Auto-create a .gitignore in the specified "build" directory.
  file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

## External Libs ##
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        4160d259d961cd393fd8d67590a8c7d210207348 # v2.5.0
)
FetchContent_MakeAvailable(cli11_proj)

# Eigen3 has a bunch of unecessary build steps that aren't needed for HyLoRD
# Like building tests, .pc file generation and documentation building. Skipping
# these saves on build time
FetchContent_Declare(
  Eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3147391d946bb4b6c68edd901f2add6ac1f31f8c # v3.4.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
set(original_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_MPL2_ONLY ON)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_BUILD_DOC OFF)
FetchContent_MakeAvailable(Eigen)
set(BUILD_TESTING ${original_BUILD_TESTING})

## Build ##
file(GLOB HyLoRD
  src/main.cpp
  src/core/hylord.cpp
  src/data/BedRecords.cpp
)
add_executable(hylord ${HyLoRD})
target_include_directories(hylord PRIVATE src)
target_link_libraries(hylord PRIVATE Threads::Threads)
target_link_libraries(hylord PRIVATE CLI11::CLI11)
target_link_libraries(hylord PRIVATE Eigen3::Eigen)

## Install ##
install(TARGETS hylord
RUNTIME DESTINATION bin
)
