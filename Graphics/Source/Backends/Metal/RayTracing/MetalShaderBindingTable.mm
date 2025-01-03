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

#import <DenOfIzGraphics/Backends/Metal/MetalBufferResource.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderBindingTable.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderLocalData.h>

using namespace DenOfIz;

MetalShaderBindingTable::MetalShaderBindingTable( MetalContext *context, const ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline = dynamic_cast<MetalPipeline *>( desc.Pipeline );
    // In Metal, the concept of a local root signature does not apply to shader stages independently. In case a source of the shader is shared between multiple stages,
    // we need to account for the maximum size of the data for all stages.
    size_t maxBytes    = Utilities::Align( std::max( { m_desc.MaxRayGenDataBytes, m_desc.MaxHitGroupDataBytes, m_desc.MaxMissDataBytes } ), 16 );
    m_rayGenNumBytes   = sizeof( IRShaderIdentifier ) + maxBytes;
    m_hitGroupNumBytes = sizeof( IRShaderIdentifier ) + maxBytes;
    m_missNumBytes     = sizeof( IRShaderIdentifier ) + maxBytes;

    m_debugData.RayGenNumBytes   = m_rayGenNumBytes;
    m_debugData.MissNumBytes     = m_missNumBytes;
    m_debugData.HitGroupNumBytes = m_hitGroupNumBytes;

    Resize( desc.SizeDesc );
}

void MetalShaderBindingTable::Resize( const SBTSizeDesc &desc )
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

void MetalShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const uint32_t    &functionIndex    = m_pipeline->FindVisibleShaderIndexByName( desc.ShaderName.Get( ) );
    IRShaderIdentifier shaderIdentifier = EncodeShaderIndex( 0, functionIndex );
    uint32_t           numBytes         = 0;
    if ( desc.Data )
    {
        numBytes = EncodeData( sizeof( IRShaderIdentifier ), desc.Data );
    }
#ifndef NDEBUG
    m_debugData.RayGenerationShaders.AddElement( { &shaderIdentifier, sizeof( IRShaderIdentifier ), numBytes, desc.ShaderName.Get( ) } );
#endif
}

void MetalShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    const uint32_t offset = m_hitGroupOffset + desc.Offset * m_hitGroupNumBytes;

    const HitGroupExport &hitGroupExport = m_pipeline->FindHitGroupExport( desc.HitGroupExportName.Get( ) );

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

    uint32_t numBytes = 0;
    if ( desc.Data )
    {
        numBytes = EncodeData( offset + sizeof( IRShaderIdentifier ), desc.Data );
    }

#ifndef NDEBUG
    m_debugData.HitGroups.AddElement( { &shaderIdentifier, sizeof( IRShaderIdentifier ), numBytes, desc.HitGroupExportName.Get( ) } );
#endif
}

void MetalShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    uint32_t           offset           = m_missGroupOffset + desc.Offset * m_missNumBytes;
    const uint32_t    &functionIndex    = m_pipeline->FindVisibleShaderIndexByName( desc.ShaderName.Get( ) );
    IRShaderIdentifier shaderIdentifier = EncodeShaderIndex( offset, functionIndex );
    uint32_t           numBytes         = 0;
    if ( desc.Data )
    {
        numBytes = EncodeData( offset + sizeof( IRShaderIdentifier ), desc.Data );
    }

#ifndef NDEBUG
    m_debugData.MissShaders.AddElement( { &shaderIdentifier, sizeof( IRShaderIdentifier ), numBytes, desc.ShaderName.Get( ) } );
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

const id<MTLBuffer> MetalShaderBindingTable::MetalBuffer( ) const
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

#ifndef NDEBUG
    LOG( INFO ) << " Offset: " << offset << " IRShaderIdentifier: {.intersectionShaderHandle=" << shaderIdentifier.intersectionShaderHandle
                << ", .shaderHandle=" << shaderIdentifier.shaderHandle << " }";
#endif
    Byte *shaderEntry = static_cast<Byte *>( m_mappedMemory ) + offset;
    memcpy( shaderEntry, &shaderIdentifier, sizeof( IRShaderIdentifier ) );

    return shaderIdentifier;
}

uint32_t MetalShaderBindingTable::EncodeData( uint32_t offset, const IShaderLocalData *iData )
{
    MetalShaderLocalData *localData = static_cast<MetalShaderLocalData *>( const_cast<IShaderLocalData *>( iData ) );
    Byte                 *dest      = static_cast<Byte *>( m_mappedMemory ) + offset;

    const Byte    *data     = localData->Data( );
    const uint32_t numBytes = localData->DataNumBytes( );
    memcpy( dest, data, numBytes );

    const DescriptorTable *srvUavTable = localData->SrvUavTable( );
    if ( srvUavTable )
    {
        uint32_t      srvUavOffset        = numBytes;
        id<MTLBuffer> srvUavBuffer        = srvUavTable->Buffer( );
        uint64_t      srvUavBufferAddress = srvUavBuffer.gpuAddress;
        memcpy( dest + numBytes, &srvUavBufferAddress, sizeof( uint64_t ) );
    }
    const DescriptorTable *samplerTable = localData->SamplerTable( );
    if ( samplerTable )
    {
        uint32_t      samplerOffset  = numBytes + sizeof( uint64_t );
        id<MTLBuffer> samplerBuffer  = localData->SamplerTable( )->Buffer( );
        uint64_t      samplerAddress = samplerBuffer.gpuAddress;
        memcpy( dest + samplerOffset, &samplerAddress, sizeof( uint64_t ) );
    }

    // Todo optimize this
    for ( const id<MTLResource> &resource : localData->UsedResources( ) )
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
