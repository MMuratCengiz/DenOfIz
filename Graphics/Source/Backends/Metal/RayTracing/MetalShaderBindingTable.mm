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
    m_pipeline              = dynamic_cast<MetalPipeline *>( desc.Pipeline );
    m_rayGenEntryNumBytes   = sizeof( IRShaderIdentifier ) + m_desc.MaxRayGenDataBytes;
    m_hitGroupEntryNumBytes = sizeof( IRShaderIdentifier ) + m_desc.MaxHitGroupDataBytes;
    m_missEntryNumBytes     = sizeof( IRShaderIdentifier ) + m_desc.MaxMissDataBytes;
    Resize( desc.SizeDesc );
}

void MetalShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    uint32_t numHitGroups     = desc.NumInstances * desc.NumGeometries * desc.NumRayTypes;
    size_t   rayGenNumBytes   = desc.NumRayGenerationShaders * m_rayGenEntryNumBytes;
    size_t   hitGroupNumBytes = numHitGroups * m_hitGroupEntryNumBytes;
    size_t   missNumBytes     = desc.NumMissShaders * m_missEntryNumBytes;
    m_numBufferBytes          = rayGenNumBytes + hitGroupNumBytes + missNumBytes;

    m_buffer = [m_context->Device newBufferWithLength:m_numBufferBytes options:MTLResourceStorageModeShared];
    [m_buffer setLabel:@"Shader Binding Table"];

    m_mappedMemory = static_cast<Byte *>( [m_buffer contents] );

    m_rayGenerationShaderRange.StartAddress = m_buffer.gpuAddress;
    m_rayGenerationShaderRange.SizeInBytes  = rayGenNumBytes;

    m_hitGroupOffset                    = m_rayGenEntryNumBytes;
    m_hitGroupShaderRange.StartAddress  = m_buffer.gpuAddress + m_hitGroupOffset;
    m_hitGroupShaderRange.SizeInBytes   = hitGroupNumBytes;
    m_hitGroupShaderRange.StrideInBytes = m_hitGroupEntryNumBytes;

    m_missGroupOffset               = m_hitGroupOffset + hitGroupNumBytes;
    m_missShaderRange.StartAddress  = m_buffer.gpuAddress + m_missGroupOffset;
    m_missShaderRange.SizeInBytes   = missNumBytes;
    m_missShaderRange.StrideInBytes = m_missEntryNumBytes;
}

void MetalShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const uint32_t &functionIndex = m_pipeline->FindVisibleShaderIndexByName( desc.ShaderName.Get( ) );
    EncodeShaderIndex( 0, functionIndex );
    if ( desc.Data )
    {
        EncodeData( sizeof( IRShaderIdentifier ), desc.Data );
    }
}

void MetalShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    if ( BindHitGroupRecursive( desc ) )
    {
        return;
    }

    const uint32_t instanceOffset = desc.InstanceIndex * m_desc.SizeDesc.NumGeometries * m_desc.SizeDesc.NumRayTypes;
    const uint32_t geometryOffset = desc.GeometryIndex * m_desc.SizeDesc.NumRayTypes;
    const uint32_t rayTypeOffset  = desc.RayTypeIndex;
    const uint32_t offset         = m_hitGroupOffset + ( instanceOffset + geometryOffset + rayTypeOffset ) * sizeof( IRShaderIdentifier );

    const HitGroupExport &hitGroupExport = m_pipeline->FindHitGroupExport( desc.HitGroupExportName.Get( ) );

    if ( hitGroupExport.AnyHit != 0 )
    {
        EncodeShaderIndex( offset, hitGroupExport.ClosestHit, hitGroupExport.AnyHit );
    }
    else if ( hitGroupExport.Intersection != 0 )
    {
        EncodeShaderIndex( offset, hitGroupExport.ClosestHit, hitGroupExport.Intersection );
    }
    else
    {
        EncodeShaderIndex( offset, hitGroupExport.ClosestHit );
    }

    if ( desc.Data )
    {
        EncodeData( offset + sizeof( IRShaderIdentifier ), desc.Data );
    }
}

bool MetalShaderBindingTable::BindHitGroupRecursive( const HitGroupBindingDesc &desc )
{
    if ( desc.InstanceIndex == -1 )
    {
        if ( desc.GeometryIndex != -1 || desc.RayTypeIndex != -1 )
        {
            LOG( ERROR ) << "Invalid hierarchy for InstanceIndex/GeometryIndex/RayTypeIndex.";
            return true;
        }

        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumInstances; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.InstanceIndex       = i;
            BindHitGroupRecursive( hitGroupDesc );
        }
        return true;
    }

    if ( desc.GeometryIndex == -1 )
    {
        if ( desc.RayTypeIndex != -1 )
        {
            LOG( ERROR ) << "Invalid hierarchy for GeometryIndex/RayTypeIndex.";
            return true;
        }

        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumGeometries; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.GeometryIndex       = i;
            BindHitGroupRecursive( hitGroupDesc );
        }
        return true;
    }

    if ( desc.RayTypeIndex == -1 )
    {
        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumRayTypes; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.RayTypeIndex        = i;
            BindHitGroup( hitGroupDesc );
        }
        return true;
    }

    return false;
}

void MetalShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    uint32_t        offset        = m_missGroupOffset + desc.RayTypeIndex * sizeof( IRShaderIdentifier );
    const uint32_t &functionIndex = m_pipeline->FindVisibleShaderIndexByName( desc.ShaderName.Get( ) );
    EncodeShaderIndex( offset, functionIndex );
    if ( desc.Data )
    {
        EncodeData( offset + sizeof( IRShaderIdentifier ), desc.Data );
    }
}

void MetalShaderBindingTable::Build( )
{
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

const id<MTLBuffer> MetalShaderBindingTable::MetalBuffer( ) const
{
    return m_buffer;
}

void MetalShaderBindingTable::EncodeShaderIndex( uint32_t offset, uint32_t shaderIndex, int customIntersectionIndex )
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
}

void MetalShaderBindingTable::EncodeData( uint32_t offset, const IShaderLocalData *iData )
{
    MetalShaderLocalData *localData = static_cast<MetalShaderLocalData *>( const_cast<IShaderLocalData *>( iData ) );
    Byte                 *dest      = static_cast<Byte *>( m_mappedMemory ) + offset;

    const Byte    *data     = localData->Data( );
    const uint32_t numBytes = localData->DataNumBytes( );
    memcpy( dest, data, numBytes );

    uint32_t      srvUavOffset        = numBytes;
    id<MTLBuffer> srvUavBuffer        = localData->SrvUavTable( )->Buffer( );
    uint64_t      srvUavBufferAddress = srvUavBuffer.gpuAddress;
    memcpy( dest + numBytes, &srvUavBufferAddress, sizeof( uint64_t ) );

    uint32_t      samplerOffset  = numBytes + sizeof( uint64_t );
    id<MTLBuffer> samplerBuffer  = localData->SamplerTable( )->Buffer( );
    uint64_t      samplerAddress = samplerBuffer.gpuAddress;
    memcpy( dest + srvUavOffset, &samplerOffset, sizeof( uint64_t ) );
}
