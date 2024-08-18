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
#include <DenOfIzCore/ContainerUtilities.h>
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

    struct ResourceBindingSlot
    {
        uint32_t                    Binding  = 0;
        uint32_t                    Register = 0;
        DescriptorBufferBindingType Type     = DescriptorBufferBindingType::ConstantBuffer;

        static ResourceBindingSlot Cbv( const uint32_t binding = 0, const uint32_t reg = 0 )
        {
            return ResourceBindingSlot{ binding, reg, DescriptorBufferBindingType::ConstantBuffer };
        }

        static ResourceBindingSlot Uav( const uint32_t binding = 0, const uint32_t reg = 0 )
        {
            return ResourceBindingSlot{ binding, reg, DescriptorBufferBindingType::UnorderedAccess };
        }

        static ResourceBindingSlot Srv( const uint32_t binding = 0, const uint32_t reg = 0 )
        {
            return ResourceBindingSlot{ binding, reg, DescriptorBufferBindingType::ShaderResource };
        }

        static ResourceBindingSlot Sampler( const uint32_t binding = 0, const uint32_t reg = 0 )
        {
            return ResourceBindingSlot{ binding, reg, DescriptorBufferBindingType::Sampler };
        }

        // To simplify having a really odd looking vector of ResourceBindingSlots
        [[nodiscard]] uint32_t Key( ) const
        {
            return static_cast<uint32_t>( Type ) * 1000 + Register * 100 + Binding;
        }
    };

    struct ResourceBindingDesc
    {
        std::string                 Name;
        DescriptorBufferBindingType BindingType = DescriptorBufferBindingType::ConstantBuffer;
        uint32_t                    Binding{ };
        uint32_t                    RegisterSpace = 0;
        BitSet<ResourceDescriptor>  Descriptor;
        std::vector<ShaderStage>    Stages;
        // 1 is both 'Arr[1]'(Size of 1) and Simply 'Var'(Non array variable)
        int ArraySize = 1;
        // Metal workaround since it is a bit hard to migrate HLSL bindings to Metal.
        // This is normally set by the compiler, If this field is not set then(because you manually built the ResourceBinding)
        // This may not always be correct, this is okay to be set manually to fix such issues, using the ShaderCompiler
        // to generate the locations and overwriting other fields
        uint32_t LocationHint = 0;
    };

    struct StaticSamplerDesc
    {
        SamplerDesc         Sampler;
        ResourceBindingDesc Binding;
    };

    struct RootConstantResourceBinding
    {
        std::string              Name;
        uint32_t                 Binding{ };
        uint32_t                 RegisterSpace = 0;
        int                      Size{ };
        std::vector<ShaderStage> Stages;
    };

    struct RootSignatureDesc
    {
        RootSignatureType Type;
        // The order of the bindings must match the order of the shader inputs!!! TODO might need to be fixed but this is normal for DX12
        std::vector<ResourceBindingDesc>         ResourceBindings;
        std::vector<StaticSamplerDesc>           StaticSamplers;
        std::vector<RootConstantResourceBinding> RootConstants;
    };

    class IRootSignature : public NonCopyable
    {
    protected:
        std::unordered_map<uint32_t, ResourceBindingDesc> m_resourceBindings;

    public:
        explicit IRootSignature( const RootSignatureDesc &desc )
        {
            for ( const auto &binding : desc.ResourceBindings )
            {
                ResourceBindingSlot slot{
                    .Binding  = binding.Binding,
                    .Register = binding.RegisterSpace,
                    .Type     = binding.BindingType,
                };
                m_resourceBindings[ slot.Key( ) ] = binding;
            }
        };

        const ResourceBindingDesc &FindBinding( const ResourceBindingSlot &slot )
        {
            const auto it = m_resourceBindings.find( slot.Key( ) );
            if ( it == m_resourceBindings.end( ) )
            {
                LOG( ERROR ) << "Unable to find slot with type[" << static_cast<int>( slot.Type ) << "],binding[" << slot.Binding << "],register[" << slot.Register << "].";
            }
            return it->second;
        }

        virtual ~IRootSignature( ) = default;
    };

} // namespace DenOfIz
