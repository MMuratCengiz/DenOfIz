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

    struct DZ_API ResourceBindingSlot
    {
        uint32_t                    Binding       = 0;
        uint32_t                    RegisterSpace = 0;
        DescriptorBufferBindingType Type          = DescriptorBufferBindingType::ConstantBuffer;

        // To simplify having a really odd looking vector of ResourceBindingSlots
        [[nodiscard]] uint32_t      Key( ) const;
        [[nodiscard]] InteropString ToString( ) const;
    };

    template class DZ_API InteropArray<ResourceBindingSlot>;
    template class DZ_API InteropArray<ShaderStage>;

    struct DZ_API ResourceBindingDesc
    {
        InteropString               Name;
        DescriptorBufferBindingType BindingType = DescriptorBufferBindingType::ConstantBuffer;
        uint32_t                    Binding{ };
        uint32_t                    RegisterSpace = 0;
        BitSet<ResourceDescriptor>  Descriptor;
        InteropArray<ShaderStage>   Stages;
        int                         ArraySize = 1; // 1 is both 'Arr[1]'(Size of 1) and Simply 'Var'(Non array variable)
        ReflectionDesc              Reflection{ };
    };

    struct DZ_API StaticSamplerDesc
    {
        SamplerDesc         Sampler;
        ResourceBindingDesc Binding;
    };

    // For cross api compatibility the RegisterSpace is hardcoded to 99, make sure to use the same value in the HLSL Shader
    struct DZ_API RootConstantResourceBindingDesc
    {
        InteropString             Name;
        uint32_t                  Binding{ };
        int                       NumBytes{ };
        InteropArray<ShaderStage> Stages;
        ReflectionDesc            Reflection{ };
    };

    template class DZ_API InteropArray<ResourceBindingDesc>;
    template class DZ_API InteropArray<StaticSamplerDesc>;
    template class DZ_API InteropArray<RootConstantResourceBindingDesc>;

    struct DZ_API RootSignatureDesc
    {
        // The order of the bindings must match the order of the shader inputs!!! TODO might need to be fixed but this is normal for DX12
        InteropArray<ResourceBindingDesc>             ResourceBindings;
        InteropArray<StaticSamplerDesc>               StaticSamplers; // Not supported yet due to lack of support in Metal
        InteropArray<RootConstantResourceBindingDesc> RootConstants;
    };

    class DZ_API IRootSignature
    {
    public:
        virtual ~IRootSignature( ) = default;
    };

} // namespace DenOfIz
