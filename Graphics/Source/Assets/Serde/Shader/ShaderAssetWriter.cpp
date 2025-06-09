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
#include "DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

ShaderAssetWriter::ShaderAssetWriter( const ShaderAssetWriterDesc &desc ) : m_writer( desc.Writer )
{
    DZ_NOT_NULL( m_writer );
}

ShaderAssetWriter::~ShaderAssetWriter( ) = default;

void ShaderAssetWriter::Write( const ShaderAsset &shaderAsset )
{
    if ( m_finalized )
    {
        spdlog::error( "Cannot write to a finalized asset writer" );
        return;
    }

    m_shaderAsset       = shaderAsset;
    m_streamStartOffset = m_writer->Position( );
    WriteHeader( 0 );

    const uint32_t numStages = shaderAsset.Stages.NumElements;
    m_writer->WriteUInt32( numStages );

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        const ShaderStageAsset &stage = shaderAsset.Stages.Elements[ i ];

        m_writer->WriteUInt32( static_cast<uint32_t>( stage.Stage ) );
        m_writer->WriteString( stage.EntryPoint );

        m_writer->WriteUInt64( stage.DXIL.NumElements( ) );
        m_writer->WriteBytes( stage.DXIL );

        m_writer->WriteUInt64( stage.MSL.NumElements( ) );
        m_writer->WriteBytes( stage.MSL );

        m_writer->WriteUInt64( stage.SPIRV.NumElements( ) );
        m_writer->WriteBytes( stage.SPIRV );

        m_writer->WriteUInt64( stage.Reflection.NumElements( ) );
        m_writer->WriteBytes( stage.Reflection );

        const uint32_t numLocalBindings = stage.RayTracing.LocalBindings.NumElements;
        m_writer->WriteUInt32( numLocalBindings );

        for ( uint32_t j = 0; j < numLocalBindings; ++j )
        {
            const ResourceBindingSlot &binding = stage.RayTracing.LocalBindings.Elements[ j ];
            m_writer->WriteUInt32( binding.RegisterSpace );
            m_writer->WriteUInt32( binding.Binding );
            m_writer->WriteUInt32( static_cast<uint32_t>( binding.Type ) );
        }

        m_writer->WriteUInt32( static_cast<uint32_t>( stage.RayTracing.HitGroupType ) );
    }

    WriteRootSignature( shaderAsset.ReflectDesc.RootSignature );
    WriteInputLayout( shaderAsset.ReflectDesc.InputLayout );

    const uint32_t numLocalRootSigs = shaderAsset.ReflectDesc.LocalRootSignatures.NumElements( );
    m_writer->WriteUInt32( numLocalRootSigs );
    for ( uint32_t i = 0; i < numLocalRootSigs; ++i )
    {
        const LocalRootSignatureDesc &localDesc = shaderAsset.ReflectDesc.LocalRootSignatures.GetElement( i );
        WriteLocalRootSignature( localDesc );
    }

    m_writer->WriteUInt32( shaderAsset.RayTracing.MaxNumPayloadBytes );
    m_writer->WriteUInt32( shaderAsset.RayTracing.MaxNumAttributeBytes );
    m_writer->WriteUInt32( shaderAsset.RayTracing.MaxRecursionDepth );

    AssetWriterHelpers::WriteProperties( m_writer, shaderAsset.UserProperties );
}

void ShaderAssetWriter::WriteHeader( const uint32_t totalNumBytes ) const
{
    m_writer->WriteUInt64( m_shaderAsset.Magic );
    m_writer->WriteUInt32( m_shaderAsset.Version );
    m_writer->WriteUInt64( totalNumBytes );
    m_writer->WriteString( m_shaderAsset.Uri.ToInteropString( ) );
}

void ShaderAssetWriter::WriteInputLayout( const InputLayoutDesc &inputLayout ) const
{
    const uint32_t numInputGroups = inputLayout.InputGroups.NumElements( );
    m_writer->WriteUInt32( numInputGroups );

    for ( uint32_t i = 0; i < numInputGroups; ++i )
    {
        const InputGroupDesc &inputGroup = inputLayout.InputGroups.GetElement( i );
        m_writer->WriteUInt32( inputGroup.Elements.NumElements( ) );

        for ( int j = 0; j < inputGroup.Elements.NumElements( ); ++j )
        {
            const InputLayoutElementDesc &element = inputGroup.Elements.GetElement( j );

            m_writer->WriteString( element.Semantic );
            m_writer->WriteUInt32( element.SemanticIndex );
            m_writer->WriteUInt32( static_cast<uint32_t>( element.Format ) );
        }
        m_writer->WriteUInt32( static_cast<uint32_t>( inputGroup.StepRate ) );
    }
}

