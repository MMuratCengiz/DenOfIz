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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalBuffer.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderBindingTable.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderLocalData.h"
#import "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define MTL_PIPELINE_IMPL( handle )          DENOFIZ_FROM_HANDLE( DenOfIz::MetalPipeline, handle )
#define MTL_SHADER_LOCAL_DATA_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::MetalShaderLocalData, handle )

using namespace DenOfIz;

MetalShaderBindingTable::MetalShaderBindingTable( MetalContext *context, const DenOfIz_ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline = MTL_PIPELINE_IMPL( desc.Pipeline );
    // In Metal, the concept of a local root signature does not apply to shader stages independently. In case a source of the shader is shared between multiple stages,
    // we need to account for the maximum size of the data for all stages.
    const uint32_t maxBytes = Utilities::Align( std::max( { m_desc.MaxRayGenDataBytes, m_desc.MaxHitGroupDataBytes, m_desc.MaxMissDataBytes, m_pipeline->LocalShaderLayout( ).NumBytes( ) } ), 16 );
    m_maxDataBytes     = maxBytes;
    m_rayGenNumBytes   = sizeof( IRShaderIdentifier ) + maxBytes;
    m_hitGroupNumBytes = sizeof( IRShaderIdentifier ) + maxBytes;
    m_missNumBytes     = sizeof( IRShaderIdentifier ) + maxBytes;

    m_debugData.RayGenNumBytes   = m_rayGenNumBytes;
    m_debugData.MissNumBytes     = m_missNumBytes;
    m_debugData.HitGroupNumBytes = m_hitGroupNumBytes;

    Resize( desc.SizeDesc );
}

void MetalShaderBindingTable::Resize( const DenOfIz_SBTSizeDesc &desc )
{
    size_t rayGenNumBytes   = desc.NumRayGenerationShaders * m_rayGenNumBytes;
    size_t hitGroupNumBytes = desc.NumHitGroups * m_hitGroupNumBytes;
    size_t missNumBytes     = desc.NumMissShaders * m_missNumBytes;
    m_numBufferBytes        = AlignRecord( rayGenNumBytes ) + AlignRecord( hitGroupNumBytes ) + AlignRecord( missNumBytes );

    m_buffer = [m_context->Device newBufferWithLength:m_numBufferBytes options:MTLResourceStorageModeShared];
    [m_buffer setLabel:@"Shader Binding Table"];
    m_usedResources.push_back( m_buffer );

    m_mappedMemory = static_cast<Byte *>( [m_buffer contents] );

    m_rayGenerationShaderRange.StartAddress = m_buffer.gpuAddress;
    m_rayGenerationShaderRange.SizeInBytes  = rayGenNumBytes;

    m_hitGroupOffset                    = AlignRecord( m_rayGenNumBytes );
    m_hitGroupShaderRange.StartAddress  = m_buffer.gpuAddress + m_hitGroupOffset;
    m_hitGroupShaderRange.SizeInBytes   = hitGroupNumBytes;
    m_hitGroupShaderRange.StrideInBytes = m_hitGroupNumBytes;

    m_missGroupOffset               = AlignRecord( m_hitGroupOffset + hitGroupNumBytes );
    m_missShaderRange.StartAddress  = m_buffer.gpuAddress + m_missGroupOffset;
    m_missShaderRange.SizeInBytes   = missNumBytes;
    m_missShaderRange.StrideInBytes = m_missNumBytes;
}

void MetalShaderBindingTable::BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc )
{
    const std::string        shaderName( desc.ShaderName.Chars, desc.ShaderName.NumChars );
    const uint32_t          &functionIndex    = m_pipeline->FindVisibleShaderIndexByName( shaderName );
    IRShaderIdentifier       shaderIdentifier = EncodeShaderIndex( 0, functionIndex );
    uint32_t                 numBytes         = 0;
    MetalShaderLocalData *localData    = DENOFIZ_HANDLE_IS_VALID( desc.Data ) ? MTL_SHADER_LOCAL_DATA_IMPL( desc.Data ) : nullptr;
    if ( localData )
    {
        numBytes = EncodeData( sizeof( IRShaderIdentifier ), localData );
    }
#ifndef NDEBUG
    m_debugData.RayGenerationShaders.push_back( { &shaderIdentifier, sizeof( IRShaderIdentifier ), numBytes, shaderName } );
#endif
}

