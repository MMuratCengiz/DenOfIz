# Download curl binary at configure time
set(CURL_VERSION "8.4.0")
set(CURL_DIR "${CMAKE_CURRENT_BINARY_DIR}/curl")

if(WIN32)
    set(CURL_URL "https://curl.se/windows/dl-${CURL_VERSION}_6/curl-${CURL_VERSION}_6-win64-mingw.zip")
    set(CURL_EXECUTABLE "${CURL_DIR}/bin/curl.exe")
elseif(APPLE)
    set(CURL_EXECUTABLE "curl")
else()
    set(CURL_EXECUTABLE "curl")
endif()

if(WIN32 AND NOT EXISTS "${CURL_EXECUTABLE}")
    message(STATUS "Downloading curl for Windows...")
    file(DOWNLOAD ${CURL_URL} "${CMAKE_CURRENT_BINARY_DIR}/curl.zip" 
         SHOW_PROGRESS
         STATUS CURL_DOWNLOAD_STATUS)
    
    list(GET CURL_DOWNLOAD_STATUS 0 CURL_DOWNLOAD_ERROR)
    if(CURL_DOWNLOAD_ERROR EQUAL 0)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_CURRENT_BINARY_DIR}/curl.zip"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        file(GLOB CURL_EXTRACTED_DIR "${CMAKE_CURRENT_BINARY_DIR}/curl-*")
        if(CURL_EXTRACTED_DIR)
            file(MAKE_DIRECTORY "${CURL_DIR}/bin")
            file(COPY "${CURL_EXTRACTED_DIR}/bin/curl.exe" DESTINATION "${CURL_DIR}/bin/")
        endif()
    else()
        message(WARNING "Failed to download curl, falling back to system curl")
        set(CURL_EXECUTABLE "curl")
    endif()
endif()

add_executable(FontToEmbeddedInl FontToEmbeddedInl.cpp GoogleFontsDownloader.cpp)
target_link_libraries(FontToEmbeddedInl PRIVATE
    DenOfIzGraphics
    miniz::miniz
)
target_compile_definitions(FontToEmbeddedInl PRIVATE
    CURL_EXECUTABLE="${CURL_EXECUTABLE}"
)

add_executable(DeployFont DeployFont.cpp GoogleFontsDownloader.cpp)
target_link_libraries(DeployFont PRIVATE
    DenOfIzGraphics
)
target_compile_definitions(DeployFont PRIVATE
    CURL_EXECUTABLE="${CURL_EXECUTABLE}"
)

add_executable(ShaderToEmbeddedInl ShaderToEmbeddedInl.cpp)
target_link_libraries(ShaderToEmbeddedInl PRIVATE
    DenOfIzGraphics
    miniz::miniz
)