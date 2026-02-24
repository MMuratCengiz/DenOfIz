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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include <utility>
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

using namespace DenOfIz;

VulkanBuffer::VulkanBuffer( VulkanContext *context, DenOfIz_BufferDesc desc ) :
    m_desc( std::move( desc ) ), m_debugName( StringViewToStdString( m_desc.DebugName ) ), m_context( context )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    uint32_t alignment = m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment;
    if ( ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT ) || m_desc.StructureDesc.Stride > 0 )
    {
        alignment = m_context->SelectedDeviceInfo.Constants.StorageBufferAlignment;
    }
    alignment  = std::max<uint32_t>( alignment, m_desc.StructureDesc.Stride );
    m_numBytes = Utilities::Align( m_desc.NumBytes, alignment );

    VkBufferCreateInfo bufferCreateInfo{ };
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage       = DenOfIz_VulkanEnumConverter_ConvertBufferUsageToVk( m_desc.Usage );
    bufferCreateInfo.size        = m_numBytes;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if ( m_desc.StructureDesc.Stride > 0 )
    {
        bufferCreateInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    VmaAllocationCreateInfo allocationCreateInfo{ };

    allocationCreateInfo.usage = DenOfIz_VulkanEnumConverter_ConvertHeapType( m_desc.HeapType );

    if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_CPU )
    {
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    else if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_CPU_GPU )
    {
        allocationCreateInfo.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_GPU )
    {
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    else if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_GPU_CPU )
    {
        allocationCreateInfo.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
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

void *VulkanBuffer::MapMemory( )
{
    if ( m_desc.HeapType == DENOFIZ_HEAP_TYPE_GPU )
    {
        spdlog::warn( "Can only map to CPU visible buffer" );
        return nullptr;
    }
    if ( m_mappedMemory != nullptr )
    {
        spdlog::warn( "Memory already mapped before mapping: {}", m_debugName );
        return m_mappedMemory;
    }

    vmaMapMemory( m_context->Vma, m_allocation, &m_mappedMemory );
    return m_mappedMemory;
}

void VulkanBuffer::UnmapMemory( )
{
    if ( m_mappedMemory == nullptr )
    {
        spdlog::warn( "Memory not mapped before unmapping: {}", m_debugName );
        return;
    }
    vmaUnmapMemory( m_context->Vma, m_allocation );
    m_mappedMemory = nullptr;
}

VulkanBuffer::~VulkanBuffer( )
{
    if ( m_mappedMemory != nullptr )
    {
        spdlog::warn( "Memory not unmapped before lifetime of the buffer." );
        vmaUnmapMemory( m_context->Vma, m_allocation );
    }

    vmaDestroyBuffer( m_context->Vma, m_instance, m_allocation );
}

DenOfIz_ByteArray VulkanBuffer::GetData( ) const
{
    return { static_cast<Byte *>( m_mappedMemory ), m_numBytes };
}

void VulkanBuffer::SetData( const DenOfIz_ByteArrayView &data, const bool keepMapped )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( m_mappedMemory, data.Elements, data.NumElements );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

void VulkanBuffer::WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( static_cast<Byte *>( m_mappedMemory ) + bufferOffset, data.Elements, data.NumElements );
}

size_t VulkanBuffer::NumBytes( ) const
{
    return m_numBytes;
}

const void *VulkanBuffer::Data( ) const
{
    return m_mappedMemory;
}

size_t VulkanBuffer::Offset( ) const
{
    return m_offset;
}

const VkBuffer &VulkanBuffer::Instance( ) const
{
    return m_instance;
}

const VkDeviceAddress &VulkanBuffer::DeviceAddress( ) const
{
    return m_deviceAddress;
}
