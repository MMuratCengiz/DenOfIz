cmake_minimum_required(VERSION 3.17.2)
enable_testing()

project(DenOfIzCore-Tests)

find_package(GTest CONFIG REQUIRED)

set(SOURCE Source/BitSetTest.cpp)
add_executable(DenOfIzCore-Tests ${SOURCE})
target_link_libraries(DenOfIzCore-Tests PRIVATE DenOfIzCore GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(DenOfIzCore-Tests)
