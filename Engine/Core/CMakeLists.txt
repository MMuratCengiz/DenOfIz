include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

find_package(Boost COMPONENTS thread atomic log REQUIRED)

find_path(STB_INCLUDE_DIRS "stb_image.h")

set(DEN_OF_IZ_CORE_SOURCES
        Source/Utilities.cpp
        Source/Time.cpp
        Source/Logger.cpp)

add_library(DenOfIzCore ${LIB_TYPE} ${DEN_OF_IZ_CORE_SOURCES})

target_include_default_directories(DenOfIzCore)
target_link_libraries(DenOfIzCore
        PUBLIC
        Boost::boost
        Boost::thread
        Boost::atomic
        Boost::log
)
target_include_directories(DenOfIzCore PUBLIC PRIVATE ${STB_INCLUDE_DIRS})
install_target(DenOfIzCore)

set_target_properties(DenOfIzCore PROPERTIES LINKER_LANGUAGE CXX)
set_target_default_properties(DenOfIzCore)