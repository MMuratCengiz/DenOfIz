/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IInputLayout.h>
#include "VulkanContext.h"
#include "VulkanEnumConverter.h"

namespace DenOfIz
{

    class VulkanInputLayout final : public IInputLayout
    {
    private:
        VkPipelineVertexInputStateCreateInfo           m_vertexInputState{ };
        std::vector<VkVertexInputBindingDescription>   m_bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

    public:
        explicit VulkanInputLayout( const DenOfIz_InputLayoutDesc &inputLayoutDesc );
        [[nodiscard]] const VkPipelineVertexInputStateCreateInfo &GetVertexInputState( ) const;
        ~VulkanInputLayout( ) override = default;
    };

} // namespace DenOfIz
