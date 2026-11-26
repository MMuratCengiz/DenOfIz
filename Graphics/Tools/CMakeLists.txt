add_executable(FontToEmbeddedInl FontToEmbeddedInl.cpp)
target_link_libraries(FontToEmbeddedInl PRIVATE miniz::miniz)

add_executable(ShaderToEmbeddedInl ShaderToEmbeddedInl.cpp)
target_link_libraries(ShaderToEmbeddedInl PRIVATE
    DenOfIzGraphics
    miniz::miniz
)
