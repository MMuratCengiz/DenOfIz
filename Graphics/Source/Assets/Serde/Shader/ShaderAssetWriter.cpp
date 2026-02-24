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

#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetWriterHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <cstring>

#define SHADER_ASSET_WRITER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ShaderAssetWriter, handle )

namespace DenOfIz
{
    class ShaderAssetWriter
    {
    public:
        DenOfIz_ShaderAsset  m_shaderAsset = DENOFIZ_NULL_HANDLE;
        DenOfIz_BinaryWriter m_writer;
        uint64_t             m_streamStartOffset = 0;
        bool                 m_finalized         = false;

        explicit ShaderAssetWriter( const DenOfIz_ShaderAssetWriterDesc &desc );
        ~ShaderAssetWriter( );

        void Write( DenOfIz_ShaderAsset shaderAsset );
        void End( );

        static DenOfIz_ShaderAsset CreateFromCompiledShader( const DenOfIz_CompiledShader *compiledShader );
        static size_t              NumRequiredArenaBytes( const DenOfIz_CompiledShader *compiledShader );

    private:
        void WriteHeader( uint32_t totalNumBytes ) const;
        void WriteInputLayout( const DenOfIz_InputLayoutDesc &inputLayout ) const;
        void WriteBindGroupLayouts( const DenOfIz_BindGroupLayoutDescArray &bindGroupLayouts ) const;
        void WriteBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &bindGroupLayout ) const;
        void WriteRootConstants( const DenOfIz_RootConstantBindingDescArray &rootConstants ) const;
        void WriteLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &localDesc ) const;
        void WriteBinding( const DenOfIz_BindingDesc &binding ) const;
        void WriteLocalResourceBinding( const DenOfIz_LocalResourceBindingDesc &resourceBinding ) const;
    };
} // namespace DenOfIz

using namespace DenOfIz;

ShaderAssetWriter::ShaderAssetWriter( const DenOfIz_ShaderAssetWriterDesc &desc ) : m_writer( desc.Writer )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_writer ) )
    {
        spdlog::critical( "BinaryWriter cannot be null for ShaderAssetWriter" );
    }
}

ShaderAssetWriter::~ShaderAssetWriter( ) = default;

void ShaderAssetWriter::Write( DenOfIz_ShaderAsset shaderAsset )
{
    if ( m_finalized )
    {
        spdlog::error( "Cannot write to a finalized asset writer" );
        return;
    }

    m_shaderAsset       = shaderAsset;
    m_streamStartOffset = DenOfIz_BinaryWriter_Position( m_writer );
    WriteHeader( 0 );

    const uint32_t numStages = static_cast<uint32_t>( DenOfIz_ShaderAsset_NumStages( shaderAsset ) );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numStages );

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        const DenOfIz_ShaderStageAsset *stage = DenOfIz_ShaderAsset_GetStage( shaderAsset, i );

        DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( stage->Stage ) );
        DenOfIz_BinaryWriter_WriteString( m_writer, stage->EntryPoint );

        DenOfIz_BinaryWriter_WriteUInt64( m_writer, stage->DXIL.NumElements );
        DenOfIz_BinaryWriter_WriteBytes( m_writer, { stage->DXIL.Elements, stage->DXIL.NumElements } );

        DenOfIz_BinaryWriter_WriteUInt64( m_writer, stage->MSL.NumElements );
        DenOfIz_BinaryWriter_WriteBytes( m_writer, { stage->MSL.Elements, stage->MSL.NumElements } );

        DenOfIz_BinaryWriter_WriteUInt64( m_writer, stage->SPIRV.NumElements );
        DenOfIz_BinaryWriter_WriteBytes( m_writer, { stage->SPIRV.Elements, stage->SPIRV.NumElements } );

        DenOfIz_BinaryWriter_WriteUInt64( m_writer, stage->WGSL.NumElements );
        DenOfIz_BinaryWriter_WriteBytes( m_writer, { stage->WGSL.Elements, stage->WGSL.NumElements } );

        DenOfIz_BinaryWriter_WriteUInt64( m_writer, stage->Reflection.NumElements );
        DenOfIz_BinaryWriter_WriteBytes( m_writer, { stage->Reflection.Elements, stage->Reflection.NumElements } );

        const uint32_t numLocalBindings = stage->RayTracing.LocalBindings.NumElements;
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, numLocalBindings );

        for ( uint32_t j = 0; j < numLocalBindings; ++j )
        {
            const DenOfIz_ResourceBindingSlot &binding = stage->RayTracing.LocalBindings.Elements[ j ];
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, binding.RegisterSpace );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, binding.Binding );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( binding.Type ) );
        }

        DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( stage->RayTracing.HitGroupType ) );
    }

    WriteBindGroupLayouts( DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->BindGroupLayouts );
    WriteRootConstants( DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->RootConstants );
    WriteInputLayout( DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->InputLayout );

    const uint32_t numLocalRootSigs = DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->LocalRootSignatures.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numLocalRootSigs );
    for ( uint32_t i = 0; i < numLocalRootSigs; ++i )
    {
        const DenOfIz_LocalRootSignatureDesc &localDesc = DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->LocalRootSignatures.Elements[ i ];
        WriteLocalRootSignature( localDesc );
    }

    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_ShaderAsset_GetRayTracing( shaderAsset )->MaxNumPayloadBytes );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_ShaderAsset_GetRayTracing( shaderAsset )->MaxNumAttributeBytes );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_ShaderAsset_GetRayTracing( shaderAsset )->MaxRecursionDepth );
    const uint32_t numThreadGroups = DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->ThreadGroups.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numThreadGroups );
    for ( uint32_t i = 0; i < numThreadGroups; ++i )
    {
        const DenOfIz_ThreadGroupDesc &threadGroup = DenOfIz_ShaderAsset_GetReflectDesc( shaderAsset )->ThreadGroups.Elements[ i ];
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, threadGroup.X );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, threadGroup.Y );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, threadGroup.Z );
    }
}

