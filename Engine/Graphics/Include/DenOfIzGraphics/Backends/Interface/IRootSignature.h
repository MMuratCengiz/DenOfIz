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
#include <DenOfIzCore/Utilities.h>
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
        std::vector<ShaderStage>   Stages;
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
        RootSignatureType Type;
        // The order of the bindings must match the order of the shader inputs!!! TODO might need to be fixed but this is normal for DX12
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

    class IRootSignature : public NonCopyable
    {
    protected:
        struct RegisterSpaceOrder
        {
            uint32_t                                  Space;
            uint32_t                                  ResourceCount;
            uint32_t                                  SamplerCount;
            std::unordered_map<std::string, uint32_t> ResourceOffsetMap;
        };
        std::vector<RegisterSpaceOrder>                              m_registerSpaceOrder;
        std::unordered_map<std::string, ResourceBindingDesc>         m_resourceBindingMap;
        std::unordered_map<std::string, RootConstantResourceBinding> m_rootConstantMap;
        bool                                                         m_created = false;

    public:
        virtual ~IRootSignature() = default;

        inline uint32_t GetResourceOffset(uint32_t registerSpace, std::string name) const
        {
            if ( registerSpace >= m_registerSpaceOrder.size() )
            {
                LOG(ERROR) << "Register space " << registerSpace << " is not bound to any bind group.";
            }

            return SafeGetMapValue(m_registerSpaceOrder[ registerSpace ].ResourceOffsetMap, name);
        }

        inline ResourceBindingDesc GetResourceBinding(std::string name) const
        {
            return SafeGetMapValue(m_resourceBindingMap, name);
        }

        inline RootConstantResourceBinding GetRootConstantBinding(std::string name) const
        {
            return SafeGetMapValue(m_rootConstantMap, name);
        }

    protected:
        void AddResourceBinding(const ResourceBindingDesc &binding)
        {
            RegisterSpaceOrder &spaceOrder = Utilities::SafeAt(m_registerSpaceOrder, binding.RegisterSpace);

            if ( binding.Descriptor.IsSet(ResourceDescriptor::Sampler) )
            {
                spaceOrder.ResourceOffsetMap[ binding.Name ] = spaceOrder.SamplerCount++;
            }
            else
            {
                spaceOrder.ResourceOffsetMap[ binding.Name ] = spaceOrder.ResourceCount++;
            }
            // Todo remove and fix vulkan
            m_resourceBindingMap[ binding.Name ] = binding;
            AddResourceBindingInternal(binding);
        }

        void AddRootConstant(const RootConstantResourceBinding &rootConstant)
        {
            m_rootConstantMap[ rootConstant.Name ] = rootConstant;
            AddRootConstantInternal(rootConstant);
        }

        virtual void AddResourceBindingInternal(const ResourceBindingDesc &binding)           = 0;
        virtual void AddRootConstantInternal(const RootConstantResourceBinding &rootConstant) = 0;

    private:
        template <typename R>
        R SafeGetMapValue(const std::unordered_map<std::string, R> &map, const std::string &key) const
        {
            auto value = map.find(key);
            if ( value == map.end() )
            {
                LOG(ERROR) << "Unable to find key: " << key << ". Make sure the name described in the RootSignature matches the resource name.";
            }
            return value->second;
        }
    };

} // namespace DenOfIz
