find_package(SDL3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_path(CGLTF_INCLUDE_DIRS "cgltf.h")

set(DEN_OF_IZ_EXAMPLES_BASE_SOURCES
        Source/Assets/AssetData.cpp
        Source/Assets/MaterialData.cpp
        Source/FreeCamera.cpp
        Source/FileIO.cpp
        Source/DefaultRenderPipeline.cpp
        Source/GltfExporter.cpp
        Source/GltfLoader.cpp
        Source/Main.cpp
        Source/NullTexture.cpp
        Source/PerDrawBinding.cpp
        Source/PerFrameBinding.cpp
        Source/PerMaterialBinding.cpp
        Source/QuadPipeline.cpp
        Source/SphereAsset.cpp
        Source/InteropMathConverter.cpp
)

add_library(DenOfIzExamples-Base ${DEN_OF_IZ_EXAMPLES_BASE_SOURCES})
target_include_default_directories(DenOfIzExamples-Base)
target_include_directories(DenOfIzExamples-Base PRIVATE ${CGLTF_INCLUDE_DIRS})
target_link_libraries(DenOfIzExamples-Base
        PUBLIC
        DenOfIzGraphics
        spdlog::spdlog
        SDL3::SDL3
        PRIVATE
        assimp::assimp
)
target_compile_definitions(DenOfIzExamples-Base PUBLIC -DEXAMPLES_PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzExamples-Base PRIVATE DZ_EXAMPLES_EXPORTS)
    target_compile_definitions(DenOfIzExamples-Base INTERFACE DZ_EXAMPLES_IMPORTS)
endif ()
denofiz_lib(DenOfIzExamples-Base)