void ShaderAssetWriter::WriteHeader( const uint32_t totalNumBytes ) const
{
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, DenOfIz_ShaderAsset_Magic( m_shaderAsset ) );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_ShaderAsset_Version( m_shaderAsset ) );
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, totalNumBytes );
    DenOfIz_BinaryWriter_WriteString( m_writer, DenOfIz_ShaderAsset_Path( m_shaderAsset ) );
}

void ShaderAssetWriter::WriteInputLayout( const DenOfIz_InputLayoutDesc &inputLayout ) const
{
    const uint32_t numInputGroups = inputLayout.InputGroups.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numInputGroups );

    for ( uint32_t i = 0; i < numInputGroups; ++i )
    {
        const DenOfIz_InputGroupDesc &inputGroup = inputLayout.InputGroups.Elements[ i ];
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, inputGroup.Elements.NumElements );

        for ( uint32_t j = 0; j < inputGroup.Elements.NumElements; ++j )
        {
            const DenOfIz_InputLayoutElementDesc &element = inputGroup.Elements.Elements[ j ];

            DenOfIz_BinaryWriter_WriteString( m_writer, element.Semantic );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, element.SemanticIndex );
            DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( element.Format ) );
        }
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, static_cast<uint32_t>( inputGroup.StepRate ) );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, inputGroup.Stride );
    }
}

void ShaderAssetWriter::WriteBindGroupLayouts( const DenOfIz_BindGroupLayoutDescArray &bindGroupLayouts ) const
{
    const uint32_t numBindGroupLayouts = bindGroupLayouts.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numBindGroupLayouts );

    for ( uint32_t i = 0; i < numBindGroupLayouts; ++i )
    {
        const DenOfIz_BindGroupLayoutDesc &layout = bindGroupLayouts.Elements[ i ];
        WriteBindGroupLayout( layout );
    }
}

void ShaderAssetWriter::WriteBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &bindGroupLayout ) const
{
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, bindGroupLayout.RegisterSpace );

    const uint32_t numBindings = bindGroupLayout.Bindings.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numBindings );

    for ( uint32_t i = 0; i < numBindings; ++i )
    {
        const DenOfIz_BindingDesc &binding = bindGroupLayout.Bindings.Elements[ i ];
        WriteBinding( binding );
    }
}

