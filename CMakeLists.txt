cmake_minimum_required(VERSION 3.14)
project(HyLoRD)

include(FetchContent)

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

## Build ##
file(GLOB HyLoRD_SRC "src/*.hpp" "src/*.cpp")
add_executable(hylord ${HyLoRD_SRC})
target_link_libraries(hylord PRIVATE Threads::Threads)
target_link_libraries(hylord PRIVATE CLI11::CLI11)

## Install ##
install(TARGETS hylord
RUNTIME DESTINATION bin
)
