include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

add_executable(DenOfIzExamples-UIBenchmark MACOSX_BUNDLE main.cpp)
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets/Fonts" * DenOfIzExamples-UIBenchmark)
target_link_libraries(DenOfIzExamples-UIBenchmark
        PUBLIC
        DenOfIzExamples-Base
)
denofiz_app(DenOfIzExamples-UIBenchmark)
