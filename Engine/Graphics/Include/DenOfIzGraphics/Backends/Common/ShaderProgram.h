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

#pragma once

#include "DenOfIzGraphics/Backends/Interface/IInputLayout.h"
#include "DenOfIzGraphics/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/IShader.h"
#include "ShaderCompiler.h"
#include "directx/d3d12shader.h"

namespace DenOfIz
{

    struct ShaderDesc
    {
        ShaderStage              Stage;
        std::string              Path;
        std::vector<std::string> Defines;
        std::string              EntryPoint = "main";
    };

    struct ShaderProgramDesc
    {
        TargetIL                TargetIL;
        std::vector<ShaderDesc> Shaders;
    };

    struct ShaderReflectDesc
    {
        InputLayoutDesc   InputLayout;
        RootSignatureDesc RootSignature;
    };

    class ShaderProgram
    {
        friend class GraphicsApi;

    private:
        std::vector<std::unique_ptr<CompiledShader>> m_compiledShaders;
        const ShaderProgramDesc                     &m_desc;

        ShaderProgram( const ShaderProgramDesc &desc );

    public:
        const std::vector<CompiledShader *> GetCompiledShaders( ) const;
        ShaderReflectDesc                   Reflect( ) const;

    private:
        const ShaderCompiler &ShaderCompilerInstance( ) const;
        void                  Compile( );
        void                  InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const;
        void                  ProcessRootSignature( ID3D12ShaderReflection *shaderReflection, RootSignatureDesc &rootSignatureDesc, const D3D12_SHADER_DESC &shaderDesc ) const;
    };

} // namespace DenOfIz
