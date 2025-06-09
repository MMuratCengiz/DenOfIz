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

#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#ifdef _WIN32
#include <wrl/client.h>
#include "DenOfIzGraphics/Utilities/Common_Windows.h"
#else
#define __EMULATE_UUID
#include "WinAdapter.h"
#endif

#include "dxcapi.h"

using namespace DenOfIz;

ShaderAssetReader::ShaderAssetReader( const ShaderAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    DZ_NOT_NULL( m_reader );
}

ShaderAssetReader::~ShaderAssetReader( ) = default;

ShaderAsset ShaderAssetReader::Read( )
{
    if ( m_assetRead )
    {
        return m_shaderAsset;
    }

    m_streamStartOffset = m_reader->Position( );
    ReadHeader( );

    const uint32_t numStages         = m_reader->ReadUInt32( );
    m_shaderAsset.Stages.NumElements = numStages;
    m_shaderAsset.Stages.Elements    = static_cast<ShaderStageAsset *>( std::malloc( numStages * sizeof( ShaderStageAsset ) ) );

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        ShaderStageAsset &stage = m_shaderAsset.Stages.Elements[ i ];

        stage.Stage      = static_cast<ShaderStage>( m_reader->ReadUInt32( ) );
        stage.EntryPoint = m_reader->ReadString( );

        const uint64_t dxilSize = m_reader->ReadUInt64( );
        stage.DXIL              = m_reader->ReadBytes( dxilSize );

        const uint64_t mslSize = m_reader->ReadUInt64( );
        stage.MSL              = m_reader->ReadBytes( mslSize );

        const uint64_t spirvSize = m_reader->ReadUInt64( );
        stage.SPIRV              = m_reader->ReadBytes( spirvSize );

        const uint64_t reflectionSize = m_reader->ReadUInt64( );
        stage.Reflection              = m_reader->ReadBytes( reflectionSize );

        const uint32_t numLocalBindings = m_reader->ReadUInt32( );
        stage.RayTracing.LocalBindings.Resize( numLocalBindings );

        for ( uint32_t j = 0; j < numLocalBindings; ++j )
        {
            ResourceBindingSlot &binding = stage.RayTracing.LocalBindings.GetElement( j );
            binding.RegisterSpace        = m_reader->ReadUInt32( );
            binding.Binding              = m_reader->ReadUInt32( );
            binding.Type                 = static_cast<ResourceBindingType>( m_reader->ReadUInt32( ) );
        }

        stage.RayTracing.HitGroupType = static_cast<HitGroupType>( m_reader->ReadUInt32( ) );
    }

    ReadRootSignature( m_shaderAsset.ReflectDesc.RootSignature );
    ReadInputLayout( m_shaderAsset.ReflectDesc.InputLayout );

    const uint32_t numLocalRootSigs = m_reader->ReadUInt32( );
    m_shaderAsset.ReflectDesc.LocalRootSignatures.Resize( numLocalRootSigs );

    for ( uint32_t i = 0; i < numLocalRootSigs; ++i )
    {
        LocalRootSignatureDesc &localDesc = m_shaderAsset.ReflectDesc.LocalRootSignatures.GetElement( i );
        ReadLocalRootSignature( localDesc );
    }

    m_shaderAsset.RayTracing.MaxNumPayloadBytes   = m_reader->ReadUInt32( );
    m_shaderAsset.RayTracing.MaxNumAttributeBytes = m_reader->ReadUInt32( );
    m_shaderAsset.RayTracing.MaxRecursionDepth    = m_reader->ReadUInt32( );

    m_shaderAsset.UserProperties = AssetReaderHelpers::ReadUserProperties( m_reader );

    m_assetRead = true;
    return m_shaderAsset;
}

void ShaderAssetReader::ReadHeader( )
{
    m_shaderAsset.Magic    = m_reader->ReadUInt64( );
    m_shaderAsset.Version  = m_reader->ReadUInt32( );
    m_shaderAsset.NumBytes = m_reader->ReadUInt64( );
    m_shaderAsset.Uri      = AssetUri::Parse( m_reader->ReadString( ) );
}

