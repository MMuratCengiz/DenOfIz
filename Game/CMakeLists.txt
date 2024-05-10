include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")
include("${PROJECT_SOURCE_DIR}/Engine/The-Forge/cmake/theforge_helper.cmake")


set(DenOfIzGameSources
    "Source/DemoScene.cpp" 
    "Source/MainApp.cpp"
)

add_library(DenOfIzGame ${LIB_TYPE} ${DenOfIzGameSources})

#DEPENDENCIES
# find_package(unofficial-gainput CONFIG REQUIRED)
# target_link_libraries(DenOfIzGame PRIVATE unofficial::gainput::gainputstatic)
#--DEPENDENCIES
install_target(DenOfIzGame)

target_include_default_directories(DenOfIzGame)
set_target_default_properties(DenOfIzGame)

include_theforge(DenOfIzGame)

compile_shaders(
    TARGET_PROJECT DenOfIzGame
    SHADER_TARGET UiShader
    SHADER_LIST "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/UI/Shaders/FSL/UI_ShaderList.fsl"
)

compile_shaders(
    TARGET_PROJECT DenOfIzGame
    SHADER_TARGET FontShader
    SHADER_LIST "${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/Application/Fonts/Shaders/FSL/Fonts_ShaderList.fsl"
)

compile_shaders(
    TARGET_PROJECT DenOfIzGame
    SHADER_TARGET ProjectShader
    SHADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/FSL/ShaderList.fsl"
)