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
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define SHADER_ASSET_READER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ShaderAssetReader, handle )

namespace DenOfIz
{
    class ShaderAssetReader
    {
    public:
        DenOfIz_BinaryReader m_reader;
        DenOfIz_ShaderAsset  m_shaderAsset       = DENOFIZ_NULL_HANDLE;
        bool                 m_assetRead         = false;
        uint64_t             m_streamStartOffset = 0;

        explicit ShaderAssetReader( const DenOfIz_ShaderAssetReaderDesc &desc );
        ~ShaderAssetReader( );

        DenOfIz_ShaderAsset Read( );

    private:
        void ReadHeader( ) const;
        void ReadInputLayout( DenOfIz_InputLayoutDesc &inputLayout ) const;
        void ReadBindGroupLayouts( DenOfIz_BindGroupLayoutDescArray &bindGroupLayouts ) const;
        void ReadBindGroupLayout( DenOfIz_BindGroupLayoutDesc &bindGroupLayout ) const;
        void ReadRootConstants( DenOfIz_RootConstantBindingDescArray &rootConstants ) const;
        void ReadLocalRootSignature( DenOfIz_LocalRootSignatureDesc &localDesc ) const;
        void ReadBinding( DenOfIz_BindingDesc &binding ) const;
        void ReadLocalResourceBinding( DenOfIz_LocalResourceBindingDesc &binding ) const;
    };
} // namespace DenOfIz

using namespace DenOfIz;

ShaderAssetReader::ShaderAssetReader( const DenOfIz_ShaderAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_reader ) )
    {
        spdlog::critical( "BinaryReader cannot be null for ShaderAssetReader" );
    }
}

ShaderAssetReader::~ShaderAssetReader( ) = default;

DenOfIz_ShaderAsset ShaderAssetReader::Read( )
{
    if ( m_assetRead )
    {
        return m_shaderAsset;
    }

    m_shaderAsset       = DenOfIz_ShaderAsset_Create( );
    m_streamStartOffset = DenOfIz_BinaryReader_Position( m_reader );
    ReadHeader( );

    const uint32_t numStages = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    DenOfIz_ShaderAsset_ReserveStages( m_shaderAsset, numStages );

    for ( uint32_t i = 0; i < numStages; ++i )
    {
        DenOfIz_ShaderStageAsset *stage = DenOfIz_ShaderAsset_AddStage( m_shaderAsset );

        stage->Stage = static_cast<DenOfIz_ShaderStageFlags>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) );

        DenOfIz_StringView entryPointStr = DenOfIz_BinaryReader_ReadString( m_reader );
        if ( entryPointStr.NumChars > 0 )
        {
            DenOfIz_ShaderAsset_SetStageEntryPoint( m_shaderAsset, i, entryPointStr );
        }

        const uint64_t dxilSize = DenOfIz_BinaryReader_ReadUInt64( m_reader );
        stage->DXIL             = DenOfIz_BinaryReader_ReadBytes( m_reader, dxilSize );

        const uint64_t mslSize = DenOfIz_BinaryReader_ReadUInt64( m_reader );
        stage->MSL             = DenOfIz_BinaryReader_ReadBytes( m_reader, mslSize );

        const uint64_t spirvSize = DenOfIz_BinaryReader_ReadUInt64( m_reader );
        stage->SPIRV             = DenOfIz_BinaryReader_ReadBytes( m_reader, spirvSize );

        const uint64_t wgslSize = DenOfIz_BinaryReader_ReadUInt64( m_reader );
        stage->WGSL             = DenOfIz_BinaryReader_ReadBytes( m_reader, wgslSize );

        const uint64_t reflectionSize = DenOfIz_BinaryReader_ReadUInt64( m_reader );
        stage->Reflection             = DenOfIz_BinaryReader_ReadBytes( m_reader, reflectionSize );

        const uint32_t numLocalBindings             = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        stage->RayTracing.LocalBindings.NumElements = numLocalBindings;
        stage->RayTracing.LocalBindings.Elements    = new DenOfIz_ResourceBindingSlot[ numLocalBindings ];

        for ( uint32_t j = 0; j < numLocalBindings; ++j )
        {
            DenOfIz_ResourceBindingSlot &binding = stage->RayTracing.LocalBindings.Elements[ j ];
            binding.RegisterSpace                = DenOfIz_BinaryReader_ReadUInt32( m_reader );
            binding.Binding                      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
            binding.Type                         = static_cast<DenOfIz_ResourceBindingType>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
        }

        stage->RayTracing.HitGroupType = static_cast<DenOfIz_HitGroupType>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
    }

    ReadBindGroupLayouts( DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->BindGroupLayouts );
    ReadRootConstants( DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->RootConstants );
    ReadInputLayout( DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->InputLayout );

    const uint32_t numLocalRootSigs                                                      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->LocalRootSignatures.NumElements = numLocalRootSigs;
    DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->LocalRootSignatures.Elements    = new DenOfIz_LocalRootSignatureDesc[ numLocalRootSigs ];

    for ( uint32_t i = 0; i < numLocalRootSigs; ++i )
    {
        DenOfIz_LocalRootSignatureDesc &localDesc = DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->LocalRootSignatures.Elements[ i ];
        ReadLocalRootSignature( localDesc );
    }

    DenOfIz_ShaderAsset_GetRayTracing( m_shaderAsset )->MaxNumPayloadBytes        = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    DenOfIz_ShaderAsset_GetRayTracing( m_shaderAsset )->MaxNumAttributeBytes      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    DenOfIz_ShaderAsset_GetRayTracing( m_shaderAsset )->MaxRecursionDepth         = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    const uint32_t numThreadGroups                                                = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->ThreadGroups.NumElements = numThreadGroups;
    DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->ThreadGroups.Elements    = new DenOfIz_ThreadGroupDesc[ numThreadGroups ];

    for ( uint32_t i = 0; i < numThreadGroups; ++i )
    {
        DenOfIz_ThreadGroupDesc &threadGroup = DenOfIz_ShaderAsset_GetReflectDesc( m_shaderAsset )->ThreadGroups.Elements[ i ];
        threadGroup.X                        = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        threadGroup.Y                        = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        threadGroup.Z                        = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    }
    m_assetRead = true;
    return m_shaderAsset;
}

