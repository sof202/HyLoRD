cmake_minimum_required(VERSION 3.14)
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

# ------------------
# Git
# ------------------
if(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  # Auto-create a .gitignore in the specified "build" directory.
  file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()

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
file(GLOB HyLoRD
  src/main.cpp
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
add_executable(hylord ${HyLoRD})
target_include_directories(hylord PRIVATE src)
target_link_libraries(hylord PRIVATE Threads::Threads)
target_link_libraries(hylord PRIVATE CLI11::CLI11)
target_link_libraries(hylord PRIVATE Eigen3::Eigen)
target_link_libraries(hylord PRIVATE qpmad)
target_include_directories(hylord PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/third_party/pcg/include
)

# ------------------
# Install
# ------------------
install(TARGETS hylord
RUNTIME DESTINATION bin
)

# ------------------
# Doxygen
# ------------------
find_package( Doxygen QUIET )
if(NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen not found, doc target not available")
    return()
endif()

include(FetchContent)
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