void ShaderAssetWriter::WriteRootConstants( const DenOfIz_RootConstantBindingDescArray &rootConstants ) const
{
    const uint32_t numRootConstants = rootConstants.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numRootConstants );

    for ( uint32_t i = 0; i < numRootConstants; ++i )
    {
        const DenOfIz_RootConstantBindingDesc &constant = rootConstants.Elements[ i ];
        DenOfIz_BinaryWriter_WriteString( m_writer, constant.Name );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, constant.Binding );
        DenOfIz_BinaryWriter_WriteUInt64( m_writer, constant.NumBytes );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, constant.Stages );
    }
}

void ShaderAssetWriter::WriteLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &localDesc ) const
{
    const uint32_t numBindings = localDesc.ResourceBindings.NumElements;
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numBindings );

    for ( uint32_t j = 0; j < numBindings; ++j )
    {
        const DenOfIz_LocalResourceBindingDesc &bindingDesc = localDesc.ResourceBindings.Elements[ j ];
        WriteLocalResourceBinding( bindingDesc );
    }
}

void ShaderAssetWriter::WriteBinding( const DenOfIz_BindingDesc &binding ) const
{
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, binding.Binding );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, binding.Descriptor );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, binding.Stages );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, binding.ArraySize );
    DenOfIz_BinaryWriter_WriteByte( m_writer, binding.IsBindless ? 1 : 0 );
}

void ShaderAssetWriter::WriteLocalResourceBinding( const DenOfIz_LocalResourceBindingDesc &resourceBinding ) const
{
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, resourceBinding.Binding );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, resourceBinding.RegisterSpace );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, resourceBinding.Descriptor );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, resourceBinding.Stages );
    DenOfIz_BinaryWriter_WriteInt32( m_writer, resourceBinding.ArraySize );
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, resourceBinding.NumBytes );
}

void ShaderAssetWriter::End( )
{
    if ( m_finalized )
    {
        return;
    }

    const uint64_t currentPos = DenOfIz_BinaryWriter_Position( m_writer );
    const uint64_t numBytes   = currentPos - m_streamStartOffset;

    DenOfIz_BinaryWriter_Seek( m_writer, m_streamStartOffset );
    WriteHeader( numBytes );
    DenOfIz_BinaryWriter_Seek( m_writer, currentPos );

    m_finalized = true;
}

DenOfIz_ShaderAsset ShaderAssetWriter::CreateFromCompiledShader( const DenOfIz_CompiledShader *compiledShader )
{
    if ( compiledShader == NULL )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    DenOfIz_ShaderAsset shaderAsset                                        = DenOfIz_ShaderAsset_Create( );
    DenOfIz_ShaderAsset_GetRayTracing( shaderAsset )->MaxNumPayloadBytes   = compiledShader->RayTracing.MaxNumPayloadBytes;
    DenOfIz_ShaderAsset_GetRayTracing( shaderAsset )->MaxNumAttributeBytes = compiledShader->RayTracing.MaxNumAttributeBytes;
    DenOfIz_ShaderAsset_GetRayTracing( shaderAsset )->MaxRecursionDepth    = compiledShader->RayTracing.MaxRecursionDepth;
    DenOfIz_ShaderAsset_SetReflectDesc( shaderAsset, &compiledShader->ReflectDesc );

    const uint32_t numStages = compiledShader->Stages.NumElements;
    DenOfIz_ShaderAsset_ReserveStages( shaderAsset, numStages );

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        const DenOfIz_CompiledShaderStage *compiledStage = compiledShader->Stages.Elements[ i ];
        DenOfIz_ShaderStageAsset          *stageAsset    = DenOfIz_ShaderAsset_AddStage( shaderAsset );

        stageAsset->Stage = compiledStage->Stage;
        DenOfIz_ShaderAsset_SetStageEntryPoint( shaderAsset, i, compiledStage->EntryPoint );
        stageAsset->RayTracing = compiledStage->RayTracing;

        if ( compiledStage->DXIL.NumElements > 0 )
        {
            stageAsset->DXIL.NumElements = compiledStage->DXIL.NumElements;
            stageAsset->DXIL.Elements    = new Byte[ compiledStage->DXIL.NumElements ];
            memcpy( stageAsset->DXIL.Elements, compiledStage->DXIL.Elements, compiledStage->DXIL.NumElements );
        }

        if ( compiledStage->MSL.NumElements > 0 )
        {
            stageAsset->MSL.NumElements = compiledStage->MSL.NumElements;
            stageAsset->MSL.Elements    = new Byte[ compiledStage->MSL.NumElements ];
            memcpy( stageAsset->MSL.Elements, compiledStage->MSL.Elements, compiledStage->MSL.NumElements );
        }

        if ( compiledStage->SPIRV.NumElements > 0 )
        {
            stageAsset->SPIRV.NumElements = compiledStage->SPIRV.NumElements;
            stageAsset->SPIRV.Elements    = new Byte[ compiledStage->SPIRV.NumElements ];
            memcpy( stageAsset->SPIRV.Elements, compiledStage->SPIRV.Elements, compiledStage->SPIRV.NumElements );
        }

        if ( compiledStage->WGSL.NumElements > 0 )
        {
            stageAsset->WGSL.NumElements = compiledStage->WGSL.NumElements;
            stageAsset->WGSL.Elements    = new Byte[ compiledStage->WGSL.NumElements ];
            memcpy( stageAsset->WGSL.Elements, compiledStage->WGSL.Elements, compiledStage->WGSL.NumElements );
        }

        if ( compiledStage->Reflection.NumElements > 0 )
        {
            stageAsset->Reflection.NumElements = compiledStage->Reflection.NumElements;
            stageAsset->Reflection.Elements    = new Byte[ compiledStage->Reflection.NumElements ];
            memcpy( stageAsset->Reflection.Elements, compiledStage->Reflection.Elements, compiledStage->Reflection.NumElements );
        }
    }
    return shaderAsset;
}

