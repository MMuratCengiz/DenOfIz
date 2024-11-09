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

#include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderRecordLayout.h>
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

    /// <summary>
    /// This is used to bind data to raytracing shaders. With this structure you specify a register space, a stage and an entry name which will be then created as a root structure
    /// Any data you bind to this register space will be considered input for the specific ShaderStage.
    /// Note there is no difference between the hit shaders as normally they are all exported together.
    /// </summary>
    struct DZ_API ShaderRecordBindingDesc
    {
        uint32_t      RegisterSpace;
        ShaderStage   Stage;
        InteropString EntryName;
    };
    template class DZ_API InteropArray<ShaderRecordBindingDesc>;

    struct DZ_API ShaderProgramDesc
    {
        TargetIL                              TargetIL;
        InteropArray<ShaderDesc>              Shaders;
        bool                                  EnableCaching = true;
        InteropArray<ShaderRecordBindingDesc> ShaderRecordBindings;
    };
    template class DZ_API InteropArray<ShaderDesc>;

    struct DZ_API ShaderReflectDesc
    {
        InputLayoutDesc        InputLayout;
        RootSignatureDesc      RootSignature;
        ShaderRecordLayoutDesc ShaderRecordLayout;
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

    // State data for reflection processing during `ShaderProgram::Reflect( );`
    struct ReflectionState
    {
        RootSignatureDesc              *RootSignatureDesc;
        InputLayoutDesc                *InputLayoutDesc;
        ShaderRecordLayoutDesc         *ShaderRecordLayout;
        CompiledShader                 *CompiledShader;
        ID3D12ShaderReflection         *ShaderReflection;
        ID3D12LibraryReflection        *LibraryReflection;
        ID3D12FunctionReflection       *FunctionReflection;
        std::unordered_set<std::string> ProcessedFiles;
        // For metal:
        std::vector<uint32_t> *DescriptorTableLocations;
#ifdef BUILD_METAL
        IRShaderReflection             *IRReflection;
        std::vector<IRResourceLocation> MetalResourceLocations;
#endif
    };

    class ShaderProgram
    {
    private:
        typedef const std::function<void( D3D12_SHADER_INPUT_BIND_DESC &, int )> ReflectionCallback;

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
        void                                FillReflectionData( ReflectionState &state, ReflectionDesc &reflectionDesc, int resourceIndex ) const;
        void                    InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const;
        ID3D12ShaderReflection *ShaderReflection( const CompiledShader *compiledShader ) const;
        void                    ReflectShader( ReflectionState &state ) const;
        void                    ReflectLibrary( ReflectionState &state ) const;
        void                    ProcessBoundResource( ReflectionState &state, D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, int resourceIndex ) const;
        // Returns true if the bound resource is found(and an update is performed), false otherwise
        // Adds additional stages if existing stages are found
        bool UpdateBoundResourceStage( ReflectionState &state, D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc ) const;
#ifdef BUILD_METAL
        void IterateBoundResources( CompiledShader *shader, ReflectionState &state, ReflectionCallback &callback ) const;
        void ProduceMSL( );
#endif
    };

} // namespace DenOfIz
