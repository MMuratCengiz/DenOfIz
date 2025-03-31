cmake_minimum_required(VERSION 3.17.2)
enable_testing()

project(DenOfIzGraphics-Tests)

find_package(GTest CONFIG REQUIRED)

set(GeneralSources
        Source/General/BasicCompute.cpp
        Source/Assets/Stream/BinaryReaderWriterTests.cpp
        Source/Assets/Serde/AnimationAssetReaderWriterTests.cpp
        Source/Assets/Serde/MeshAssetReaderWriterTests.cpp
        Source/BitSetTest.cpp

)
set(SOURCE
        ${GeneralSources}
)

add_executable(DenOfIzGraphics-Tests ${SOURCE})
target_link_libraries(DenOfIzGraphics-Tests PRIVATE DenOfIzGraphics GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(DenOfIzGraphics-Tests)