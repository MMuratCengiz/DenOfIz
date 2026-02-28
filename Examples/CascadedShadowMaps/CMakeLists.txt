include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_CASCADED_SHADOW_MAPS_SOURCES
        main.cpp
        Source/CascadedShadowMapsExample.cpp
)

add_executable(DenOfIzExamples-CascadedShadowMaps MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_CASCADED_SHADOW_MAPS_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-CascadedShadowMaps)
target_include_default_directories(DenOfIzExamples-CascadedShadowMaps)
target_link_libraries(DenOfIzExamples-CascadedShadowMaps
        PUBLIC
        DenOfIzExamples-Base
)
denofiz_app(DenOfIzExamples-CascadedShadowMaps)
denofiz_setup_target(DenOfIzExamples-CascadedShadowMaps)
