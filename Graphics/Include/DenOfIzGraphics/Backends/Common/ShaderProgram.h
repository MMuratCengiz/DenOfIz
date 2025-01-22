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

#include <DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h>
#include <DenOfIzGraphics/Assets/Shaders/ShaderReflectionHelper.h>
#include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>

#ifndef _WIN32
#define interface struct
#endif

#include "directx/d3d12shader.h"

namespace DenOfIz
{

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
        InteropArray<ShaderStageDesc> ShaderStages;
        ShaderRayTracingDesc          RayTracing;
    };

    // State data for reflection processing during `ShaderProgram::Reflect( );`
    struct ReflectionState
    {
        RootSignatureDesc        *RootSignatureDesc;
        InputLayoutDesc          *InputLayoutDesc;
        LocalRootSignatureDesc   *LocalRootSignature;
        ShaderStageDesc const    *ShaderDesc;
        CompiledShaderStage      *CompiledShader;
        ID3D12ShaderReflection   *ShaderReflection;
        ID3D12LibraryReflection  *LibraryReflection;
        ID3D12FunctionReflection *FunctionReflection;
    };

    struct DZ_API CompiledShader
    {
        InteropArray<CompiledShaderStage *> Stages;
        ShaderReflectDesc                   ReflectDesc;
        ShaderRayTracingDesc                RayTracing;
    };

    class ShaderProgram
    {
        ShaderCompiler                                    m_compiler;
        std::vector<std::unique_ptr<CompiledShaderStage>> m_compiledShaders;
        std::vector<ShaderStageDesc>                      m_shaderDescs; // Index matched with m_compiledShaders
        ShaderProgramDesc                                 m_desc;

    public:
        DZ_API explicit ShaderProgram( ShaderProgramDesc desc );
        [[nodiscard]] DZ_API InteropArray<CompiledShaderStage *> CompiledShaders( ) const;
        [[nodiscard]] DZ_API ShaderReflectDesc                   Reflect( ) const;
        [[nodiscard]] DZ_API ShaderProgramDesc                   Desc( ) const;
        DZ_API ~ShaderProgram( );

    private:
        void Compile( );
        void InitInputLayout( ID3D12ShaderReflection *shaderReflection, InputLayoutDesc &inputLayoutDesc, const D3D12_SHADER_DESC &shaderDesc ) const;
        void ReflectShader( const ReflectionState &state ) const;
        void ReflectLibrary( ReflectionState &state ) const;
        void ProcessInputBindingDesc( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc, int resourceIndex ) const;
        bool UpdateBoundResourceStage( const ReflectionState &state, const D3D12_SHADER_INPUT_BIND_DESC &shaderInputBindDesc ) const;
    };

} // namespace DenOfIz
