if(NOT DEFINED DESTINATION)
    message(FATAL_ERROR "DESTINATION variable is required.")
endif()

if(NOT DEFINED FILES)
    message(FATAL_ERROR "FILES variable is required.")
endif()

set(_destination "${DESTINATION}")
file(MAKE_DIRECTORY "${_destination}")

if(DEFINED CLEAR_DESTINATION AND CLEAR_DESTINATION)
    file(GLOB _existing "${_destination}/*")
    if(_existing)
        file(REMOVE ${_existing})
    endif()
endif()

string(REPLACE "|" ";" _file_list "${FILES}")

foreach(_file_path IN LISTS _file_list)
    if(_file_path STREQUAL "")
        continue()
    endif()

    if(EXISTS "${_file_path}")
        file(COPY "${_file_path}" DESTINATION "${_destination}")
    else()
        message(STATUS "Skipping missing runtime dependency: ${_file_path}")
    endif()
endforeach()
