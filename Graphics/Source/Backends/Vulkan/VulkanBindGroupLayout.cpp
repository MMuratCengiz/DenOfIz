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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBindGroupLayout.h"

#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/CommonDataUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanBindGroupLayout::VulkanBindGroupLayout( VulkanContext *context, const DenOfIz_BindGroupLayoutDesc &desc ) : IBindGroupLayout( desc ), m_context( context ), m_desc( desc )
{
    for ( uint32_t i = 0; i < desc.Bindings.NumElements; ++i )
    {
        AddResourceBinding( desc.Bindings.Elements[ i ] );
    }

    if ( !m_layoutBindings.empty( ) )
    {
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{ };
        bindingFlagsInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount  = m_bindingFlags.size( );
        bindingFlagsInfo.pBindingFlags = m_bindingFlags.data( );

        VkDescriptorSetLayoutCreateInfo layoutInfo{ };
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext        = &bindingFlagsInfo;
        layoutInfo.bindingCount = m_layoutBindings.size( );
        layoutInfo.pBindings    = m_layoutBindings.data( );
        layoutInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &layoutInfo, nullptr, &m_layout ) );
    }
}

VulkanBindGroupLayout::~VulkanBindGroupLayout( )
{
    if ( m_layout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout( m_context->LogicalDevice, m_layout, nullptr );
    }
}

void VulkanBindGroupLayout::AddResourceBinding( const DenOfIz_BindingDesc &binding )
{
    const DenOfIz_ResourceBindingSlot slot{
        .Type          = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor ),
        .Binding       = binding.Binding,
        .RegisterSpace = m_desc.RegisterSpace,
    };

    const uint32_t slotKey          = DenOfIz_ResourceBindingSlot_Key( &slot );
    m_resourceBindingMap[ slotKey ] = binding;

    const VkDescriptorSetLayoutBinding layoutBinding = CreateDescriptorSetLayoutBinding( binding );
    m_layoutBindings.push_back( layoutBinding );
}

VkDescriptorSetLayoutBinding VulkanBindGroupLayout::CreateDescriptorSetLayoutBinding( const DenOfIz_BindingDesc &binding )
{
    VkDescriptorSetLayoutBinding layoutBinding{ };
    uint32_t                     bindingTypeOffset = 0;
    switch ( DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor ) )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        bindingTypeOffset = DENOFIZ_VK_SHIFT_CBV;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        bindingTypeOffset = DENOFIZ_VK_SHIFT_SRV;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        bindingTypeOffset = DENOFIZ_VK_SHIFT_UAV;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        bindingTypeOffset = DENOFIZ_VK_SHIFT_SAMPLER;
        break;
    }

    layoutBinding.binding         = bindingTypeOffset + binding.Binding;
    layoutBinding.descriptorType  = DenOfIz_VulkanEnumConverter_ConvertResourceDescriptorToDescriptorType( binding.Descriptor );
    layoutBinding.descriptorCount = binding.ArraySize;
    layoutBinding.stageFlags      = DenOfIz_VulkanEnumConverter_ConvertShaderStage( binding.Stages );

    VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    if ( binding.ArraySize > 1 )
    {
        bindingFlags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    }
    m_bindingFlags.push_back( bindingFlags );

    const DenOfIz_ResourceBindingSlot offsetSlot{
        .Type          = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor ),
        .Binding       = binding.Binding,
        .RegisterSpace = m_desc.RegisterSpace,
    };
    const uint32_t offsetSlotKey                  = DenOfIz_ResourceBindingSlot_Key( &offsetSlot );
    m_resourceBindingMap[ offsetSlotKey ].Binding = layoutBinding.binding;
    return layoutBinding;
}

const DenOfIz_BindGroupLayoutDesc &VulkanBindGroupLayout::Desc( ) const
{
    return m_desc;
}

uint32_t VulkanBindGroupLayout::RegisterSpace( ) const
{
    return m_desc.RegisterSpace;
}

VkDescriptorSetLayout VulkanBindGroupLayout::DescriptorSetLayout( ) const
{
    return m_layout;
}

DenOfIz_BindingDesc VulkanBindGroupLayout::GetVkShiftedBinding( const DenOfIz_ResourceBindingSlot &slot ) const
{
    const uint32_t slotKey = DenOfIz_ResourceBindingSlot_Key( &slot );
    return ContainerUtilities::SafeGetMapValue( m_resourceBindingMap, slotKey, "Binding slot does not exist in bind group layout: " + ResourceBindingSlotToString( slot ) );
}

bool VulkanBindGroupLayout::HasBindings( ) const
{
    return !m_layoutBindings.empty( );
}