size_t ShaderAssetWriter::NumRequiredArenaBytes( const DenOfIz_CompiledShader *compiledShader )
{
    if ( compiledShader == NULL )
    {
        return 0;
    }

    size_t result = 0;
    for ( uint32_t i = 0; i < compiledShader->Stages.NumElements; ++i )
    {
        const DenOfIz_CompiledShaderStage *compiledStage = compiledShader->Stages.Elements[ i ];
        result += compiledStage->DXIL.NumElements + compiledStage->MSL.NumElements + compiledStage->SPIRV.NumElements + compiledStage->WGSL.NumElements +
                  compiledStage->Reflection.NumElements;
        result += compiledStage->RayTracing.LocalBindings.NumElements * sizeof( DenOfIz_ResourceBindingSlot );
        result += sizeof( uint32_t );
    }
    result += 4096;
    return (size_t)( result * 1.2 );
}

extern "C"
{

    DenOfIz_ShaderAssetWriter DenOfIz_ShaderAssetWriter_Create( const DenOfIz_ShaderAssetWriterDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *writer = new ShaderAssetWriter( *desc );
        return DENOFIZ_TO_HANDLE( writer );
    }

    void DenOfIz_ShaderAssetWriter_Destroy( DenOfIz_ShaderAssetWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        delete SHADER_ASSET_WRITER_IMPL( writer );
    }

    void DenOfIz_ShaderAssetWriter_Write( DenOfIz_ShaderAssetWriter writer, DenOfIz_ShaderAsset shaderAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) || !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
        {
            return;
        }
        SHADER_ASSET_WRITER_IMPL( writer )->Write( shaderAsset );
    }

    void DenOfIz_ShaderAssetWriter_End( DenOfIz_ShaderAssetWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        SHADER_ASSET_WRITER_IMPL( writer )->End( );
    }

    DenOfIz_ShaderAsset DenOfIz_ShaderAssetWriter_CreateFromCompiledShader( const DenOfIz_CompiledShader *compiledShader )
    {
        return ShaderAssetWriter::CreateFromCompiledShader( compiledShader );
    }

    size_t DenOfIz_ShaderAssetWriter_NumRequiredArenaBytes( const DenOfIz_CompiledShader *compiledShader )
    {
        return ShaderAssetWriter::NumRequiredArenaBytes( compiledShader );
    }
}
