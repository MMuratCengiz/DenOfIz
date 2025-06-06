add_executable(CompressInterFont CompressInterFont.cpp)
target_include_directories(CompressInterFont PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(FontToEmbeddedInl FontToEmbeddedInl.cpp)
target_link_libraries(FontToEmbeddedInl PRIVATE 
    DenOfIzGraphics
    miniz::miniz
)