void ShaderAssetReader::ReadInputLayout( InputLayoutDesc &inputLayout ) const
{
    const uint32_t numInputGroups = m_reader->ReadUInt32( );
    inputLayout.InputGroups.Resize( numInputGroups );

    for ( uint32_t i = 0; i < numInputGroups; ++i )
    {
        InputGroupDesc &inputGroup  = inputLayout.InputGroups.GetElement( i );
        const uint32_t  numElements = m_reader->ReadUInt32( );
        inputGroup.Elements.Resize( numElements );

        for ( int j = 0; j < numElements; ++j )
        {
            InputLayoutElementDesc &element = inputGroup.Elements.GetElement( j );

            element.Semantic      = m_reader->ReadString( );
            element.SemanticIndex = m_reader->ReadUInt32( );
            element.Format        = static_cast<Format>( m_reader->ReadUInt32( ) );
        }
        inputGroup.StepRate = static_cast<StepRate>( m_reader->ReadUInt32( ) );
    }
}

void ShaderAssetReader::ReadRootSignature( RootSignatureDesc &rootSignature ) const
{
    const uint32_t numResourceBindings = m_reader->ReadUInt32( );
    rootSignature.ResourceBindings.Resize( numResourceBindings );

    for ( uint32_t i = 0; i < numResourceBindings; ++i )
    {
        ResourceBindingDesc &binding = rootSignature.ResourceBindings.GetElement( i );
        ReadResourceBinding( binding );
    }

    const uint32_t numStaticSamplers = m_reader->ReadUInt32( );
    rootSignature.StaticSamplers.Resize( numStaticSamplers );

    for ( uint32_t i = 0; i < numStaticSamplers; ++i )
    {
        StaticSamplerDesc &sampler    = rootSignature.StaticSamplers.GetElement( i );
        sampler.Sampler.MagFilter     = static_cast<Filter>( m_reader->ReadUInt32( ) );
        sampler.Sampler.MinFilter     = static_cast<Filter>( m_reader->ReadUInt32( ) );
        sampler.Sampler.AddressModeU  = static_cast<SamplerAddressMode>( m_reader->ReadUInt32( ) );
        sampler.Sampler.AddressModeV  = static_cast<SamplerAddressMode>( m_reader->ReadUInt32( ) );
        sampler.Sampler.AddressModeW  = static_cast<SamplerAddressMode>( m_reader->ReadUInt32( ) );
        sampler.Sampler.MaxAnisotropy = m_reader->ReadFloat( );
        sampler.Sampler.MaxAnisotropy = m_reader->ReadUInt32( ); // Duplicated in the writer
        sampler.Sampler.CompareOp     = static_cast<CompareOp>( m_reader->ReadUInt32( ) );
        sampler.Sampler.MipmapMode    = static_cast<MipmapMode>( m_reader->ReadUInt32( ) );
        sampler.Sampler.MipLodBias    = m_reader->ReadFloat( );
        sampler.Sampler.MinLod        = m_reader->ReadFloat( );
        sampler.Sampler.MaxLod        = m_reader->ReadFloat( );
        sampler.Sampler.DebugName     = m_reader->ReadString( );

        sampler.Binding.Name          = m_reader->ReadString( );
        sampler.Binding.BindingType   = static_cast<ResourceBindingType>( m_reader->ReadUInt32( ) );
        sampler.Binding.Binding       = m_reader->ReadUInt32( );
        sampler.Binding.RegisterSpace = m_reader->ReadUInt32( );
        sampler.Binding.Descriptor    = m_reader->ReadUInt32( );

        const uint32_t numStages = m_reader->ReadUInt32( );
        sampler.Binding.Stages.Resize( numStages );
        for ( uint32_t j = 0; j < numStages; ++j )
        {
            sampler.Binding.Stages.GetElement( j ) = static_cast<ShaderStage>( m_reader->ReadUInt32( ) );
        }

        sampler.Binding.ArraySize = m_reader->ReadInt32( );
        ReadResourceReflection( sampler.Binding.Reflection );
    }

    const uint32_t numRootConstants = m_reader->ReadUInt32( );
    rootSignature.RootConstants.Resize( numRootConstants );

    for ( uint32_t i = 0; i < numRootConstants; ++i )
    {
        RootConstantResourceBindingDesc &constant = rootSignature.RootConstants.GetElement( i );
        constant.Name                             = m_reader->ReadString( );
        constant.Binding                          = m_reader->ReadUInt32( );
        constant.NumBytes                         = m_reader->ReadInt32( );

        const uint32_t numStages = m_reader->ReadUInt32( );
        constant.Stages.Resize( numStages );
        for ( uint32_t j = 0; j < numStages; ++j )
        {
            constant.Stages.GetElement( j ) = static_cast<ShaderStage>( m_reader->ReadUInt32( ) );
        }

        ReadResourceReflection( constant.Reflection );
    }
}

void ShaderAssetReader::ReadLocalRootSignature( LocalRootSignatureDesc &localDesc ) const
{
    const uint32_t numBindings = m_reader->ReadUInt32( );
    localDesc.ResourceBindings.Resize( numBindings );

    for ( uint32_t j = 0; j < numBindings; ++j )
    {
        ResourceBindingDesc &bindingDesc = localDesc.ResourceBindings.GetElement( j );
        ReadResourceBinding( bindingDesc );
    }
}

void ShaderAssetReader::ReadResourceBinding( ResourceBindingDesc &binding ) const
{
    binding.Name          = m_reader->ReadString( );
    binding.BindingType   = static_cast<ResourceBindingType>( m_reader->ReadUInt32( ) );
    binding.Binding       = m_reader->ReadUInt32( );
    binding.RegisterSpace = m_reader->ReadUInt32( );
    binding.Descriptor    = m_reader->ReadUInt32( );

    const uint32_t numStages = m_reader->ReadUInt32( );
    binding.Stages.Resize( numStages );
    for ( uint32_t j = 0; j < numStages; ++j )
    {
        binding.Stages.GetElement( j ) = static_cast<ShaderStage>( m_reader->ReadUInt32( ) );
    }

    binding.ArraySize = m_reader->ReadInt32( );
    ReadResourceReflection( binding.Reflection );
}

void ShaderAssetReader::ReadResourceReflection( ReflectionDesc &reflection ) const
{
    reflection.Name = m_reader->ReadString( );
    reflection.Type = static_cast<ReflectionBindingType>( m_reader->ReadUInt32( ) );

    const uint32_t numFields = m_reader->ReadUInt32( );
    reflection.Fields.Resize( numFields );

    for ( uint32_t i = 0; i < numFields; ++i )
    {
        ReflectionResourceField &field = reflection.Fields.GetElement( i );
        field.Name                     = m_reader->ReadString( );
        field.Type                     = static_cast<ReflectionFieldType>( m_reader->ReadUInt32( ) );
        field.NumColumns               = m_reader->ReadUInt32( );
        field.NumRows                  = m_reader->ReadUInt32( );
        field.Elements                 = m_reader->ReadUInt32( );
        field.Offset                   = m_reader->ReadUInt32( );
        field.Level                    = m_reader->ReadUInt32( );
        field.ParentIndex              = m_reader->ReadUInt32( );
    }

    reflection.NumBytes = m_reader->ReadUInt64( );
}