void ShaderAssetWriter::WriteRootSignature( const RootSignatureDesc &rootSignature ) const
{
    const uint32_t numResourceBindings = rootSignature.ResourceBindings.NumElements( );
    m_writer->WriteUInt32( numResourceBindings );

    for ( uint32_t i = 0; i < numResourceBindings; ++i )
    {
        const ResourceBindingDesc &binding = rootSignature.ResourceBindings.GetElement( i );
        WriteResourceBinding( binding );
    }

    const uint32_t numStaticSamplers = rootSignature.StaticSamplers.NumElements( );
    m_writer->WriteUInt32( numStaticSamplers );
    for ( uint32_t i = 0; i < numStaticSamplers; ++i )
    {
        const StaticSamplerDesc &sampler = rootSignature.StaticSamplers.GetElement( i );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.MagFilter ) );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.MinFilter ) );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.AddressModeU ) );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.AddressModeV ) );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.AddressModeW ) );
        m_writer->WriteFloat( sampler.Sampler.MaxAnisotropy );
        m_writer->WriteUInt32( sampler.Sampler.MaxAnisotropy );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.CompareOp ) );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Sampler.MipmapMode ) );
        m_writer->WriteFloat( sampler.Sampler.MipLodBias );
        m_writer->WriteFloat( sampler.Sampler.MinLod );
        m_writer->WriteFloat( sampler.Sampler.MaxLod );
        m_writer->WriteString( sampler.Sampler.DebugName );

        m_writer->WriteString( sampler.Binding.Name );
        m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Binding.BindingType ) );
        m_writer->WriteUInt32( sampler.Binding.Binding );
        m_writer->WriteUInt32( sampler.Binding.RegisterSpace );
        m_writer->WriteUInt32( sampler.Binding.Descriptor );

        const uint32_t numStages = sampler.Binding.Stages.NumElements( );
        m_writer->WriteUInt32( numStages );
        for ( uint32_t j = 0; j < numStages; ++j )
        {
            m_writer->WriteUInt32( static_cast<uint32_t>( sampler.Binding.Stages.GetElement( j ) ) );
        }

        m_writer->WriteInt32( sampler.Binding.ArraySize );
        WriteResourceReflection( sampler.Binding.Reflection );
    }

    const uint32_t numRootConstants = rootSignature.RootConstants.NumElements( );
    m_writer->WriteUInt32( numRootConstants );

    for ( uint32_t i = 0; i < numRootConstants; ++i )
    {
        const RootConstantResourceBindingDesc &constant = rootSignature.RootConstants.GetElement( i );
        m_writer->WriteString( constant.Name );
        m_writer->WriteUInt32( constant.Binding );
        m_writer->WriteInt32( constant.NumBytes );

        const uint32_t numStages = constant.Stages.NumElements( );
        m_writer->WriteUInt32( numStages );
        for ( uint32_t j = 0; j < numStages; ++j )
        {
            m_writer->WriteUInt32( static_cast<uint32_t>( constant.Stages.GetElement( j ) ) );
        }
        WriteResourceReflection( constant.Reflection );
    }
}

void ShaderAssetWriter::WriteLocalRootSignature( const LocalRootSignatureDesc &localDesc ) const
{
    const uint32_t numBindings = localDesc.ResourceBindings.NumElements( );
    m_writer->WriteUInt32( numBindings );

    for ( uint32_t j = 0; j < numBindings; ++j )
    {
        const ResourceBindingDesc &bindingDesc = localDesc.ResourceBindings.GetElement( j );
        WriteResourceBinding( bindingDesc );
    }
}

void ShaderAssetWriter::WriteResourceBinding( const ResourceBindingDesc &resourceBinding ) const
{
    m_writer->WriteString( resourceBinding.Name );
    m_writer->WriteUInt32( static_cast<uint32_t>( resourceBinding.BindingType ) );
    m_writer->WriteUInt32( resourceBinding.Binding );
    m_writer->WriteUInt32( resourceBinding.RegisterSpace );
    m_writer->WriteUInt32( resourceBinding.Descriptor );

    const uint32_t numStages = resourceBinding.Stages.NumElements( );
    m_writer->WriteUInt32( numStages );
    for ( uint32_t j = 0; j < numStages; ++j )
    {
        m_writer->WriteUInt32( static_cast<uint32_t>( resourceBinding.Stages.GetElement( j ) ) );
    }

    m_writer->WriteInt32( resourceBinding.ArraySize );
    WriteResourceReflection( resourceBinding.Reflection );
}

