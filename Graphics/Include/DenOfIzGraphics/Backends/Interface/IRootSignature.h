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

#include <DenOfIzGraphics/Utilities/Common.h>
#include <DenOfIzGraphics/Utilities/ContainerUtilities.h>
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
            return "(" + typeString + std::to_string( Binding ) + ", space" + std::to_string( RegisterSpace ) + ")";
        }
    };

#define DZ_MAX_SHADER_STAGES 5
    struct ShaderStages
    {
        size_t      NumElements = 0;
        ShaderStage Array[ DZ_MAX_SHADER_STAGES ];
    };

    struct ResourceBindingDesc
    {
        std::string                 Name;
        DescriptorBufferBindingType BindingType = DescriptorBufferBindingType::ConstantBuffer;
        uint32_t                    Binding{ };
        uint32_t                    RegisterSpace = 0;
        BitSet<ResourceDescriptor>  Descriptor;
        ShaderStages                Stages;
        int                         ArraySize = 1; // 1 is both 'Arr[1]'(Size of 1) and Simply 'Var'(Non array variable)
        ReflectionDesc              Reflection{ };
    };

    struct StaticSamplerDesc
    {
        SamplerDesc         Sampler;
        ResourceBindingDesc Binding;
    };

    // For cross api compatibility the RegisterSpace is hardcoded to 99, make sure to use the same value in the HLSL Shader
    struct RootConstantResourceBindingDesc
    {
        std::string    Name;
        uint32_t       Binding{ };
        int            NumBytes{ };
        ShaderStages   Stages;
        ReflectionDesc Reflection{ };
    };

#define DZ_MAX_ROOT_CONSTANTS 5
    struct RootConstantBindings
    {
        size_t                          NumElements = 0;
        RootConstantResourceBindingDesc Array[ DZ_MAX_ROOT_CONSTANTS ];
    };
#define DZ_MAX_RESOURCE_BINDINGS 32
    struct ResourceBindings
    {
        size_t              NumElements = 0;
        ResourceBindingDesc Array[ DZ_MAX_RESOURCE_BINDINGS ];
    };

    struct RootSignatureDesc
    {
        RootSignatureType Type;
        // The order of the bindings must match the order of the shader inputs!!! TODO might need to be fixed but this is normal for DX12
        ResourceBindings                     ResourceBindings;
        const std::vector<StaticSamplerDesc> StaticSamplers; // Not supported yet due to lack of support in Metal
        RootConstantBindings                 RootConstants;
    };

    class IRootSignature
    {
    public:
        virtual ~IRootSignature( ) = default;
    };

} // namespace DenOfIz
