include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

set(SUB_PROJECTS Core Graphics)
foreach (SUB_PROJECT IN LISTS SUB_PROJECTS)
    add_subdirectory(${SUB_PROJECT})
endforeach ()

add_library(DenOfIzEngine ${LIB_TYPE} Engine.cpp)
target_link_libraries(DenOfIzEngine
        PUBLIC
        DenOfIzGraphics
)
set_target_default_properties(DenOfIzEngine)

if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif ()