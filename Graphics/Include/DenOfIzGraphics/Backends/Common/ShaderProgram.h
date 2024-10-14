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

    struct ShaderDesc
    {
        ShaderStage   Stage;
        std::string   Path;
        ShaderDefines Defines;
        std::string   EntryPoint = "main";
    };

#define DZ_MAX_SHADER_DESCS 16
    struct ShaderDescs
    {
        size_t     NumElements = 0;
        ShaderDesc Array[ DZ_MAX_SHADER_DESCS ];

        void SetElement( size_t index, const ShaderDesc &value )
        {
            Array[ index ] = value;
        }
        const ShaderDesc &GetElement( size_t index )
        {
            return Array[ index ];
        }
    };

    struct ShaderProgramDesc
    {
        TargetIL    TargetIL;
        ShaderDescs Shaders;
    };

    struct ShaderReflectDesc
    {
        InputLayoutDesc   InputLayout;
        RootSignatureDesc RootSignature;
    };

#define DZ_MAX_COMPILED_SHADERS 8
    struct CompiledShaders
    {
        size_t          NumElements = 0;
        CompiledShader *Array[ DZ_MAX_COMPILED_SHADERS ];

        void SetElement( size_t index, CompiledShader *&value )
        {
            Array[ index ] = value;
        }
        const CompiledShader *GetElement( size_t index )
        {
            return Array[ index ];
        }
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
        friend class GraphicsApi;

        std::vector<std::unique_ptr<CompiledShader>> m_compiledShaders;
        ShaderProgramDesc                            m_desc;
#ifdef BUILD_METAL
        std::vector<MetalDescriptorOffsets> m_metalDescriptorOffsets;

#endif

        explicit ShaderProgram( ShaderProgramDesc desc );

    public:
        [[nodiscard]] CompiledShaders   GetCompiledShaders( ) const;
        [[nodiscard]] ShaderReflectDesc Reflect( ) const;
        ~ShaderProgram( );

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
