cmake_minimum_required(VERSION 3.17.2)
enable_testing()

project(DenOfIzGraphics-Tests)

find_package(GTest CONFIG REQUIRED)

set(GeneralSources
        Source/General/BasicCompute.cpp
        Source/Assets/Import/AssimpImporterTest.cpp
        Source/Assets/Stream/BinaryReaderWriterTests.cpp
        Source/Assets/Serde/AnimationAssetReaderWriterTests.cpp
        Source/Assets/Serde/MaterialAssetReaderWriterTests.cpp
        Source/Assets/Serde/MeshAssetReaderWriterTests.cpp
        Source/Assets/Serde/SkeletonAssetReaderWriterTests.cpp
        Source/Assets/Serde/PhysicsAssetReaderWriterTests.cpp
        Source/Assets/Serde/TextureAssetReaderWriterTests.cpp
        Source/BitSetTest.cpp
        Source/TestComparators.h

)
set(SOURCE
        ${GeneralSources}
)

set(DZ_TEST_DATA_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
set(DZ_TEST_DATA_DEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TestDataOut)

add_executable(DenOfIzGraphics-Tests ${SOURCE})
target_compile_definitions(DenOfIzGraphics-Tests PRIVATE
        DZ_TEST_DATA_SRC_DIR="${DZ_TEST_DATA_SRC_DIR}"
        DZ_TEST_DATA_DEST_DIR="${DZ_TEST_DATA_DEST_DIR}"
)
target_link_libraries(DenOfIzGraphics-Tests PRIVATE DenOfIzGraphics GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)


include(GoogleTest)
gtest_discover_tests(DenOfIzGraphics-Tests)