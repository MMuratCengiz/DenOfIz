/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h>

#include <utility>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphics/Utilities/Utilities.h"

using namespace DenOfIz;

VulkanBufferResource::VulkanBufferResource( VulkanContext *context, BufferDesc desc ) : m_desc( std::move( desc ) ), m_context( context )
{
    uint32_t alignment = m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment;
    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::StructuredBuffer ) )
    {
        alignment = m_context->SelectedDeviceInfo.Constants.StorageBufferAlignment;
    }
    m_numBytes                  = Utilities::Align( m_desc.NumBytes, alignment );
    BitSet<ResourceUsage> usage = m_desc.Usages;
    usage |= m_desc.InitialUsage;

    VkBufferCreateInfo bufferCreateInfo{ };
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage       = VulkanEnumConverter::ConvertBufferUsage( m_desc.Descriptor, usage );
    bufferCreateInfo.size        = m_numBytes;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo allocationCreateInfo{ };

    allocationCreateInfo.usage = VulkanEnumConverter::ConvertHeapType( m_desc.HeapType );

    // Todo more flexibility, or a clearer interface here:
    if ( m_desc.HeapType == HeapType::CPU || m_desc.HeapType == HeapType::CPU_GPU )
    {
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    if ( m_desc.HeapType == HeapType::CPU_GPU )
    {
        allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    if ( m_desc.HeapType == HeapType::GPU || m_desc.HeapType == HeapType::GPU_CPU )
    {
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    if ( m_desc.HeapType == HeapType::GPU_CPU )
    {
        allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    VmaAllocationInfo allocationInfo;
    vmaCreateBuffer( m_context->Vma, &bufferCreateInfo, &allocationCreateInfo, &m_instance, &m_allocation, &allocationInfo );

    m_offset   = 0; // This used to be allocationInfo.offset, but seems to be applied automatically in almost every case, not sure if required.
    m_numBytes = allocationInfo.size;

    VkBufferDeviceAddressInfo bufferDeviceAddressInfo{ };
    bufferDeviceAddressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = m_instance;
    m_deviceAddress                = vkGetBufferDeviceAddress( m_context->LogicalDevice, &bufferDeviceAddressInfo );
}

void *VulkanBufferResource::MapMemory( )
{
    DZ_ASSERTM( m_desc.HeapType == HeapType::CPU_GPU || m_desc.HeapType == HeapType::CPU, "Can only map to CPU visible buffer" );
    DZ_ASSERTM( m_mappedMemory == nullptr, std::format( "Memory already mapped {}", m_desc.DebugName.Get( ) ) );
    vmaMapMemory( m_context->Vma, m_allocation, &m_mappedMemory );
    return m_mappedMemory;
}

void VulkanBufferResource::UnmapMemory( )
{
    DZ_ASSERTM( m_mappedMemory != nullptr, std::format( "Memory not mapped, buffer: {}", m_desc.DebugName.Get( ) ) );
    vmaUnmapMemory( m_context->Vma, m_allocation );
    m_mappedMemory = nullptr;
}

VulkanBufferResource::~VulkanBufferResource( )
{
    if ( m_mappedMemory != nullptr )
    {
        LOG( WARNING ) << "Memory not unmapped before lifetime of the buffer.";
        vmaUnmapMemory( m_context->Vma, m_allocation );
    }

    vmaDestroyBuffer( m_context->Vma, m_instance, m_allocation );
}

InteropArray<Byte> VulkanBufferResource::GetData( ) const
{
    InteropArray<Byte> data( m_numBytes );
    std::memcpy( data.Data( ), m_mappedMemory, m_numBytes );
    return std::move( data );
}

void VulkanBufferResource::SetData( const InteropArray<Byte> &data, const bool keepMapped )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( m_mappedMemory, data.Data( ), data.NumElements( ) );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

void VulkanBufferResource::WriteData( const InteropArray<Byte> &data, uint32_t bufferOffset )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( static_cast<Byte *>( m_mappedMemory ) + bufferOffset, data.Data( ), data.NumElements( ) );
}

BitSet<ResourceUsage> VulkanBufferResource::InitialState( ) const
{
    return m_state;
}

size_t VulkanBufferResource::NumBytes( ) const
{
    return m_numBytes;
}

const void *VulkanBufferResource::Data( ) const
{
    return m_mappedMemory;
}

size_t VulkanBufferResource::Offset( ) const
{
    return m_offset;
}

const VkBuffer &VulkanBufferResource::Instance( ) const
{
    return m_instance;
}

const VkDeviceAddress &VulkanBufferResource::DeviceAddress( ) const
{
    return m_deviceAddress;
}
