if (WIN32)
    add_definitions(-D_WIN32=1)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
endif()

set(CMAKE_CXX_STANDARD 23)