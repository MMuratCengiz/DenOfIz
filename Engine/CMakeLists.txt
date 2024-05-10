include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

# set(SUB_PROJECTS Core Graphics TheForge)
set(SUB_PROJECTS The-Forge)

foreach(SUB_PROJECT IN LISTS SUB_PROJECTS)
    add_subdirectory(${SUB_PROJECT})
endforeach()

add_library(DenOfIzEngine ${LIB_TYPE} Engine.h)
target_link_libraries(DenOfIzEngine
        PUBLIC
        # DenOfIzGraphics
        TheForge
)
set_target_default_properties(DenOfIzEngine)