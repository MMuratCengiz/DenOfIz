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
#include "DenOfIzGraphics/Backends/Common/GfxGlobal.h"

using namespace DenOfIz;

void ShaderProgram::AddShader(const ShaderInfo &shaderInfo) { m_shaders.push_back(shaderInfo); }

void ShaderProgram::Compile()
{
    ShaderCompiler compiler = GfxGlobal::GetInstance()->GetShaderCompiler();

    for ( auto &shader : m_shaders )
    {
        CompileOptions options = {};
        options.Defines = shader.Defines;
        options.EntryPoint = shader.EntryPoint;
        options.Stage = shader.Stage;

#if defined(WIN32)
        if ( GfxGlobal::GetInstance()->GetAPIPreference().Windows == APIPreferenceWindows::DirectX12 )
        {
            options.TargetIL = TargetIL::DXIL;
        }
        else
        {
            options.TargetIL = TargetIL::SPIRV;
        }
#elif defined(__APPLE__)
        options.TargetIL = TargetIL::MSL;
#elif defined(__linux__)
        options.TargetIL = TargetIL::SPIRV;
#endif

        CComPtr<IDxcBlob> data = compiler.CompileHLSL(shader.Path, options);
        m_compiledShaders.push_back({ .Stage = shader.Stage, .Data = data });
    }
}
