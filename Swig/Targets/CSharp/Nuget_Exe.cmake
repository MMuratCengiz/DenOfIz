set(TOOLS_DIR ${CMAKE_BINARY_DIR}/tools)
file(MAKE_DIRECTORY ${TOOLS_DIR})

if (WIN32)
    set(NUGET_EXE ${TOOLS_DIR}/nuget.exe)
    set(NUGET_URL "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe")
    set(NUGET_COMMAND ${NUGET_EXE})
else ()
    find_program(MONO_EXECUTABLE mono REQUIRED)
    set(NUGET_EXE ${TOOLS_DIR}/nuget.exe)
    set(NUGET_URL "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe")
    set(NUGET_COMMAND ${MONO_EXECUTABLE} ${NUGET_EXE})
endif ()

if (NOT EXISTS ${NUGET_EXE})
    message(STATUS "Downloading NuGet to ${NUGET_EXE}...")
    file(DOWNLOAD
            "${NUGET_URL}"
            ${NUGET_EXE}
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
    )
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if (NOT STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download NuGet CLI.")
    endif ()
    if (NOT WIN32)
        file(CHMOD ${NUGET_EXE} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif ()
    message(STATUS "NuGet downloaded successfully.")
endif ()