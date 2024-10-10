find_package(SDL2 CONFIG REQUIRED)

set(DEN_OF_IZ_EXAMPLES_BASE_SOURCES
        Source/Camera.cpp
        Source/DefaultRenderPipeline.cpp
        Source/Main.cpp
        Source/NullTexture.cpp
        Source/PerDrawBinding.cpp
        Source/PerFrameBinding.cpp
        Source/PerMaterialBinding.cpp
        Source/QuadPipeline.cpp
        Source/SphereAsset.cpp
)

add_library(DenOfIzExamples-Base ${LIB_TYPE} ${DEN_OF_IZ_EXAMPLES_BASE_SOURCES})
target_include_default_directories(DenOfIzExamples-Base)
target_link_libraries(DenOfIzExamples-Base
        PUBLIC
        DenOfIzGraphics
)

apple_lib(DenOfIzExamples-Base)