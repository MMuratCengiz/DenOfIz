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
#include "DenOfIzCore/Utilities.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h"

using namespace DenOfIz;

VulkanBufferResource::VulkanBufferResource( VulkanContext *context, BufferDesc desc ) : IBufferResource( desc ), m_desc( std::move( desc ) ), m_context( context )
{
    m_numBytes = Utilities::Align( m_desc.NumBytes, m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment );

    VkBufferCreateInfo bufferCreateInfo{ };
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage       = VulkanEnumConverter::ConvertBufferUsage( m_desc.Descriptor, m_desc.InitialState );
    bufferCreateInfo.size        = m_numBytes;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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

    bufferCreateInfo.usage = VulkanEnumConverter::ConvertBufferUsage( m_desc.Descriptor, m_desc.InitialState );
    VmaAllocationInfo allocationInfo;
    vmaCreateBuffer( m_context->Vma, &bufferCreateInfo, &allocationCreateInfo, &m_instance, &m_allocation, &allocationInfo );

    m_offset   = 0; // This used to be allocationInfo.offset, but seems to be applied automatically in almost every case, not sure if required.
    m_numBytes = allocationInfo.size;
}

void *VulkanBufferResource::MapMemory( )
{
    DZ_ASSERTM( m_desc.HeapType == HeapType::CPU_GPU || m_desc.HeapType == HeapType::CPU, "Can only map to CPU visible buffer" );
    DZ_ASSERTM( m_mappedMemory == nullptr, std::format( "Memory already mapped {}", m_desc.DebugName ) );
    vmaMapMemory( m_context->Vma, m_allocation, &m_mappedMemory );
    return m_mappedMemory;
}

void VulkanBufferResource::UnmapMemory( )
{
    DZ_ASSERTM( m_mappedMemory != nullptr, std::format( "Memory not mapped, buffer: {}", m_desc.DebugName ) );
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
