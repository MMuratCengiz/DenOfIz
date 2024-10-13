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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanInputLayout.h>

using namespace DenOfIz;

VulkanInputLayout::VulkanInputLayout( const InputLayoutDesc &inputLayoutDesc )
{
    // TODO: !IMPROVEMENT! --fvk-stage-io-order=alpha should be used as a proper solution, but it is not implemented yet. Check ShaderCompiler.CompileHLSL.
    uint32_t location = 0;
    // TODO: !IMPROVEMENT! Does multiple input groups work
    for ( int bindingIndex = 0; bindingIndex < inputLayoutDesc.NumInputGroups; bindingIndex++ )
    {
        const InputGroupDesc            &inputGroup         = inputLayoutDesc.InputGroups[ bindingIndex ];
        VkVertexInputBindingDescription &bindingDescription = m_bindingDescriptions.emplace_back( VkVertexInputBindingDescription{ } );
        bindingDescription.binding                          = bindingIndex;
        bindingDescription.inputRate                        = inputGroup.StepRate == StepRate::PerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

        uint32_t offset = 0;
        for ( int inputElementIndex = 0; inputElementIndex < inputGroup.NumElements; inputElementIndex++ )
        {
            const InputLayoutElementDesc      &inputElement         = inputGroup.Elements[ inputElementIndex ];
            VkVertexInputAttributeDescription &attributeDescription = m_attributeDescriptions.emplace_back( VkVertexInputAttributeDescription{ } );
            attributeDescription.binding                            = bindingIndex;
            attributeDescription.location                           = location++; // Is this correct? !CHECK_VK!
            attributeDescription.format                             = VulkanEnumConverter::ConvertImageFormat( inputElement.Format );
            attributeDescription.offset                             = offset;
            offset += FormatNumBytes( inputElement.Format );
        }
        bindingDescription.stride = offset;
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
