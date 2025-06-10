file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../../version.txt" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" DENOFIZ_VERSION)

set(CPPSHARP_PROJECT_NAME DenOfIzGraphics.CppSharp)
set(CPPSHARP_GENERATOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
	set(ARCH_NAME "arm64")
else()
	set(ARCH_NAME "x64")
endif()

set(CPPSHARP_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Out)
set(CPPSHARP_CODE_DIR ${CPPSHARP_OUT_DIR}/Code)
set(CPPSHARP_PROJECT_DIR ${CPPSHARP_OUT_DIR}/Project)
set(NUGET_BASE_DIR ${CPPSHARP_PROJECT_DIR}/NuGet)
set(NUGET_RUNTIMES_WIN_DIR ${NUGET_BASE_DIR}/runtimes/win-x64/native)
set(NUGET_RUNTIMES_OSX_DIR ${NUGET_BASE_DIR}/runtimes/osx-arm64/native)
set(NUGET_RUNTIMES_LINUX_DIR ${NUGET_BASE_DIR}/runtimes/linux-x64/native)

message("Osx dir: ${NUGET_RUNTIMES_OSX_DIR}")

file(MAKE_DIRECTORY ${CPPSHARP_OUT_DIR})
file(MAKE_DIRECTORY ${CPPSHARP_CODE_DIR})
file(MAKE_DIRECTORY ${CPPSHARP_PROJECT_DIR})
file(MAKE_DIRECTORY ${NUGET_BASE_DIR})

find_program(DOTNET_EXECUTABLE dotnet)
if(NOT DOTNET_EXECUTABLE)
	message(STATUS "dotnet executable not found. Skipping CppSharp bindings generation.")
	return()
endif()

add_custom_target(DenOfIz-CppSharpGenerate
	COMMAND ${DOTNET_EXECUTABLE} run --project "${CPPSHARP_GENERATOR_DIR}/DenOfIzGraphics.CppSharp/DenOfIzGraphics.CppSharp.csproj"
	WORKING_DIRECTORY ${CPPSHARP_GENERATOR_DIR}/DenOfIzGraphics.CppSharp
	COMMENT "Generating CppSharp C# bindings..."
	VERBATIM
)

add_custom_target(DenOfIz-CppSharpNuGet
	DEPENDS DenOfIz-CppSharpGenerate
	COMMENT "Building CppSharp NuGet package..."
)

if(WIN32)
	set(PLATFORM_DIR "win-x64")
	set(LIB_PREFIX "")
	set(LIB_EXTENSION ".dll")
	set(STATIC_LIB_EXTENSION ".lib")
elseif(APPLE)
	set(PLATFORM_DIR "osx-arm64")
	set(LIB_PREFIX "lib")
	set(LIB_EXTENSION ".dylib")
	set(STATIC_LIB_EXTENSION ".a")
else()
	set(PLATFORM_DIR "linux-x64")
	set(LIB_PREFIX "lib")
	set(LIB_EXTENSION ".so")
	set(STATIC_LIB_EXTENSION ".a")
endif()

add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BASE_DIR}/lib/netstandard2.0
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BASE_DIR}/build
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BASE_DIR}/src
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BASE_DIR}/src_gen
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_WIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_OSX_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_LINUX_DIR}
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CPPSHARP_CODE_DIR}/DenOfIzGraphics.cs ${NUGET_BASE_DIR}/src_gen/
	COMMAND ${CMAKE_COMMAND} -E copy ${CPPSHARP_CODE_DIR}/Std.cs ${NUGET_BASE_DIR}/src_gen/
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/DenOfIzGraphics.CppSharp.nuspec ${NUGET_BASE_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/DenOfIzGraphics.CppSharp.csproj ${NUGET_BASE_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/build/DenOfIzGraphics.CppSharp.targets ${NUGET_BASE_DIR}/build/
	
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/src/ ${NUGET_BASE_DIR}/src/
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/bin/${LIB_PREFIX}DenOfIzGraphics${LIB_EXTENSION} ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
	COMMENT "Preparing CppSharp NuGet package structure..."
)

if(WIN32)
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/bin/dxcompiler.dll ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/bin/dxil.dll ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/bin/metalirconverter.dll ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
	)
elseif(APPLE)
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libdxcompiler.dylib ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libmetalirconverter.dylib ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
	)
else()
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libdxcompiler.so ${NUGET_BASE_DIR}/runtimes/${PLATFORM_DIR}/native/
	)
endif()

add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
	COMMAND ${DOTNET_EXECUTABLE} build ${NUGET_BASE_DIR}/DenOfIzGraphics.CppSharp.csproj -c Release --output lib/netstandard2.0
	WORKING_DIRECTORY ${NUGET_BASE_DIR}
	COMMENT "Building CppSharp managed assembly..."
)

include(${CPPSHARP_GENERATOR_DIR}/Nuget_Exe.cmake)
add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E echo "Building NuGet package..."
		COMMAND ${NUGET_COMMAND} pack ${NUGET_BASE_DIR}/DenOfIzGraphics.CppSharp.nuspec -Version ${DENOFIZ_VERSION} -OutputDirectory ${CPPSHARP_PROJECT_DIR}
		WORKING_DIRECTORY ${NUGET_BASE_DIR}
)