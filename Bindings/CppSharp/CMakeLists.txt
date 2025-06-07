set(CPPSHARP_PROJECT_NAME DenOfIzGraphics.CppSharp)
set(CPPSHARP_GENERATOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CPPSHARP_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Out)
set(CPPSHARP_CODE_DIR ${CPPSHARP_OUT_DIR}/Code)
set(CPPSHARP_PROJECT_DIR ${CPPSHARP_OUT_DIR}/Project)
set(CPPSHARP_NUGET_DIR ${CPPSHARP_PROJECT_DIR}/NuGet)

file(MAKE_DIRECTORY ${CPPSHARP_OUT_DIR})
file(MAKE_DIRECTORY ${CPPSHARP_CODE_DIR})
file(MAKE_DIRECTORY ${CPPSHARP_PROJECT_DIR})
file(MAKE_DIRECTORY ${CPPSHARP_NUGET_DIR})

find_program(DOTNET_EXECUTABLE dotnet)
if(NOT DOTNET_EXECUTABLE)
	message(FATAL_ERROR "dotnet executable not found. Please install .NET SDK.")
endif()

add_custom_target(DenOfIz-CppSharpGenerate
	COMMAND ${DOTNET_EXECUTABLE} run --project "${CPPSHARP_GENERATOR_DIR}/DenOfIzGraphics.CppSharp/DenOfIzGraphics.CppSharp.csproj"
	WORKING_DIRECTORY ${CPPSHARP_GENERATOR_DIR}/DenOfIzGraphics.CppSharp
	COMMENT "Generating CppSharp C# bindings..."
	VERBATIM
)

if(NOT TARGET install)
	message(WARNING "CppSharp bindings require DenOfIzGraphics to be installed first. Run 'cmake --install' before generating bindings.")
endif()

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
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPPSHARP_NUGET_DIR}/lib/netstandard2.0
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPPSHARP_NUGET_DIR}/build
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPPSHARP_NUGET_DIR}/src
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CPPSHARP_NUGET_DIR}/src_gen
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CPPSHARP_CODE_DIR}/DenOfIzGraphics.cs ${CPPSHARP_NUGET_DIR}/src_gen/
	COMMAND ${CMAKE_COMMAND} -E copy ${CPPSHARP_CODE_DIR}/Std.cs ${CPPSHARP_NUGET_DIR}/src_gen/
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/DenOfIzGraphics.CppSharp.nuspec ${CPPSHARP_NUGET_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/DenOfIzGraphics.CppSharp.csproj ${CPPSHARP_NUGET_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/build/DenOfIzGraphics.CppSharp.targets ${CPPSHARP_NUGET_DIR}/build/
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/src/DenOfIzRuntime.cs ${CPPSHARP_NUGET_DIR}/src/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/src/NativeLibraryLoader.cs ${CPPSHARP_NUGET_DIR}/src/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/NuGet/src/Example.cs ${CPPSHARP_NUGET_DIR}/src/
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_INSTALL_PREFIX}/bin/${LIB_PREFIX}DenOfIzGraphics${LIB_EXTENSION} ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
	COMMENT "Preparing CppSharp NuGet package structure..."
)

if(WIN32)
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/bin/dxcompiler.dll ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/bin/dxil.dll ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/bin/metalirconverter.dll ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
	)
elseif(APPLE)
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libdxcompiler.dylib ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libmetalirconverter.dylib ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
	)
else()
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libdxcompiler.so ${CPPSHARP_NUGET_DIR}/runtimes/${PLATFORM_DIR}/native/
	)
endif()

add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
	COMMAND ${DOTNET_EXECUTABLE} build ${CPPSHARP_NUGET_DIR}/DenOfIzGraphics.CppSharp.csproj -c Release
	WORKING_DIRECTORY ${CPPSHARP_NUGET_DIR}
	COMMENT "Building CppSharp managed assembly..."
)

find_program(NUGET_EXECUTABLE nuget)
if(NUGET_EXECUTABLE)
	add_custom_command(TARGET DenOfIz-CppSharpNuGet POST_BUILD
		COMMAND ${NUGET_EXECUTABLE} pack ${CPPSHARP_NUGET_DIR}/DenOfIzGraphics.CppSharp.nuspec -OutputDirectory ${CPPSHARP_PROJECT_DIR}
		WORKING_DIRECTORY ${CPPSHARP_NUGET_DIR}
		COMMENT "Creating CppSharp NuGet package..."
	)
else()
	message(WARNING "NuGet executable not found. The NuGet package will not be created automatically.")
endif()