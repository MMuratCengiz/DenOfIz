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
#include "ReflectionData.h"

#include <ranges>
#include <unordered_set>

namespace DenOfIz
{

    enum class RootSignatureType
    {
        Graphics,
        Compute
    };

    struct ResourceBindingSlot
    {
        uint32_t                    Binding       = 0;
        uint32_t                    RegisterSpace = 0;
        DescriptorBufferBindingType Type          = DescriptorBufferBindingType::ConstantBuffer;

        // To simplify having a really odd looking vector of ResourceBindingSlots
        [[nodiscard]] uint32_t Key( ) const
        {
            return static_cast<uint32_t>( Type ) * 1000 + RegisterSpace * 100 + Binding;
        }

        bool operator==( const ResourceBindingSlot &other ) const
        {
            return Binding == other.Binding && RegisterSpace == other.RegisterSpace && Type == other.Type;
        }

        [[nodiscard]] std::string ToString( ) const
        {
            std::string typeString;
            switch ( Type )
            {
            case DescriptorBufferBindingType::ConstantBuffer:
                typeString = "b";
                break;
            case DescriptorBufferBindingType::ShaderResource:
                typeString = "t";
                break;
            case DescriptorBufferBindingType::UnorderedAccess:
                typeString = "u";
                break;
            case DescriptorBufferBindingType::Sampler:
                typeString = "s";
                break;
            }
            return "(" + typeString + std::to_string( Binding ) + ", space" + std::to_string( RegisterSpace )  + ")";
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
        int                         ArraySize = 1; // 1 is both 'Arr[1]'(Size of 1) and Simply 'Var'(Non array variable)
        ReflectionDesc              Reflection{ };
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
        std::vector<ResourceBindingSlot>                  m_requiredBindings;

    public:
        explicit IRootSignature( const RootSignatureDesc &desc )
        {
            for ( const auto &binding : desc.ResourceBindings )
            {
                ResourceBindingSlot slot{
                    .Binding       = binding.Binding,
                    .RegisterSpace = binding.RegisterSpace,
                    .Type          = binding.BindingType,
                };
                m_resourceBindings[ slot.Key( ) ] = binding;
                m_requiredBindings.push_back( slot );
            }
        }

        [[nodiscard]] std::vector<ResourceBindingSlot> Bindings( ) const
        {
            return m_requiredBindings;
        }

        [[nodiscard]] const ResourceBindingDesc &FindBinding( const ResourceBindingSlot &slot )
        {
            const auto it = m_resourceBindings.find( slot.Key( ) );
            if ( it == m_resourceBindings.end( ) )
            {
                LOG( ERROR ) << "Unable to find slot with type[" << static_cast<int>( slot.Type ) << "],binding[" << slot.Binding << "],register[" << slot.RegisterSpace << "].";
            }
            return it->second;
        }

        virtual ~IRootSignature( ) = default;
    };

} // namespace DenOfIz