void ShaderAssetWriter::WriteResourceReflection( const ReflectionDesc &reflection ) const
{
    m_writer->WriteString( reflection.Name );
    m_writer->WriteUInt32( static_cast<uint32_t>( reflection.Type ) );

    const uint32_t numFields = reflection.Fields.NumElements( );
    m_writer->WriteUInt32( numFields );

    for ( uint32_t i = 0; i < numFields; ++i )
    {
        const ReflectionResourceField &field = reflection.Fields.GetElement( i );
        m_writer->WriteString( field.Name );
        m_writer->WriteUInt32( static_cast<uint32_t>( field.Type ) );
        m_writer->WriteUInt32( field.NumColumns );
        m_writer->WriteUInt32( field.NumRows );
        m_writer->WriteUInt32( field.Elements );
        m_writer->WriteUInt32( field.Offset );
        m_writer->WriteUInt32( field.Level );
        m_writer->WriteUInt32( field.ParentIndex );
    }

    m_writer->WriteUInt64( reflection.NumBytes );
}

void ShaderAssetWriter::End( )
{
    if ( m_finalized )
    {
        return;
    }

    const uint64_t currentPos = m_writer->Position( );
    const uint64_t numBytes   = currentPos - m_streamStartOffset;

    m_writer->Seek( m_streamStartOffset );
    WriteHeader( numBytes );
    m_writer->Seek( currentPos );

    m_finalized = true;
}

ShaderAsset ShaderAssetWriter::CreateFromCompiledShader( const CompiledShader &compiledShader )
{
    ShaderAsset shaderAsset;

    shaderAsset.RayTracing.MaxNumPayloadBytes   = compiledShader.RayTracing.MaxNumPayloadBytes;
    shaderAsset.RayTracing.MaxNumAttributeBytes = compiledShader.RayTracing.MaxNumAttributeBytes;
    shaderAsset.RayTracing.MaxRecursionDepth    = compiledShader.RayTracing.MaxRecursionDepth;

    shaderAsset.ReflectDesc = compiledShader.ReflectDesc;

    const uint32_t numStages = compiledShader.Stages.NumElements;

    shaderAsset.Stages.NumElements = numStages;
    shaderAsset.Stages.Elements    = static_cast<ShaderStageAsset *>( std::malloc( numStages * sizeof( ShaderStageAsset ) ) );

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        const CompiledShaderStage *compiledStage = compiledShader.Stages.Elements[ i ];
        ShaderStageAsset          &stageAsset    = shaderAsset.Stages.Elements[ i ];

        stageAsset.Stage      = compiledStage->Stage;
        stageAsset.EntryPoint = compiledStage->EntryPoint;
        stageAsset.RayTracing = compiledStage->RayTracing;

        if ( compiledStage->DXIL.NumElements > 0 )
        {
            const void  *data = compiledStage->DXIL.Elements;
            const size_t size = compiledStage->DXIL.NumElements;
            stageAsset.DXIL.Resize( size );
            memcpy( stageAsset.DXIL.Data( ), data, size );
        }

        if ( compiledStage->MSL.NumElements > 0 )
        {
            const void  *data = compiledStage->MSL.Elements;
            const size_t size = compiledStage->MSL.NumElements;
            stageAsset.MSL.Resize( size );
            memcpy( stageAsset.MSL.Data( ), data, size );
        }

        if ( compiledStage->SPIRV.NumElements > 0 )
        {
            const void  *data = compiledStage->SPIRV.Elements;
            const size_t size = compiledStage->SPIRV.NumElements;
            stageAsset.SPIRV.Resize( size );
            memcpy( stageAsset.SPIRV.Data( ), data, size );
        }

        if ( compiledStage->Reflection.NumElements > 0 )
        {
            const void  *data = compiledStage->Reflection.Elements;
            const size_t size = compiledStage->Reflection.NumElements;
            stageAsset.Reflection.Resize( size );
            memcpy( stageAsset.Reflection.Data( ), data, size );
        }
    }

    return shaderAsset;
}