void MetalShaderBindingTable::BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc )
{
    const uint32_t offset = m_hitGroupOffset + desc.Offset * m_hitGroupNumBytes;

    const std::string hitGroupExportName( desc.HitGroupExportName.Chars, desc.HitGroupExportName.NumChars );
    const HitGroupExport &hitGroupExport = m_pipeline->FindHitGroupExport( hitGroupExportName );

    IRShaderIdentifier shaderIdentifier;
    if ( hitGroupExport.AnyHit != 0 )
    {
        shaderIdentifier = EncodeShaderIndex( offset, hitGroupExport.ClosestHit, hitGroupExport.AnyHit );
    }
    else if ( hitGroupExport.Intersection != 0 )
    {
        shaderIdentifier = EncodeShaderIndex( offset, hitGroupExport.ClosestHit, hitGroupExport.Intersection );
    }
    else
    {
        shaderIdentifier = EncodeShaderIndex( offset, hitGroupExport.ClosestHit );
    }

    uint32_t                   numBytes  = 0;
    MetalShaderLocalData *localData = DENOFIZ_HANDLE_IS_VALID( desc.Data ) ? MTL_SHADER_LOCAL_DATA_IMPL( desc.Data ) : nullptr;
    if ( localData )
    {
        numBytes = EncodeData( offset + sizeof( IRShaderIdentifier ), localData );
    }

#ifndef NDEBUG
    m_debugData.HitGroups.push_back( { &shaderIdentifier, sizeof( IRShaderIdentifier ), numBytes, hitGroupExportName } );
#endif
}

void MetalShaderBindingTable::BindMissShader( const DenOfIz_MissBindingDesc &desc )
{
    uint32_t                      offset           = m_missGroupOffset + desc.Offset * m_missNumBytes;
    const std::string             shaderName( desc.ShaderName.Chars, desc.ShaderName.NumChars );
    const uint32_t               &functionIndex    = m_pipeline->FindVisibleShaderIndexByName( shaderName );
    IRShaderIdentifier            shaderIdentifier = EncodeShaderIndex( offset, functionIndex );
    uint32_t                      numBytes         = 0;
    MetalShaderLocalData         *localData        = DENOFIZ_HANDLE_IS_VALID( desc.Data ) ? MTL_SHADER_LOCAL_DATA_IMPL( desc.Data ) : nullptr;
    if ( localData )
    {
        numBytes = EncodeData( offset + sizeof( IRShaderIdentifier ), localData );
    }

#ifndef NDEBUG
    m_debugData.MissShaders.push_back( { &shaderIdentifier, sizeof( IRShaderIdentifier ), numBytes, shaderName } );
#endif
}

void MetalShaderBindingTable::Build( )
{
#ifndef NDEBUG
    PrintShaderBindingTableDebugData( m_debugData );
#endif
}

const IRVirtualAddressRange &MetalShaderBindingTable::RayGenerationShaderRange( ) const
{
    return m_rayGenerationShaderRange;
}

const IRVirtualAddressRangeAndStride &MetalShaderBindingTable::HitGroupShaderRange( ) const
{
    return m_hitGroupShaderRange;
}

const IRVirtualAddressRangeAndStride &MetalShaderBindingTable::MissShaderRange( ) const
{
    return m_missShaderRange;
}

const std::vector<id<MTLResource>> &MetalShaderBindingTable::UsedResources( ) const
{
    return m_usedResources;
}

const id<MTLBuffer> MetalShaderBindingTable::GetMetalBuffer( ) const
{
    return m_buffer;
}

IRShaderIdentifier MetalShaderBindingTable::EncodeShaderIndex( uint32_t offset, uint32_t shaderIndex, int customIntersectionIndex )
{
    IRShaderIdentifier shaderIdentifier;
    if ( customIntersectionIndex != -1 )
    {
        IRShaderIdentifierInitWithCustomIntersection( &shaderIdentifier, shaderIndex, customIntersectionIndex );
    }
    else
    {
        IRShaderIdentifierInit( &shaderIdentifier, shaderIndex );
    }

    Byte *shaderEntry = static_cast<Byte *>( m_mappedMemory ) + offset;
    memcpy( shaderEntry, &shaderIdentifier, sizeof( IRShaderIdentifier ) );

    return shaderIdentifier;
}

uint32_t MetalShaderBindingTable::EncodeData( uint32_t offset, MetalShaderLocalData *localData )
{
    Byte *dest = static_cast<Byte *>( m_mappedMemory ) + offset;

    const MetalLocalRecord *record   = localData->RecordFor( m_pipeline->LocalShaderLayout( ) );
    const uint32_t          numBytes = static_cast<uint32_t>( record->Data.size( ) );
    if ( numBytes > m_maxDataBytes )
    {
        spdlog::error( "Shader local data requires {} bytes but the shader binding table record only holds {} bytes. Record skipped.", numBytes, m_maxDataBytes );
        return 0;
    }
    memcpy( dest, record->Data.data( ), numBytes );

    // Todo optimize this
    for ( const id<MTLResource> &resource : record->UsedResources )
    {
        if ( std::find( m_usedResources.begin( ), m_usedResources.end( ), resource ) == m_usedResources.end( ) )
        {
            m_usedResources.push_back( resource );
        }
    }

    return numBytes;
}

uint32_t MetalShaderBindingTable::AlignRecord( const uint32_t& size )
{
    return Utilities::Align( size, 256 );
}
