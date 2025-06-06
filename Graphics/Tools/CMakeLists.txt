add_executable(FontToEmbeddedInl FontToEmbeddedInl.cpp)
target_link_libraries(FontToEmbeddedInl PRIVATE 
    DenOfIzGraphics
    miniz::miniz
)