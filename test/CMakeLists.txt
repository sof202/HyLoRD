if(HYLORD_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    QUIET
    GIT_REPOSITORY  https://github.com/google/googletest/
    GIT_TAG  52eb8108c5bdec04579160ae17225d66034bd723 #v1.17.0
  )
  # For Windows: Prevent overriding the parent project's compiler/linker
  # settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  file(COPY integration/test_data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  add_executable(
    hylord_test
    unit/PercentageProportionConversionTest.cpp
    integration/TSVFileReaderTest.cpp
  )
  target_link_libraries(
    hylord_test
    hylord_lib
    GTest::gtest_main
  )

  target_compile_definitions(
    hylord_test
    PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  )

  include(GoogleTest)
  gtest_discover_tests(hylord_test)
endif()
