/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    config.Namespace        = "DenOfIz";
    config.LibraryName      = "DenOfIzGraphics";
    config.GenerateWrappers = false;

    CSharpEmitter emitter( &dzir, config );
    emitter.SetOutputDirectory( repositoryRoot / "Bindings" / "CSharp" / "Generated" );
    emitter.Emit( );

    std::cout << "C# bindings generated successfully." << std::endl;
    return 0;
}
