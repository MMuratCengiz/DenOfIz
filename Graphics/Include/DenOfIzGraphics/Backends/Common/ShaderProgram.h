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

#ifndef _WIN32
#define interface struct
#endif

#include "directx/d3d12shader.h"

namespace DenOfIz
{

    struct DZ_API ShaderDesc
    {
        ShaderStage                 Stage;
        InteropString               Path;
        InteropArray<InteropString> Defines;
        InteropString               EntryPoint = "main";
    };

    struct DZ_API ShaderProgramDesc
    {
        TargetIL                 TargetIL;
        InteropArray<ShaderDesc> Shaders;
    };
    template class DZ_API InteropArray<ShaderDesc>;

    struct DZ_API ShaderReflectDesc
    {
        InputLayoutDesc   InputLayout;
        RootSignatureDesc RootSignature;
    };

#ifdef BUILD_METAL
    struct MetalDescriptorOffsets
    {
        // -1 is used for debugging purposes to show that no descriptor table exists in this root signature of that type
        int                                    CbvSrvUavOffset = -1;
        int                                    SamplerOffset   = -1;
        std::unordered_map<uint32_t, uint32_t> UniqueTLABIndex{ };
    };
#endif

    class ShaderProgram
    {
    private:
        std::vector<std::unique_ptr<CompiledShader>> m_compiledShaders;
        ShaderProgramDesc                            m_desc;
#ifdef BUILD_METAL
        std::vector<MetalDescriptorOffsets> m_metalDescriptorOffsets;
#endif
    public:
        DZ_API explicit ShaderProgram( ShaderProgramDesc desc );
        [[nodiscard]] DZ_API InteropArray<CompiledShader *> GetCompiledShaders( ) const;
        [[nodiscard]] DZ_API ShaderReflectDesc              Reflect( ) const;
        DZ_API ~ShaderProgram( );

    private:
        [[nodiscard]] const ShaderCompiler &ShaderCompilerInstance( ) const;
        void                                Compile( );
        void                                FillReflectionData( ID3D12ShaderReflection *shaderReflection, ReflectionDesc &reflectionDesc, int resourceIndex ) const;
        void                    InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const;
        ID3D12ShaderReflection *ShaderReflection( const CompiledShader *compiledShader ) const;
#ifdef BUILD_METAL
        void ProduceMSL( );
#endif
    };

} // namespace DenOfIz
