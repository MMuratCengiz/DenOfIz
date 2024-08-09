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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>

using namespace DenOfIz;

ShaderProgram::ShaderProgram( const ShaderProgramDesc &desc ) : m_desc( desc )
{
    Compile( );
}

void ShaderProgram::Compile( )
{
    const ShaderCompiler& compiler = GetShaderCompiler( );

    for ( auto &shader : m_desc.Shaders )
    {
        CompileOptions options = { };
        options.Defines        = shader.Defines;
        options.EntryPoint     = shader.EntryPoint;
        options.Stage          = shader.Stage;
        options.TargetIL       = m_desc.TargetIL;

        IDxcBlob *blob = compiler.CompileHLSL( shader.Path, options );
        m_compiledShaders.push_back( { .Stage = shader.Stage, .Blob = blob, .EntryPoint = shader.EntryPoint } );
    }
}

ShaderProgram::~ShaderProgram( )
{
    for ( auto &shader : m_compiledShaders )
    {
        if ( shader.Blob )
        {
            shader.Blob->Release( );
            shader.Blob = nullptr;
        }
    }
}

const ShaderCompiler &ShaderProgram::GetShaderCompiler( )
{
    static ShaderCompiler compiler;
    return compiler;
}
