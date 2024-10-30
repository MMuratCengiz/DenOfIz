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
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalBottomLevelAS.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalTopLevelAS.h>

using namespace DenOfIz;

MetalTopLevelAS::MetalTopLevelAS( MetalContext *context, const TopLevelASDesc &desc ) : m_context( context ), m_desc( desc )
{
    createInstanceBuffer( );
    id<MTLBuffer> instanceBuffer = m_instanceBuffer->Instance( );
    m_instanceDescriptors        = (MTLAccelerationStructureInstanceDescriptor *)instanceBuffer.contents;
    m_indirectResources.push_back( instanceBuffer );

    m_blasList = [NSMutableArray arrayWithCapacity:desc.Instances.NumElements( )];
    for ( size_t i = 0; i < desc.Instances.NumElements( ); ++i )
    {
        const ASInstanceDesc &instanceDesc = desc.Instances.GetElement( i );
        MetalBottomLevelAS   *blas         = dynamic_cast<MetalBottomLevelAS *>( instanceDesc.BLAS );

        if ( blas == nullptr )
        {
            LOG( WARNING ) << "BLAS is null.";
            continue;
        }

        [m_blasList addObject:blas->AccelerationStructure( )];

        MTLAccelerationStructureInstanceDescriptor *instance = &m_instanceDescriptors[ i ];
        instance->intersectionFunctionTableOffset            = i;
        instance->accelerationStructureIndex                 = i;
        instance->options                                    = blas->Options( );
        instance->mask                                       = instanceDesc.Mask;

        const float *transformData = instanceDesc.Transform.Data( );
        for ( int row = 0; row < 3; ++row )
        {
            for ( int col = 0; col < 4; ++col )
            {
                instance->transformationMatrix.columns[ col ][ row ] = transformData[ row * 4 + col ];
            }
        }
        m_indirectResources.push_back( blas->AccelerationStructure( ) );
        m_indirectResources.push_back( blas->Scratch( )->Instance( ) );

        m_contributionsToHitGroupIndices.emplace_back( instanceDesc.ContributionToHitGroupIndex );
    }

    BufferDesc headerBufferDesc = { };
    headerBufferDesc.HeapType   = HeapType::CPU_GPU;
    headerBufferDesc.NumBytes   = sizeof( IRRaytracingAccelerationStructureGPUHeader ) + sizeof( uint32_t ) * m_contributionsToHitGroupIndices.size( );
    headerBufferDesc.Descriptor = BitSet( ResourceDescriptor::Buffer );
    headerBufferDesc.DebugName  = "Top Level Acceleration Structure Header Buffer";
    m_headerBuffer              = std::make_unique<MetalBufferResource>( m_context, headerBufferDesc );
    m_indirectResources.push_back( m_headerBuffer->Instance( ) );

    uint8_t *headerBufferContents = (uint8_t *)m_headerBuffer->Instance( ).contents;
    IRRaytracingSetAccelerationStructure( headerBufferContents, m_accelerationStructure.gpuResourceID, headerBufferContents + sizeof( IRRaytracingAccelerationStructureGPUHeader ),
                                          m_contributionsToHitGroupIndices.data( ), m_contributionsToHitGroupIndices.size( ) );
    // Acceleration Structure Configuration
    m_descriptor                                 = [MTLInstanceAccelerationStructureDescriptor descriptor];
    m_descriptor.instancedAccelerationStructures = m_blasList;
    m_descriptor.instanceDescriptorBuffer        = instanceBuffer;
    m_descriptor.instanceCount                   = desc.Instances.NumElements( );
    m_descriptor.usage                           = MTLAccelerationStructureUsagePreferFastBuild;

    MTLAccelerationStructureSizes asSize = [m_context->Device accelerationStructureSizesWithDescriptor:m_descriptor];

    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.HeapType     = HeapType::GPU;
    scratchBufferDesc.NumBytes     = asSize.buildScratchBufferSize;
    scratchBufferDesc.Descriptor   = BitSet( ResourceDescriptor::RWBuffer );
    scratchBufferDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    scratchBufferDesc.DebugName    = "Top Level Acceleration Structure Scratch";
    m_scratch                      = std::make_unique<MetalBufferResource>( m_context, scratchBufferDesc );
    m_indirectResources.push_back( m_scratch->Instance( ) );

    m_accelerationStructure = [context->Device newAccelerationStructureWithDescriptor:m_descriptor];
    m_indirectResources.push_back( m_accelerationStructure );
}

void MetalTopLevelAS::createInstanceBuffer( )
{
    BufferDesc instanceDescBuffer   = { };
    instanceDescBuffer.HeapType     = HeapType::CPU_GPU;
    instanceDescBuffer.NumBytes     = sizeof( MTLAccelerationStructureInstanceDescriptor ) * m_desc.Instances.NumElements( );
    instanceDescBuffer.Descriptor   = BitSet( ResourceDescriptor::RWBuffer );
    instanceDescBuffer.InitialUsage = ResourceUsage::AccelerationStructureWrite;
    instanceDescBuffer.DebugName    = "Instance Descriptor Buffer";

    m_instanceBuffer = std::make_unique<MetalBufferResource>( m_context, instanceDescBuffer );
}

id<MTLAccelerationStructure> MetalTopLevelAS::AccelerationStructure( ) const
{
    return m_accelerationStructure;
}

MetalBufferResource *MetalTopLevelAS::HeaderBuffer( ) const
{
    return m_headerBuffer.get( );
}

MetalBufferResource *MetalTopLevelAS::Scratch( ) const
{
    return m_scratch.get( );
}

size_t MetalTopLevelAS::NumInstances( ) const
{
    return m_desc.Instances.NumElements( );
}

const MetalBufferResource *MetalTopLevelAS::InstanceBuffer( ) const
{
    return m_instanceBuffer.get( );
}

MTLAccelerationStructureInstanceDescriptor *MetalTopLevelAS::InstanceDescriptors( )
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

void MetalTopLevelAS::Update( const TopLevelASDesc &desc )
{
    // Re-build or update the instances in the structure based on `desc`
}
