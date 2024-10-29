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

MetalTopLevelAS::MetalTopLevelAS( MetalContext *context, const TopLevelASDesc &desc ) : m_context( context )
{
    id<MTLBuffer> instanceBuffer = createInstanceBuffer( );
    m_instanceDescriptors        = (MTLAccelerationStructureInstanceDescriptor *)instanceBuffer.contents;

    for ( size_t i = 0; i < desc.Instances.NumElements( ); ++i )
    {
        const ASInstanceDesc &instanceDesc = desc.Instances.GetElement( i );
        MetalBottomLevelAS   *blas         = dynamic_cast<MetalBottomLevelAS *>( instanceDesc.BLAS );

        if ( blas == nullptr )
        {
            LOG( WARNING ) << "BLAS is null.";
            continue;
        }

        MTLAccelerationStructureInstanceDescriptor *instance = &m_instanceDescriptors[ i ];
        instance->intersectionFunctionTableOffset            = 0;
        instance->accelerationStructureIndex                 = i;
        instance->options                                    = blas->Options( );
        instance->mask                                       = instanceDesc.Mask;
        memcpy( instance->transformationMatrix.columns, instanceDesc.Transform.Data( ), 12 * sizeof( float ) );

        m_contributionsToHitGroupIndices.emplace_back( instanceDesc.ContributionToHitGroupIndex );
    }

    BufferDesc headerBufferDesc = { };
    headerBufferDesc.HeapType   = HeapType::CPU_GPU;
    headerBufferDesc.NumBytes   = sizeof( IRRaytracingAccelerationStructureGPUHeader ) + sizeof( uint32_t ) * m_contributionsToHitGroupIndices.size( );
    headerBufferDesc.Descriptor = BitSet( ResourceDescriptor::Buffer );
    m_headerBuffer              = std::make_unique<MetalBufferResource>( m_context, headerBufferDesc );

    uint8_t *headerBufferContents = (uint8_t *)m_headerBuffer->Instance( ).contents;
    IRRaytracingSetAccelerationStructure( headerBufferContents, m_accelerationStructure.gpuResourceID, headerBufferContents + sizeof( IRRaytracingAccelerationStructureGPUHeader ),
                                          m_contributionsToHitGroupIndices.data( ), m_contributionsToHitGroupIndices.size( ) );
    // Acceleration Structure Configuration
    m_descriptor                          = [MTLInstanceAccelerationStructureDescriptor descriptor];
    m_descriptor.instanceDescriptorBuffer = createInstanceBuffer( );
    m_descriptor.instanceCount            = desc.Instances.NumElements( );
    m_descriptor.usage                    = MTLAccelerationStructureUsagePreferFastBuild;

    MTLAccelerationStructureSizes asSize = [m_context->Device accelerationStructureSizesWithDescriptor:m_descriptor];

    BufferDesc bufferDesc   = { };
    bufferDesc.Descriptor   = BitSet( ResourceDescriptor::RWBuffer ) | ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.NumBytes     = asSize.accelerationStructureSize;
    bufferDesc.InitialUsage = ResourceUsage::AccelerationStructureWrite;
    bufferDesc.DebugName    = "Top Level Acceleration Structure";
    m_buffer                = std::make_unique<MetalBufferResource>( m_context, bufferDesc );

    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.HeapType     = HeapType::GPU;
    scratchBufferDesc.NumBytes     = asSize.buildScratchBufferSize;
    scratchBufferDesc.Descriptor   = BitSet( ResourceDescriptor::RWBuffer );
    scratchBufferDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    scratchBufferDesc.DebugName    = "Top Level Acceleration Structure Scratch";
    m_scratch                      = std::make_unique<MetalBufferResource>( m_context, scratchBufferDesc );

    m_accelerationStructure = [context->Device newAccelerationStructureWithSize:asSize.accelerationStructureSize];
}

id<MTLBuffer> MetalTopLevelAS::createInstanceBuffer( )
{
    BufferDesc instanceDescBuffer   = { };
    instanceDescBuffer.HeapType     = HeapType::GPU;
    instanceDescBuffer.NumBytes     = sizeof( MTLAccelerationStructureInstanceDescriptor ) * m_desc.Instances.NumElements( );
    instanceDescBuffer.Descriptor   = BitSet( ResourceDescriptor::RWBuffer );
    instanceDescBuffer.InitialUsage = ResourceUsage::AccelerationStructureWrite;
    instanceDescBuffer.DebugName    = "Instance Descriptor Buffer";

    m_instanceBuffer                  = std::make_unique<MetalBufferResource>( m_context, instanceDescBuffer );
    id<MTLBuffer> instanceBufferMetal = m_instanceBuffer->Instance( );
    return m_instanceBuffer->Instance( );
}

id<MTLAccelerationStructure> MetalTopLevelAS::AccelerationStructure( ) const
{
    return m_accelerationStructure;
}

MetalBufferResource *MetalTopLevelAS::MetalBuffer( ) const
{
    return m_buffer.get( );
}

IBufferResource *MetalTopLevelAS::Buffer( ) const
{
    return m_buffer.get( );
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
void MetalTopLevelAS::Update( const TopLevelASDesc &desc )
{
    // Re-build or update the instances in the structure based on `desc`
}
