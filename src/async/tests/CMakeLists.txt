enable_testing()

add_executable(
  conjur_async_tests
    simple_thread_pool_tests.cpp
)
target_link_libraries(
  conjur_async_tests
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)

target_include_directories(
  conjur_async_tests
    PUBLIC
      ${conjur_async_includes}
)

include(GoogleTest)
gtest_discover_tests(conjur_async_tests)
