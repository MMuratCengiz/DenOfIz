cmake_minimum_required(VERSION 3.20)

add_executable(DenOfIzBindingGenerator ${BINDING_CORE_SOURCES} Source/CSharpMain.cpp)
target_include_default_directories(DenOfIzBindingGenerator)
target_include_directories(DenOfIzBindingGenerator PRIVATE ${BINDINGS_LLVM_INCLUDE_DIRS})
if (BINDINGS_LLVM_DEFINITIONS)
	target_compile_definitions(DenOfIzBindingGenerator PRIVATE ${BINDINGS_LLVM_DEFINITIONS})
endif ()
target_compile_definitions(DenOfIzBindingGenerator PRIVATE DZB_BUILD_DLL)
target_link_libraries(DenOfIzBindingGenerator PRIVATE ${LIBCLANG_LIB} DenOfIzGraphics spdlog::spdlog)

file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" DENOFIZ_BINDINGS_REPO_PATH)
target_compile_definitions(DenOfIzBindingGenerator PRIVATE DENOFIZ_BINDINGS_REPOSITORY_ROOT=\"${DENOFIZ_BINDINGS_REPO_PATH}\")

set(DENOFIZ_GRAPHICS_INCLUDE_ROOT "${CMAKE_SOURCE_DIR}/Graphics/Include/")
file(TO_CMAKE_PATH "${DENOFIZ_GRAPHICS_INCLUDE_ROOT}" DENOFIZ_GRAPHICS_INCLUDE_ROOT)
target_compile_definitions(DenOfIzBindingGenerator PRIVATE DENOFIZ_GRAPHICS_INCLUDE_ROOT=\"${DENOFIZ_GRAPHICS_INCLUDE_ROOT}\")

if (WIN32 AND DEFINED LLVM_ROOT_DIR AND EXISTS "${LLVM_ROOT_DIR}/bin/libclang.dll")
	add_custom_command(TARGET DenOfIzBindingGenerator POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LLVM_ROOT_DIR}/bin/libclang.dll" $<TARGET_FILE_DIR:DenOfIzBindingGenerator>
	)
endif ()

add_custom_target(DenOfIzCSharpGenerate
	COMMAND ${CMAKE_COMMAND} -E make_directory "${DENOFIZ_GENERATED_CSHARP_DIR}"
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_SOURCE_DIR}" "$<TARGET_FILE:DenOfIzBindingGenerator>"
	DEPENDS DenOfIzBindingGenerator
	COMMENT "Generating C# bindings"
	VERBATIM
)

if (NOT DOTNET_EXECUTABLE)
	message(STATUS "dotnet not found. DenOfIzCSharpBuild target unavailable.")
	return()
endif ()

if (CMAKE_CONFIGURATION_TYPES)
	set(BINDINGS_DOTNET_CONFIGURATION "$<CONFIG>")
else ()
	if (CMAKE_BUILD_TYPE)
		set(BINDINGS_DOTNET_CONFIGURATION "${CMAKE_BUILD_TYPE}")
	else ()
		set(BINDINGS_DOTNET_CONFIGURATION "Debug")
	endif ()
endif ()

set(BINDINGS_NATIVE_FILES "$<TARGET_FILE:DenOfIzGraphics>")
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
	list(APPEND BINDINGS_NATIVE_FILES "$<TARGET_RUNTIME_DLLS:DenOfIzGraphics>")
endif ()

if (WIN32)
	set(BINDINGS_PLATFORM_SEGMENT "win32")
	set(BINDINGS_BINARY_SUBDIR "bin")
	list(APPEND BINDINGS_NATIVE_FILES
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/dxcompiler.dll"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/dxil.dll"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/WinPixEventRuntime.dll"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/wgpu_native.dll")
	if (NOT CPU_ARCHITECTURE STREQUAL "arm64")
		list(APPEND BINDINGS_NATIVE_FILES
				"$<TARGET_FILE_DIR:DenOfIzGraphics>/metalirconverter.dll")
	endif ()
elseif (APPLE)
	if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64" OR CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
		set(BINDINGS_PLATFORM_SEGMENT "osx-arm64")
	else ()
		set(BINDINGS_PLATFORM_SEGMENT "osx-x64")
	endif ()
	set(BINDINGS_BINARY_SUBDIR "lib")
	list(APPEND BINDINGS_NATIVE_FILES
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/libdxcompiler.dylib"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/libmetalirconverter.dylib"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/libwgpu_native.dylib")
else ()
	set(BINDINGS_PLATFORM_SEGMENT "linux")
	set(BINDINGS_BINARY_SUBDIR "lib")
	list(APPEND BINDINGS_NATIVE_FILES
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/libdxcompiler.so"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/libdxil.so"
			"$<TARGET_FILE_DIR:DenOfIzGraphics>/libwgpu_native.so")
endif ()

list(REMOVE_DUPLICATES BINDINGS_NATIVE_FILES)

set(BINDINGS_NATIVE_ROOT "${CMAKE_SOURCE_DIR}/Bindings/CSharp/Native/DenOfIzGraphics")
file(TO_CMAKE_PATH "${BINDINGS_NATIVE_ROOT}" BINDINGS_NATIVE_ROOT)
set(BINDINGS_NATIVE_DESTINATION "${BINDINGS_NATIVE_ROOT}/${BINDINGS_PLATFORM_SEGMENT}/${BINDINGS_BINARY_SUBDIR}")
string(REPLACE ";" "|" BINDINGS_NATIVE_FILES_ESC "${BINDINGS_NATIVE_FILES}")

set(BINDINGS_MANAGED_PROJECT "${CMAKE_SOURCE_DIR}/Bindings/CSharp/DenOfIzGraphicsCSharp.csproj")
set(BINDINGS_SAMPLE_PROJECT "${CMAKE_SOURCE_DIR}/Bindings/CSharp/DenOfIzGraphicsExample/DenOfIzGraphicsExample.csproj")

add_custom_target(DenOfIzCSharpBuild
		COMMAND ${CMAKE_COMMAND} "-DDESTINATION=${BINDINGS_NATIVE_DESTINATION}" "-DFILES=${BINDINGS_NATIVE_FILES_ESC}" -DCLEAR_DESTINATION=ON -P "${CMAKE_SOURCE_DIR}/Bindings/cmake/CopyRuntime.cmake"
		COMMAND ${CMAKE_COMMAND} -E env DOTNET_CLI_TELEMETRY_OPTOUT=1 ${DOTNET_EXECUTABLE} build "${BINDINGS_MANAGED_PROJECT}" --configuration ${BINDINGS_DOTNET_CONFIGURATION}
		COMMAND ${CMAKE_COMMAND} -E env DOTNET_CLI_TELEMETRY_OPTOUT=1 ${DOTNET_EXECUTABLE} build "${BINDINGS_SAMPLE_PROJECT}" --configuration ${BINDINGS_DOTNET_CONFIGURATION}
		COMMENT "Building C# bindings"
		VERBATIM
)
add_dependencies(DenOfIzCSharpBuild DenOfIzCSharpGenerate DenOfIzGraphics)