void ShaderAssetReader::ReadHeader( ) const
{
    uint64_t           magic    = DenOfIz_BinaryReader_ReadUInt64( m_reader );
    uint32_t           version  = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    uint64_t           numBytes = DenOfIz_BinaryReader_ReadUInt64( m_reader );
    DenOfIz_StringView pathStr  = DenOfIz_BinaryReader_ReadString( m_reader );

    DenOfIz_ShaderAsset_SetVersion( m_shaderAsset, version );
    DenOfIz_ShaderAsset_SetNumBytes( m_shaderAsset, numBytes );
    if ( pathStr.NumChars > 0 )
    {
        DenOfIz_ShaderAsset_SetPath( m_shaderAsset, pathStr );
    }
}

void ShaderAssetReader::ReadInputLayout( DenOfIz_InputLayoutDesc &inputLayout ) const
{
    const uint32_t numInputGroups       = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    inputLayout.InputGroups.NumElements = numInputGroups;
    inputLayout.InputGroups.Elements    = new DenOfIz_InputGroupDesc[ numInputGroups ];

    for ( uint32_t i = 0; i < numInputGroups; ++i )
    {
        DenOfIz_InputGroupDesc &inputGroup  = inputLayout.InputGroups.Elements[ i ];
        const uint32_t          numElements = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        inputGroup.Elements.NumElements     = numElements;
        inputGroup.Elements.Elements        = new DenOfIz_InputLayoutElementDesc[ numElements ];

        for ( uint32_t j = 0; j < numElements; ++j )
        {
            DenOfIz_InputLayoutElementDesc &element = inputGroup.Elements.Elements[ j ];

            DenOfIz_StringView semanticStr = DenOfIz_BinaryReader_ReadString( m_reader );
            element.Semantic               = semanticStr.NumChars > 0 ? DenOfIz_ShaderAsset_StoreString( m_shaderAsset, semanticStr ) : DENOFIZ_NULL_STRING;
            element.SemanticIndex          = DenOfIz_BinaryReader_ReadUInt32( m_reader );
            element.Format                 = static_cast<DenOfIz_Format>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
        }
        inputGroup.StepRate = static_cast<DenOfIz_StepRate>( DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
        inputGroup.Stride   = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    }
}

void ShaderAssetReader::ReadBindGroupLayouts( DenOfIz_BindGroupLayoutDescArray &bindGroupLayouts ) const
{
    const uint32_t numBindGroupLayouts = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    bindGroupLayouts.NumElements       = numBindGroupLayouts;
    bindGroupLayouts.Elements          = new DenOfIz_BindGroupLayoutDesc[ numBindGroupLayouts ];

    for ( uint32_t i = 0; i < numBindGroupLayouts; ++i )
    {
        DenOfIz_BindGroupLayoutDesc &layout = bindGroupLayouts.Elements[ i ];
        ReadBindGroupLayout( layout );
    }
}

void ShaderAssetReader::ReadBindGroupLayout( DenOfIz_BindGroupLayoutDesc &bindGroupLayout ) const
{
    bindGroupLayout.RegisterSpace = DenOfIz_BinaryReader_ReadUInt32( m_reader );

    const uint32_t numBindings           = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    bindGroupLayout.Bindings.NumElements = numBindings;
    bindGroupLayout.Bindings.Elements    = new DenOfIz_BindingDesc[ numBindings ];

    for ( uint32_t i = 0; i < numBindings; ++i )
    {
        DenOfIz_BindingDesc &binding = bindGroupLayout.Bindings.Elements[ i ];
        ReadBinding( binding );
    }
}

void ShaderAssetReader::ReadRootConstants( DenOfIz_RootConstantBindingDescArray &rootConstants ) const
{
    const uint32_t numRootConstants = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    rootConstants.NumElements       = numRootConstants;
    rootConstants.Elements          = new DenOfIz_RootConstantBindingDesc[ numRootConstants ];

    for ( uint32_t i = 0; i < numRootConstants; ++i )
    {
        DenOfIz_RootConstantBindingDesc &constant     = rootConstants.Elements[ i ];
        DenOfIz_StringView               constantName = DenOfIz_BinaryReader_ReadString( m_reader );
        constant.Name                                 = constantName.NumChars > 0 ? DenOfIz_ShaderAsset_StoreString( m_shaderAsset, constantName ) : DENOFIZ_NULL_STRING;
        constant.Binding                              = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        constant.NumBytes                             = DenOfIz_BinaryReader_ReadUInt64( m_reader );
        constant.Stages                               = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    }
}

void ShaderAssetReader::ReadLocalRootSignature( DenOfIz_LocalRootSignatureDesc &localDesc ) const
{
    const uint32_t numBindings             = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    localDesc.ResourceBindings.NumElements = numBindings;
    localDesc.ResourceBindings.Elements    = new DenOfIz_LocalResourceBindingDesc[ numBindings ];

    for ( uint32_t j = 0; j < numBindings; ++j )
    {
        DenOfIz_LocalResourceBindingDesc &bindingDesc = localDesc.ResourceBindings.Elements[ j ];
        ReadLocalResourceBinding( bindingDesc );
    }
}

void ShaderAssetReader::ReadBinding( DenOfIz_BindingDesc &binding ) const
{
    binding.Binding    = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.Descriptor = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.Stages     = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.ArraySize  = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.IsBindless = ReadByte( m_reader ) == 1;
}

void ShaderAssetReader::ReadLocalResourceBinding( DenOfIz_LocalResourceBindingDesc &binding ) const
{
    binding.Binding       = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.RegisterSpace = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.Descriptor    = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.Stages        = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    binding.ArraySize     = DenOfIz_BinaryReader_ReadInt32( m_reader );
    binding.NumBytes      = DenOfIz_BinaryReader_ReadUInt64( m_reader );
}

extern "C"
{

    DenOfIz_ShaderAssetReader DenOfIz_ShaderAssetReader_Create( const DenOfIz_ShaderAssetReaderDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *reader = new ShaderAssetReader( *desc );
        return DENOFIZ_TO_HANDLE( reader );
    }

    void DenOfIz_ShaderAssetReader_Destroy( DenOfIz_ShaderAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return;
        }
        delete SHADER_ASSET_READER_IMPL( reader );
    }

    DenOfIz_ShaderAsset DenOfIz_ShaderAssetReader_Read( DenOfIz_ShaderAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return SHADER_ASSET_READER_IMPL( reader )->Read( );
    }
}
