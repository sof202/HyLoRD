cmake_minimum_required(VERSION 3.17)
project(HyLoRD)

include(FetchContent)
include(ExternalProject)

# ------------------
# Options
# ------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

add_compile_definitions(BUILD_TYPE="${CMAKE_BUILD_TYPE}")

# ------------------
# Git
# ------------------
if(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  # Auto-create a .gitignore in the specified "build" directory.
  file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

# Fallback
if(NOT GIT_TAG)
    set(GIT_TAG "v0.0.0")
endif()
if(NOT GIT_HASH)
  set(GIT_HASH "Not found")
endif()

add_compile_definitions(GIT_TAG="${GIT_TAG}")
add_compile_definitions(GIT_HASH="${GIT_HASH}")

# ------------------
# External Libraries
# ------------------
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
set(original_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "Disable tests for Eigen" FORCE)
set(EIGEN_TEST_NOQT ON CACHE BOOL "Disable Qt-dependent tests" FORCE)
set(EIGEN_BUILD_TESTING OFF CACHE BOOL "Disable Eigen tests" FORCE)
set(EIGEN_TEST_NOQT ON CACHE BOOL "Disable QT unit tests" FORCE)
set(EIGEN_MPL2_ONLY ON CACHE BOOL "Use only MPL2 code" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_DOC OFF CACHE BOOL "Disable documentation building" FORCE)

# There's also a lot of messages that Eigen3 outputs, which we would like to
# suppress (to avoid user confusion).
set(original_LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL ERROR)
FetchContent_Declare(
  Eigen
  QUIET
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3147391d946bb4b6c68edd901f2add6ac1f31f8c # v3.4.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(Eigen)
set(BUILD_TESTING 
  ${original_BUILD_TESTING} CACHE BOOL "Restore global test setting" FORCE
)
set(CMAKE_MESSAGE_LOG_LEVEL ${original_LOG_LEVEL})

# qpmad's CMakeLists.txt contains 'find_package(Eigen3)'. We just retrieved
# this above, so we just ensure that qpmad knows it exists and can find it.
set(EIGEN3_INCLUDE_DIR ${eigen_SOURCE_DIR})
set(Eigen3_FOUND TRUE)
if(NOT TARGET Eigen3::Eigen)
  add_library(Eigen3::Eigen INTERFACE IMPORTED)
  set_target_properties(Eigen3::Eigen PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${eigen_SOURCE_DIR}"
  )
endif()
FetchContent_Declare(
  qpmad
  QUIET
  GIT_REPOSITORY https://github.com/asherikov/qpmad.git
  GIT_TAG b2fd8d57d973cab5d74decc1bbfc3c622d111240 #1.4.0
)
FetchContent_MakeAvailable(qpmad)

# ------------------
# Build
# ------------------
file(GLOB HyLoRD_SOURCES
  src/cli.cpp
  src/core/hylord.cpp
  src/core/Deconvolver.cpp
  src/data/BedRecords.cpp
  src/data/BedData.cpp
  src/maths/LinearAlgebra.cpp
  src/io/writeMetrics.cpp
  src/data/DataProcessing.cpp
  src/data/Filters.cpp
)
add_library(hylord_lib STATIC ${HyLoRD_SOURCES})
target_include_directories(hylord_lib 
  PUBLIC 
    src
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pcg-cpp/include
)
target_link_libraries(hylord_lib 
  PUBLIC
    CLI11::CLI11
    Eigen3::Eigen
    qpmad
  PRIVATE
    Threads::Threads
)

add_executable(hylord src/main.cpp)
target_link_libraries(hylord PRIVATE hylord_lib CLI11::CLI11)

# ------------------
# Install
# ------------------
install(TARGETS hylord
RUNTIME DESTINATION bin
)

# ------------------
# Tests
# ------------------
if(HYLORD_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

# ------------------
# Doxygen
# ------------------
find_package( Doxygen QUIET )
if(NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen not found, doc target not available")
    return()
endif()

FetchContent_Declare(
    doxygen-awesome-css
    GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
    GIT_TAG 568f56cde6ac78b6dfcc14acd380b2e745c301ea # v2.3.4
)
FetchContent_MakeAvailable(doxygen-awesome-css)

FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

if(DOT)
    set(HAVE_DOT YES)
else(DOT)
    set(HAVE_DOT NO)
endif(DOT)

configure_file(
  ${PROJECT_SOURCE_DIR}/docs/doxyfile.cmake 
  ${PROJECT_BINARY_DIR}/docs/${PROJECT_NAME}.doxyfile
)

add_custom_target(
  docs
  DEPENDS ${PROJECT_BINARY_DIR}/docs/${PROJECT_NAME}.doxyfile
  COMMAND ${DOXYGEN} ${PROJECT_BINARY_DIR}/docs/${PROJECT_NAME}.doxyfile
)