CompiledShader ShaderAssetReader::ConvertToCompiledShader( const ShaderAsset &shaderAsset )
{
    CompiledShader compiledShader;

    compiledShader.RayTracing.MaxNumPayloadBytes   = shaderAsset.RayTracing.MaxNumPayloadBytes;
    compiledShader.RayTracing.MaxNumAttributeBytes = shaderAsset.RayTracing.MaxNumAttributeBytes;
    compiledShader.RayTracing.MaxRecursionDepth    = shaderAsset.RayTracing.MaxRecursionDepth;

    compiledShader.ReflectDesc = shaderAsset.ReflectDesc;

    const uint32_t numStages          = shaderAsset.Stages.NumElements;
    compiledShader.Stages.NumElements = numStages;
    compiledShader.Stages.Elements    = static_cast<CompiledShaderStage **>( std::malloc( numStages * sizeof( CompiledShaderStage * ) ) );

    IDxcLibrary  *dxcLibrary = nullptr;
    const HRESULT result     = DxcCreateInstance( CLSID_DxcLibrary, IID_PPV_ARGS( &dxcLibrary ) );
    if ( FAILED( result ) || !dxcLibrary )
    {
        spdlog::error( "Failed to create DXC library for blob conversion" );
        return compiledShader;
    }

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        const ShaderStageAsset &stageAsset = shaderAsset.Stages.Elements[ i ];
        // We expect the user to delete these pointers(ShaderProgram assigns them to a unique ptr).
        auto compiledStage                  = new CompiledShaderStage( );
        compiledShader.Stages.Elements[ i ] = compiledStage;

        compiledStage->Stage      = stageAsset.Stage;
        compiledStage->EntryPoint = stageAsset.EntryPoint;
        compiledStage->RayTracing = stageAsset.RayTracing;

        if ( stageAsset.DXIL.NumElements( ) > 0 )
        {
            IDxcBlobEncoding *blob = nullptr;
            if ( SUCCEEDED( dxcLibrary->CreateBlobWithEncodingOnHeapCopy( stageAsset.DXIL.Data( ), stageAsset.DXIL.NumElements( ), DXC_CP_ACP, &blob ) ) )
            {
                compiledStage->DXIL.Elements    = static_cast<Byte *>( std::malloc( blob->GetBufferSize( ) ) );
                compiledStage->DXIL.NumElements = blob->GetBufferSize( );
                std::memcpy( compiledStage->DXIL.Elements, blob->GetBufferPointer( ), blob->GetBufferSize( ) );
                blob->Release( );
            }
        }

        if ( stageAsset.MSL.NumElements( ) > 0 )
        {
            IDxcBlobEncoding *blob = nullptr;
            if ( SUCCEEDED( dxcLibrary->CreateBlobWithEncodingOnHeapCopy( stageAsset.MSL.Data( ), stageAsset.MSL.NumElements( ), DXC_CP_ACP, &blob ) ) )
            {
                compiledStage->MSL.Elements    = static_cast<Byte *>( std::malloc( blob->GetBufferSize( ) ) );
                compiledStage->MSL.NumElements = blob->GetBufferSize( );
                std::memcpy( compiledStage->MSL.Elements, blob->GetBufferPointer( ), blob->GetBufferSize( ) );
                blob->Release( );
            }
        }

        if ( stageAsset.SPIRV.NumElements( ) > 0 )
        {
            IDxcBlobEncoding *blob = nullptr;
            if ( SUCCEEDED( dxcLibrary->CreateBlobWithEncodingOnHeapCopy( stageAsset.SPIRV.Data( ), stageAsset.SPIRV.NumElements( ), DXC_CP_ACP, &blob ) ) )
            {
                compiledStage->SPIRV.Elements    = static_cast<Byte *>( std::malloc( blob->GetBufferSize( ) ) );
                compiledStage->SPIRV.NumElements = blob->GetBufferSize( );
                std::memcpy( compiledStage->SPIRV.Elements, blob->GetBufferPointer( ), blob->GetBufferSize( ) );
                blob->Release( );
            }
        }

        if ( stageAsset.Reflection.NumElements( ) > 0 )
        {
            IDxcBlobEncoding *blob = nullptr;
            if ( SUCCEEDED( dxcLibrary->CreateBlobWithEncodingOnHeapCopy( stageAsset.Reflection.Data( ), stageAsset.Reflection.NumElements( ), DXC_CP_ACP, &blob ) ) )
            {
                compiledStage->Reflection.Elements    = static_cast<Byte *>( std::malloc( blob->GetBufferSize( ) ) );
                compiledStage->Reflection.NumElements = blob->GetBufferSize( );
                std::memcpy( compiledStage->Reflection.Elements, blob->GetBufferPointer( ), blob->GetBufferSize( ) );
                blob->Release( );
            }
        }
    }

    if ( dxcLibrary )
    {
        dxcLibrary->Release( );
    }

    return compiledShader;
}
