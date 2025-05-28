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

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <functional>
#include <unordered_map>
#include "DxcEnumConverter.h"
#include "ShaderCompiler.h"

namespace DenOfIz
{
    struct MetalDescriptorOffsets
    {
        // -1 is used for debugging purposes to show that no descriptor table exists in this root signature of that type
        int                                    CbvSrvUavOffset      = -1;
        int                                    SamplerOffset        = -1;
        int                                    LocalCbvSrvUavOffset = -1;
        int                                    LocalSamplerOffset   = -1;
        std::unordered_map<uint32_t, uint32_t> UniqueTLABIndex{ };
    };

    // This is used to order the root parameters in the root signature as metal top level argument buffer expects them in the same order
    // Binding goes from 0 till max register space.
    struct RegisterSpaceRange
    {
        std::vector<IRRootConstants>     RootConstants;
        std::vector<IRRootDescriptor>    RootArguments;
        std::vector<IRRootParameterType> RootArgumentTypes;
        std::vector<IRDescriptorRange1>  CbvSrvUavRanges;
        std::vector<IRDescriptorRange1>  SamplerRanges;
        IRShaderVisibility               ShaderVisibility;
        bool                             HasBindlessResources = false;
    };

    typedef const std::function<void( D3D12_SHADER_INPUT_BIND_DESC &, int )> ReflectionCallback;

    struct CompileMslDesc
    {
        IRRootSignature     *RootSignature;
        IRRootSignature     *LocalRootSignature;
        ShaderRayTracingDesc RayTracing;
    };

    struct DZ_API DxilToMslDesc
    {
        InteropArray<ShaderStageDesc>       Shaders;
        InteropArray<CompiledShaderStage *> DXILShaders;
        ShaderRayTracingDesc                RayTracing;
    };

    class DxilToMsl
    {
        ShaderCompiler m_compiler;

        ID3D12ShaderReflection   *m_shaderReflection   = nullptr;
        ID3D12LibraryReflection  *m_libraryReflection  = nullptr;
        ID3D12FunctionReflection *m_functionReflection = nullptr;

        IRCompiler *m_irCompiler = nullptr;

    public:
        DZ_API ~DxilToMsl( );
        DZ_API InteropArray<InteropArray<Byte>> Convert( const DxilToMslDesc &desc );

    private:
        InteropArray<Byte> Compile( const CompileDesc &compileDesc, const InteropArray<Byte> &dxil, const CompileMslDesc &compileMslDesc,
                                    const RayTracingShaderDesc &rayTracingShaderDesc ) const;

        IRRootSignature *CreateRootSignature( std::vector<RegisterSpaceRange> &registerSpaceRanges, bool isLocal ) const;
        void             IterateBoundResources( CompiledShaderStage *shader, ReflectionCallback &callback );
        void             DxcCheckResult( HRESULT hr ) const;
    };
} // namespace DenOfIz
