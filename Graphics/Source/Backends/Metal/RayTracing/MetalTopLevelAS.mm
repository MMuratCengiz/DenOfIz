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

#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalBottomLevelAS.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalTopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalTopLevelAS::MetalTopLevelAS( MetalContext *context, const TopLevelASDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_instanceBuffer = [m_context->Device newBufferWithLength:sizeof( MTLAccelerationStructureUserIDInstanceDescriptor ) * m_desc.Instances.NumElements( )
                                                      options:MTLResourceStorageModeShared];
    [m_instanceBuffer setLabel:@"Instance Descriptor Buffer"];

    m_instanceDescriptors = (MTLAccelerationStructureUserIDInstanceDescriptor *)m_instanceBuffer.contents;
    m_indirectResources.push_back( m_instanceBuffer );

    m_blasList = [NSMutableArray arrayWithCapacity:desc.Instances.NumElements( )];
    for ( size_t i = 0; i < desc.Instances.NumElements( ); ++i )
    {
        const ASInstanceDesc &instanceDesc = desc.Instances.GetElement( i );
        MetalBottomLevelAS   *blas         = dynamic_cast<MetalBottomLevelAS *>( instanceDesc.BLAS );

        if ( blas == nullptr )
        {
            spdlog::warn("BLAS is null.");
            continue;
        }

        [m_blasList addObject:blas->AccelerationStructure( )];

        MTLAccelerationStructureUserIDInstanceDescriptor *instance = &m_instanceDescriptors[ i ];
        instance->intersectionFunctionTableOffset                  = blas->GeometryType( ) == HitGroupType::Triangles ? TriangleIntersectionShader : ProceduralIntersectionShader;
        instance->accelerationStructureIndex                       = i;
        instance->userID                                           = instanceDesc.ID;
        instance->options                                          = blas->Options( );
        instance->mask                                             = instanceDesc.Mask;

        const float *transformData = instanceDesc.Transform.Data( );
        for ( int row = 0; row < 3; ++row )
        {
            for ( int col = 0; col < 4; ++col )
            {
                instance->transformationMatrix.columns[ col ][ row ] = transformData[ row * 4 + col ];
            }
        }

        const std::vector<id<MTLResource>> &blasResources = blas->IndirectResources( );
        m_indirectResources.insert( m_indirectResources.end( ), blasResources.begin( ), blasResources.end( ) );

        m_contributionsToHitGroupIndices.emplace_back( instanceDesc.ContributionToHitGroupIndex );
    }

    // Acceleration Structure Configuration
    m_descriptor                                 = [MTLInstanceAccelerationStructureDescriptor descriptor];
    m_descriptor.instancedAccelerationStructures = m_blasList;
    m_descriptor.instanceDescriptorBuffer        = m_instanceBuffer;
    m_descriptor.instanceCount                   = desc.Instances.NumElements( );
    m_descriptor.instanceDescriptorType          = MTLAccelerationStructureInstanceDescriptorTypeUserID;
    if ( m_desc.BuildFlags.IsSet( ASBuildFlags::AllowUpdate ) )
    {
        m_descriptor.usage |= MTLAccelerationStructureUsageRefit;
    }
    if ( m_desc.BuildFlags.IsSet( ASBuildFlags::FastBuild ) )
    {
        m_descriptor.usage = MTLAccelerationStructureUsagePreferFastBuild;
    }

    MTLAccelerationStructureSizes asSize = [m_context->Device accelerationStructureSizesWithDescriptor:m_descriptor];

    m_scratch = [m_context->Device newBufferWithLength:asSize.buildScratchBufferSize options:MTLResourceStorageModePrivate];
    [m_scratch setLabel:@"Top Level Acceleration Structure Scratch"];
    m_indirectResources.push_back( m_scratch );

    m_accelerationStructure = [context->Device newAccelerationStructureWithSize:asSize.accelerationStructureSize];
    [m_accelerationStructure setLabel:@"Top Level Acceleration Structure"];
    m_indirectResources.push_back( m_accelerationStructure );

    m_headerBuffer = [m_context->Device newBufferWithLength:sizeof( IRRaytracingAccelerationStructureGPUHeader ) + sizeof( uint32_t ) * m_contributionsToHitGroupIndices.size( )
                                                    options:MTLResourceStorageModeShared];
    [m_headerBuffer setLabel:@"Top Level Acceleration Structure Header Buffer"];

    IRRaytracingSetAccelerationStructure( (uint8_t *)m_headerBuffer.contents, m_accelerationStructure.gpuResourceID,
                                          (uint8_t *)m_headerBuffer.contents + sizeof( IRRaytracingAccelerationStructureGPUHeader ), m_contributionsToHitGroupIndices.data( ),
                                          m_contributionsToHitGroupIndices.size( ) );
    auto pHdr                            = (IRRaytracingAccelerationStructureGPUHeader *)( m_headerBuffer.contents );
    pHdr->addressOfInstanceContributions = m_headerBuffer.gpuAddress + sizeof( IRRaytracingAccelerationStructureGPUHeader );
    m_indirectResources.push_back( m_headerBuffer );
}

id<MTLAccelerationStructure> MetalTopLevelAS::AccelerationStructure( ) const
{
    return m_accelerationStructure;
}

id<MTLBuffer> MetalTopLevelAS::HeaderBuffer( ) const
{
    return m_headerBuffer;
}

id<MTLBuffer> MetalTopLevelAS::Scratch( ) const
{
    return m_scratch;
}

size_t MetalTopLevelAS::NumInstances( ) const
{
    return m_desc.Instances.NumElements( );
}

id<MTLBuffer> MetalTopLevelAS::InstanceBuffer( ) const
{
    return m_instanceBuffer;
}

MTLAccelerationStructureUserIDInstanceDescriptor *MetalTopLevelAS::InstanceDescriptors( )
{
    return m_instanceDescriptors;
}

MTLAccelerationStructureDescriptor *MetalTopLevelAS::Descriptor( )
{
    return m_descriptor;
}

const std::vector<id<MTLResource>> &MetalTopLevelAS::IndirectResources( ) const
{
    return m_indirectResources;
}

void MetalTopLevelAS::UpdateInstanceTransforms( const UpdateTransformsDesc &desc )
{
    for ( size_t i = 0; i < desc.Transforms.NumElements( ); ++i )
    {
        const float                                      *transformData = desc.Transforms.GetElement( i ).Data( );
        MTLAccelerationStructureUserIDInstanceDescriptor *instance      = &m_instanceDescriptors[ i ];

        for ( int row = 0; row < 3; ++row )
        {
            for ( int col = 0; col < 4; ++col )
            {
                instance->transformationMatrix.columns[ col ][ row ] = transformData[ row * 4 + col ];
            }
        }
    }
}
