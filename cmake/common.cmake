include(${PROJECT_SOURCE_DIR}/cmake/include_definitions.cmake)

if (BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else ()
    set(LIB_TYPE STATIC)
endif ()

function(copy_to_binary RootSource Dir FileSelect)
    file(GLOB_RECURSE FilesInDir "${RootSource}/${Dir}/${FileSelect}")
    string(LENGTH "${RootSource}/${Dir}/" PathLen)

    foreach (File IN LISTS FilesInDir)
        get_filename_component(FileParentDir ${File} DIRECTORY)
        set(FileParentDir "${FileParentDir}/")

        string(LENGTH "${File}" FilePathLen)
        string(LENGTH "${FileParentDir}" DirPathLen)

        math(EXPR FileTrimmedLen "${FilePathLen}-${PathLen}")
        math(EXPR DirTrimmedLen "${DirPathLen}-${PathLen}")

        string(SUBSTRING ${File} ${PathLen} ${FileTrimmedLen} FileStripped)
        string(SUBSTRING ${FileParentDir} ${PathLen} ${DirTrimmedLen} DirStripped)

        set(BINARY_DIR "${PROJECT_BINARY_DIR}")
        if (APPLE)
            set(BINARY_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources")
        endif ()
        file(MAKE_DIRECTORY "${BINARY_DIR}/${Dir}")
        file(MAKE_DIRECTORY "${BINARY_DIR}/${Dir}/${DirStripped}")
        message("Configuring file: ${File} ${BINARY_DIR}/${Dir}/${DirStripped} COPYONLY")
        configure_file(${File} ${BINARY_DIR}/${Dir}/${DirStripped} COPYONLY)
    endforeach ()
endfunction()

function(INSTALL_TARGET target)
    if (INSTALL_LIBS)
        install(TARGETS ${target}
                EXPORT ${target}-export
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
        )

        install(EXPORT ${target}-export
                FILE ${target}Targets.cmake
                NAMESPACE DenOfIz::
                DESTINATION cmake/${target}
        )

        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Include/ DESTINATION Include)
    endif ()
endfunction()

function(SET_TARGET_DEFAULT_PROPERTIES target)
    if (WIN32 AND CRT_LINKAGE_STATIC)
        if (POLICY CMP0091)
            cmake_policy(SET CMP0091 NEW)
        endif ()
        set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
endfunction()

function(TARGET_INCLUDE_DEFAULT_DIRECTORIES target)
    target_include_directories(${target}
            PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Source
    )
endfunction()