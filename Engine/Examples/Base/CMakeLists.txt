find_package(SDL2 CONFIG REQUIRED)

set(DEN_OF_IZ_EXAMPLES_BASE_SOURCES
        Source/Main.cpp
        Source/Camera.cpp
)

add_library(DenOfIzExamples-Base ${LIB_TYPE} ${DEN_OF_IZ_EXAMPLES_BASE_SOURCES})
target_include_default_directories(DenOfIzExamples-Base)

target_link_libraries(DenOfIzExamples-Base
        PUBLIC
        DenOfIzGraphics
)