find_package(imgui CONFIG REQUIRED)

include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_IMGUI_EXAMPLE_SOURCES
        main.cpp
        Source/ImGuiExample.cpp
        ${PROJECT_SOURCE_DIR}/ImGui/ImGuiBackend.cpp
        ${PROJECT_SOURCE_DIR}/ImGui/EmbeddedImGuiShaders.cpp
)

add_executable(DenOfIzExamples-ImGuiExample MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_IMGUI_EXAMPLE_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-ImGuiExample)
target_include_default_directories(DenOfIzExamples-ImGuiExample)
target_link_libraries(DenOfIzExamples-ImGuiExample
        PUBLIC
        DenOfIzExamples-Base
        imgui::imgui
)
denofiz_app(DenOfIzExamples-ImGuiExample)