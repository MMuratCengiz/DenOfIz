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

#include "DenOfIzGraphics/Utilities/Common_Macro.h"
#include "IBufferResource.h"
#include "ITextureResource.h"
#include "ReflectionData.h"
#include "ShaderData.h"

#include <ranges>
#include <unordered_set>

namespace DenOfIz
{
    enum class RootSignatureType
    {
        Graphics,
        Compute
    };

    template class DZ_API InteropArray<ShaderStage>;

    /// \brief Not recommended create this structure manually, instead rely on ShaderProgram to provide you an instance after compiling a shader instead.
    /// Due to differences in layouts between DX12, Vulkan and Metal, the Reflection field is very important for the functionality of rest of the API, while you can fill the data
    /// in as well, some of it might feel cryptic to the end user.
    struct DZ_API ResourceBindingDesc
    {
        InteropString             Name;
        ResourceBindingType       BindingType = ResourceBindingType::ConstantBuffer;
        uint32_t                  Binding{ };
        uint32_t                  RegisterSpace = 0;
        uint32_t                  Descriptor;
        InteropArray<ShaderStage> Stages;
        int                       ArraySize = 1; // 1 is both 'Arr[1]'(Size of 1) and Simply 'Var'(Non array variable)
        ReflectionDesc            Reflection{ };
        bool                      IsBindless = false;
    };

    struct DZ_API ResourceBindingDescArray
    {
        ResourceBindingDesc *Elements;
        uint32_t             NumElements;
    };

    struct DZ_API StaticSamplerDesc
    {
        SamplerDesc         Sampler;
        ResourceBindingDesc Binding;
    };

    struct DZ_API StaticSamplerDescArray
    {
        StaticSamplerDesc *Elements;
        uint32_t           NumElements;
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

    struct DZ_API RootConstantResourceBindingDescArray
    {
        RootConstantResourceBindingDesc *Elements;
        uint32_t                         NumElements;
    };

    template class DZ_API InteropArray<ResourceBindingDesc>;
    template class DZ_API InteropArray<StaticSamplerDesc>;
    template class DZ_API InteropArray<RootConstantResourceBindingDesc>;

    /// \brief Describes a bindless resource array that should be pre-allocated in the root signature
    struct DZ_API BindlessResourceDesc
    {
        uint32_t            Binding;
        uint32_t            RegisterSpace = 0;
        ResourceBindingType Type          = ResourceBindingType::ShaderResource;
        uint32_t            MaxArraySize  = 1024;
        bool                IsDynamic     = true; // Can be updated at runtime
        InteropString       Name;
    };
    struct DZ_API BindlessResourceDescArray
    {
        BindlessResourceDesc *Elements;
        uint32_t              NumElements;
    };

    struct DZ_API RootSignatureDesc
    {
        // The order of the bindings must match the order of the shader inputs!!! TODO might need to be fixed but this is normal for DX12
        ResourceBindingDescArray             ResourceBindings;
        StaticSamplerDescArray               StaticSamplers;
        RootConstantResourceBindingDescArray RootConstants;
        BindlessResourceDescArray            BindlessResources;
    };

    class DZ_API IRootSignature
    {
    public:
        virtual ~IRootSignature( ) = default;
    };

} // namespace DenOfIz
