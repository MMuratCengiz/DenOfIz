include(${PROJECT_SOURCE_DIR}/cmake/include_definitions.cmake)

if (BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else ()
    set(LIB_TYPE STATIC)
endif ()

function(copy_to_binary_target RootSource Dir FileSelect Target)
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
            set(BINARY_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${Target}.app/Contents/Resources")
        endif ()
        file(MAKE_DIRECTORY "${BINARY_DIR}/${Dir}")
        file(MAKE_DIRECTORY "${BINARY_DIR}/${Dir}/${DirStripped}")
        message("Configuring file: ${File} ${BINARY_DIR}/${Dir}/${DirStripped} COPYONLY")
        configure_file(${File} ${BINARY_DIR}/${Dir}/${DirStripped} COPYONLY)
    endforeach ()
endfunction()

function(copy_to_binary RootSource Dir FileSelect)
    copy_to_binary_target(${RootSource} ${Dir} ${FileSelect} ${PROJECT_NAME})
endfunction()

function(INSTALL_TARGET target)
    install(TARGETS ${target}
            EXPORT ${target}-export
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )

    install(EXPORT ${target}-export
            FILE ${target}Targets.cmake
            NAMESPACE DenOfIz::
            DESTINATION share/cmake/${target}
    )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Include/ DESTINATION Include)
endfunction()

function(SET_TARGET_DEFAULT_PROPERTIES target)
    if (WIN32 AND CRT_LINKAGE_STATIC)
        if (POLICY CMP0091)
            cmake_policy(SET CMP0091 NEW)
        endif ()
        set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
    if (WIN32)
        set_target_properties(${target} PROPERTIES
            VS_DPI_AWARE "PerMonitor"
        )
        if (MSVC AND NOT MSVC_IDE)
            target_link_options(${target} PRIVATE "/MANIFEST:EMBED" "/MANIFESTINPUT:${CMAKE_CURRENT_LIST_DIR}/dpi_aware.manifest")
        endif()
    endif()
    
    if(APPLE)
        set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST_KEY_NSHighResolutionCapable YES
        )
    endif()
endfunction()

function(APPLE_APP target)
    if (APPLE)
        set_target_properties(${target} PROPERTIES
                BUNDLE True
                XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK YES
                MACOSX_BUNDLE_GUI_IDENTIFIER mmuratcengiz.${target}
                MACOSX_BUNDLE_EXECUTABLE_NAME ${target}
                MACOSX_BUNDLE_BUNDLE_NAME ${target}
                MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
                MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
                MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/OS/MacOS/MacOSBundleInfo.plist.in
                RESOURCE ${PROJECT_SOURCE_DIR}/OS/MacOS/MainMenu.xib
        )
        set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep")

        target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
                $<$<COMPILE_LANGUAGE:C>:-x objective-c>)

        set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
    endif ()
endfunction()
function(WEB_APP target)
    if (EMSCRIPTEN)
        set_target_properties(${target} PROPERTIES SUFFIX ".html")
        if (EMSCRIPTEN_VERSION VERSION_GREATER_EQUAL "4.0.10")
            set(EM_WEBGPU_FLAG "--use-port=emdawnwebgpu")
        else()
            set(EM_WEBGPU_FLAG "-sUSE_WEBGPU=1")
        endif()
        set(EM_LINK_FLAGS "-s ${EM_WEBGPU_FLAG} -s ASYNCIFY=1 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=536870912 -s USE_SDL=2 -s USE_FREETYPE=1 -s USE_HARFBUZZ=1")
        set(BUILD_ASSETS_PATH "${CMAKE_BINARY_DIR}/DenOfIz/${CMAKE_BUILD_TYPE}_Emscripten/Assets")
        if(NOT EXISTS "${BUILD_ASSETS_PATH}")
            set(BUILD_ASSETS_PATH "${CMAKE_BINARY_DIR}/Assets")
        endif()
        if(EXISTS "${BUILD_ASSETS_PATH}")
            set(EM_LINK_FLAGS "${EM_LINK_FLAGS} --preload-file ${BUILD_ASSETS_PATH}@Assets")
        else()
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Assets")
                set(EM_LINK_FLAGS "${EM_LINK_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../Assets@Assets")
            endif()
        endif()
        set_target_properties(${target} PROPERTIES LINK_FLAGS "${EM_LINK_FLAGS}")
    endif()
endfunction()
function(APPLE_LIB target)
    if (APPLE)
        target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
                $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
        set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
    endif ()
endfunction()

function(LINUX_LIB target)
    if (UNIX AND NOT APPLE)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>)
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
function(DENOFIZ_LIB target)
    set_target_default_properties(${target})
    apple_lib(${target})
endfunction()
function(DENOFIZ_APP target)
    set_target_default_properties(${target})
    apple_app(${target})
    web_app(${target})
    if(COMMAND denofiz_setup_target)
        denofiz_setup_target(${target})
    endif()
endfunction()