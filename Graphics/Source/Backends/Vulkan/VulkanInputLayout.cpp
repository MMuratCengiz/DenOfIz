/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanInputLayout.h"

using namespace DenOfIz;

VulkanInputLayout::VulkanInputLayout( const DenOfIz_InputLayoutDesc &inputLayoutDesc )
{
    // Locations follow the declaration order of the inputs. TODO: --fvk-stage-io-order=alpha in ShaderCompiler would make this robust.
    uint32_t location = 0;
    for ( uint32_t bindingIndex = 0; bindingIndex < inputLayoutDesc.InputGroups.NumElements; bindingIndex++ )
    {
        const DenOfIz_InputGroupDesc    &inputGroup         = inputLayoutDesc.InputGroups.Elements[ bindingIndex ];
        VkVertexInputBindingDescription &bindingDescription = m_bindingDescriptions.emplace_back( VkVertexInputBindingDescription{ } );
        bindingDescription.binding                          = bindingIndex;
        bindingDescription.inputRate                        = inputGroup.StepRate == DENOFIZ_STEP_RATE_PER_INSTANCE ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

        uint32_t offset = 0;
        for ( uint32_t inputElementIndex = 0; inputElementIndex < inputGroup.Elements.NumElements; inputElementIndex++ )
        {
            const DenOfIz_InputLayoutElementDesc &inputElement         = inputGroup.Elements.Elements[ inputElementIndex ];
            VkVertexInputAttributeDescription    &attributeDescription = m_attributeDescriptions.emplace_back( VkVertexInputAttributeDescription{ } );
            attributeDescription.binding                               = bindingIndex;
            attributeDescription.location                              = location++;
            attributeDescription.format                                = DenOfIz_VulkanEnumConverter_ConvertImageFormat( inputElement.Format );
            attributeDescription.offset                                = offset;
            offset += DenOfIz_Format_NumBytes( inputElement.Format );
        }
        bindingDescription.stride = inputGroup.Stride > 0 ? inputGroup.Stride : offset;
    }

    m_vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_vertexInputState.vertexBindingDescriptionCount   = static_cast<uint32_t>( m_bindingDescriptions.size( ) );
    m_vertexInputState.pVertexBindingDescriptions      = m_bindingDescriptions.data( );
    m_vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>( m_attributeDescriptions.size( ) );
    m_vertexInputState.pVertexAttributeDescriptions    = m_attributeDescriptions.data( );

    if ( m_vertexInputState.vertexBindingDescriptionCount == 0 )
    {
        m_vertexInputState.pVertexBindingDescriptions = nullptr;
    }
    if ( m_vertexInputState.vertexAttributeDescriptionCount == 0 )
    {
        m_vertexInputState.pVertexAttributeDescriptions = nullptr;
    }
}

const VkPipelineVertexInputStateCreateInfo &VulkanInputLayout::GetVertexInputState( ) const
{
    return m_vertexInputState;
}
