/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <filesystem>
#include <iostream>

#include <DenOfIzBindings/Core/DZIndex.h>
#include <DenOfIzBindings/Core/ProjectContext.h>
#include <DenOfIzBindings/Emitters/CSharpEmitter.h>

#include "DenOfIzBindings/Core/Dzir.h"

using namespace DenOfIz;

int main( )
{
    ProjectContextDesc desc{ };
    desc.IncludeRoot = DENOFIZ_GRAPHICS_INCLUDE_ROOT;
    ProjectContext              projectContext( desc );
    DZIndex                     index( &projectContext, "DenOfIzGraphics/DenOfIzGraphics_Ext.h" );
    DzirLibrary                 dzir( &index );
    const std::filesystem::path repositoryRoot( DENOFIZ_BINDINGS_REPOSITORY_ROOT );

    CSharpEmitterConfig config{ };
    config.Namespace   = "DenOfIz";
    config.LibraryName = "DenOfIzGraphics";

    CSharpEmitter emitter( &dzir, config );
    emitter.SetOutputDirectory( repositoryRoot / "Bindings" / "CSharp" / "Generated" );
    emitter.Emit( );

    std::cout << "C# bindings generated successfully." << std::endl;
    return 0;
}
