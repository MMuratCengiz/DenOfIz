find_package(SDL2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(DEN_OF_IZ_EXAMPLES_BASE_SOURCES
        Source/Assets/AssetData.cpp
        Source/Assets/MaterialData.cpp
        Source/Camera.cpp
        Source/DefaultRenderPipeline.cpp
        Source/Main.cpp
        Source/NullTexture.cpp
        Source/PerDrawBinding.cpp
        Source/PerFrameBinding.cpp
        Source/PerMaterialBinding.cpp
        Source/QuadPipeline.cpp
        Source/SphereAsset.cpp
        Source/InteropMathConverter.cpp # Temporarily duplicated to help with pimplification
)

add_library(DenOfIzExamples-Base ${DEN_OF_IZ_EXAMPLES_BASE_SOURCES})
target_include_default_directories(DenOfIzExamples-Base)
target_link_libraries(DenOfIzExamples-Base
        PUBLIC
        DenOfIzGraphics
        spdlog::spdlog
        SDL2::SDL2
        SDL2::SDL2main
)
if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzExamples-Base PRIVATE DZ_EXAMPLES_EXPORTS)
    target_compile_definitions(DenOfIzExamples-Base INTERFACE DZ_EXAMPLES_IMPORTS)
endif ()
apple_lib(DenOfIzExamples-Base)