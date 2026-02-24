# Sets up a target with DenOfIzGraphics runtime dependencies, ie. copies all necessary DLLs/shared libraries
function(denofiz_setup_target target)
    if(TARGET DenOfIz::DenOfIzGraphics)
        set(DENOFIZ_TARGET DenOfIz::DenOfIzGraphics)
    elseif(TARGET DenOfIzGraphics)
        set(DENOFIZ_TARGET DenOfIzGraphics)
    else()
        message(WARNING "DenOfIzGraphics target not found")
        return()
    endif()
    if(WIN32)
        set_target_properties(${target} PROPERTIES
            VS_DPI_AWARE "PerMonitor"
        )
        if(MSVC AND NOT MSVC_IDE)
            if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/dpi_aware.manifest")
                target_link_options(${target} PRIVATE "/MANIFEST:EMBED" "/MANIFESTINPUT:${CMAKE_CURRENT_LIST_DIR}/dpi_aware.manifest")
            elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../dpi_aware.manifest")
                target_link_options(${target} PRIVATE "/MANIFEST:EMBED" "/MANIFESTINPUT:${CMAKE_CURRENT_LIST_DIR}/../dpi_aware.manifest")
            endif()
        endif()
    endif()
    
    if(APPLE)
        set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST_KEY_NSHighResolutionCapable YES
        )
    endif()
    
    if(COMMAND denofiz_copy_runtime_dependencies)
        denofiz_copy_runtime_dependencies(${target})
    endif()
    get_target_property(DENOFIZ_COMPILE_DEFS ${DENOFIZ_TARGET} INTERFACE_COMPILE_DEFINITIONS)
    if("BUILD_WEBGPU_NATIVE" IN_LIST DENOFIZ_COMPILE_DEFS)
        # For build tree usage, we need to fetch WebGPU again to get access to target_copy_webgpu_binaries
        if(NOT TARGET webgpu)
            include(FetchContent)
            FetchContent_Declare(
                webgpu
                GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
                GIT_TAG wgpu-v24.0.0.2
            )
            FetchContent_MakeAvailable(webgpu)
        endif()
        if(COMMAND target_copy_webgpu_binaries)
            target_copy_webgpu_binaries(${target})
        endif()
    endif()
endfunction()
macro(target_link_denofiz_graphics target)
    if(TARGET DenOfIz::DenOfIzGraphics)
        target_link_libraries(${target} ${ARGN} DenOfIz::DenOfIzGraphics)
    elseif(TARGET DenOfIzGraphics)
        target_link_libraries(${target} ${ARGN} DenOfIzGraphics)
    else()
        message(FATAL_ERROR "DenOfIzGraphics target not found")
    endif()
    denofiz_setup_target(${target})
endmacro()