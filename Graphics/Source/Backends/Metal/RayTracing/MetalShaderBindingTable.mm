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

using namespace DenOfIz;

MetalShaderBindingTable::MetalShaderBindingTable( MetalContext *context, const ShaderBindingTableDesc &desc ) : m_context( context )
{
    m_pipeline = dynamic_cast<MetalPipeline *>( desc.Pipeline );
    Resize( desc.SizeDesc );
}

void MetalShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    uint32_t numHitGroups     = desc.NumInstances * desc.NumGeometries * desc.NumRayTypes;
    auto     hitGroupNumBytes = numHitGroups * sizeof( IRShaderIdentifier );
    m_numBufferBytes =
        desc.NumRayGenerationShaders * sizeof( IRShaderIdentifier ) + desc.NumGeometries * sizeof( IRShaderIdentifier ) + desc.NumMissShaders * sizeof( IRShaderIdentifier );

    m_buffer = [m_context->Device newBufferWithLength:m_numBufferBytes options:MTLResourceStorageModeShared];
    [m_buffer setLabel:@"Shader Binding Table"];

    m_mappedMemory = static_cast<IRShaderIdentifier *>( [m_buffer contents] );

    m_rayGenerationShaderRange.StartAddress = m_buffer.gpuAddress;
    m_rayGenerationShaderRange.SizeInBytes  = desc.NumRayGenerationShaders * sizeof( IRShaderIdentifier );

    m_hitGroupOffset                    = m_rayGenerationShaderRange.SizeInBytes;
    m_hitGroupShaderRange.StartAddress  = m_buffer.gpuAddress + m_hitGroupOffset;
    m_hitGroupShaderRange.SizeInBytes   = hitGroupNumBytes;
    m_hitGroupShaderRange.StrideInBytes = sizeof( IRShaderIdentifier );

    m_missGroupOffset               = m_hitGroupOffset + hitGroupNumBytes;
    m_missShaderRange.StartAddress  = m_buffer.gpuAddress + m_missGroupOffset;
    m_missShaderRange.SizeInBytes   = desc.NumMissShaders * sizeof( IRShaderIdentifier );
    m_missShaderRange.StrideInBytes = sizeof( IRShaderIdentifier );
}

void MetalShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const ShaderFunction &function = m_pipeline->FindVisibleShaderFunctionByName( desc.ShaderName.Get( ) );
    IRShaderIdentifierInit( &m_mappedMemory[ 0 ], function.Index );
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

    const IntersectionExport &intersectionExport = m_pipeline->IntersectionExport( );

    uint32_t indexedOffset = offset / sizeof( IRShaderIdentifier );
    IRShaderIdentifierInit( &m_mappedMemory[ indexedOffset ], intersectionExport.ClosestHit.Index );
    uint32_t index = 1;
    if ( intersectionExport.HasAnyHit )
    {
        IRShaderIdentifierInitWithCustomIntersection( &m_mappedMemory[ offset + index ], intersectionExport.ClosestHit.Index, intersectionExport.AnyHit.Index );
        index++;
    }
    if ( intersectionExport.HasIntersection )
    {
        IRShaderIdentifierInitWithCustomIntersection( &m_mappedMemory[ offset + index ], intersectionExport.ClosestHit.Index, intersectionExport.Intersection.Index );
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
    uint32_t offset = m_missGroupOffset + desc.RayTypeIndex * sizeof( IRShaderIdentifier );
    uint32_t indexedOffset = offset / sizeof( IRShaderIdentifier );

    const ShaderFunction &function = m_pipeline->FindVisibleShaderFunctionByName( desc.ShaderName.Get( ) );
    IRShaderIdentifierInit( &m_mappedMemory[ indexedOffset ], function.Index );
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

IBufferResource *MetalShaderBindingTable::Buffer( ) const
{
    //
    return nil;
}
