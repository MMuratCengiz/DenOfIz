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
        std::string                 Name;
        DescriptorBufferBindingType BindingType;
        uint32_t                    Binding;
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
        uint32_t                 Binding{ };
        uint32_t                 RegisterSpace = 0;
        int                      Size{ };
        std::vector<ShaderStage> Stages;
    };

    class IRootSignature : public NonCopyable
    {
    public:
        virtual ~IRootSignature( ) = default;
    };

} // namespace DenOfIz
