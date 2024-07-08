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

#pragma once

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include "VulkanEnumConverter.h"

namespace DenOfIz
{

    class VulkanRootSignature : public IRootSignature
    {
    private:
        RootSignatureDesc m_desc;
        VulkanContext    *m_context;

        std::vector<vk::DescriptorSetLayout>        m_layouts;
        std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
        std::vector<vk::PushConstantRange>          m_pushConstants;

    public:
        VulkanRootSignature(VulkanContext *context, RootSignatureDesc desc);
        ~VulkanRootSignature();

        const std::vector<vk::DescriptorSetLayout> &GetDescriptorSetLayouts() const
        {
            return m_layouts;
        }

    protected:
        void AddResourceBindingInternal(const ResourceBinding &binding) override;
        void AddRootConstantInternal(const RootConstantBinding &rootConstantBinding) override;

        void CreateInternal() override;
    };
} // namespace DenOfIz
