include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

find_path(STB_INCLUDE_DIRS "stb_image.h")
find_package(glog CONFIG REQUIRED)

set(DEN_OF_IZ_CORE_SOURCES
        Source/Utilities.cpp
        Source/Time.cpp)

add_library(DenOfIzCore ${LIB_TYPE} ${DEN_OF_IZ_CORE_SOURCES})

target_include_default_directories(DenOfIzCore)
target_link_libraries(DenOfIzCore
        PUBLIC
        glog::glog
)
target_include_directories(DenOfIzCore PUBLIC PRIVATE ${STB_INCLUDE_DIRS})
install_target(DenOfIzCore)

set_target_properties(DenOfIzCore PROPERTIES LINKER_LANGUAGE CXX)
set_target_default_properties(DenOfIzCore)

if (BUILD_TESTING)
    add_subdirectory(Tests)
endif ()