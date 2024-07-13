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

#include <DenOfIzCore/Common.h>
#include "IBufferResource.h"
#include "IShader.h"
#include "ITextureResource.h"

namespace DenOfIz
{

    enum class RootSignatureType
    {
        Graphics,
        Compute
    };

    // Static = 0th set, Dynamic = 1, PerDraw = 2
    // Frequency is mapped 1 to 1 with DX12s RootSignature 'RegisterSpace' and Vulkan's 'Set'
    enum class ResourceUpdateFrequency : uint32_t
    {
        Static  = 0,
        Dynamic = 1,
        PerDraw = 2
    };

    struct ResourceBindingDesc
    {
        std::string                Name;
        uint32_t                   Binding;
        uint32_t                   RegisterSpace = 0;
        BitSet<ResourceDescriptor> Descriptor;
        // A binding can appear in more than one stage, i.e. both in fragment and vertex shaders.
        std::vector<ShaderStage> Stages;
        // 1 is both 'Arr[1]'(Size of 1) and Simply 'Var'(Non array variable)
        int ArraySize = 1;
    };

    struct StaticSamplerDesc
    {
        SamplerDesc         Sampler;
        ResourceBindingDesc Binding;
    };

    struct RootSignatureDesc
    {
        RootSignatureType                Type;
        std::vector<ResourceBindingDesc> ResourceBindings;
        std::vector<StaticSamplerDesc>   StaticSamplers;
    };

    struct RootConstantResourceBinding
    {
        std::string              Name;
        uint32_t                 Binding;
        uint32_t                 RegisterSpace = 0;
        int                      Size;
        std::vector<ShaderStage> Stages;
    };

    class IRootSignature
    {
    protected:
        uint32_t                                                     m_resourceCount = 0;
        std::vector<uint32_t>                                        m_resourceCountPerSet;
        std::unordered_map<std::string, ResourceBindingDesc>         m_resourceBindingMap;
        std::unordered_map<std::string, RootConstantResourceBinding> m_rootConstantMap;
        std::unordered_map<std::string, uint32_t>                    m_indices;
        bool                                                         m_created = false;

    public:
        virtual ~IRootSignature() = default;

        inline uint32_t GetResourceCount() const
        {
            return m_resourceCount;
        }

        inline uint32_t GetResourceIndex(std::string name) const
        {
            return m_indices.at(name);
        }

        inline uint32_t GetResourceCount(uint32_t registerSpace) const
        {
            return m_resourceCountPerSet[ static_cast<uint32_t>(registerSpace) ];
        }

        inline ResourceBindingDesc GetResourceBinding(std::string name) const
        {
            return m_resourceBindingMap.at(name);
        }

        inline RootConstantResourceBinding GetRootConstantBinding(std::string name)
        {
            return m_rootConstantMap.at(name);
        }

    protected:
        void AddResourceBinding(const ResourceBindingDesc &binding)
        {
            if ( m_resourceCountPerSet.size() <= binding.RegisterSpace )
            {
                m_resourceCountPerSet.resize(binding.RegisterSpace + 1);
            }
            m_resourceBindingMap[ binding.Name ] = binding;
            m_resourceCount++;
            m_resourceCountPerSet[ binding.RegisterSpace ] = std::max(m_resourceCountPerSet[ binding.RegisterSpace ], binding.Binding + 1);
            AddResourceBindingInternal(binding);
        }

        void AddRootConstant(const RootConstantResourceBinding &rootConstant)
        {
            m_rootConstantMap[ rootConstant.Name ] = rootConstant;
            AddRootConstantInternal(rootConstant);
        }

    protected:
        virtual void AddResourceBindingInternal(const ResourceBindingDesc &binding)           = 0;
        virtual void AddRootConstantInternal(const RootConstantResourceBinding &rootConstant) = 0;
    };

} // namespace DenOfIz
