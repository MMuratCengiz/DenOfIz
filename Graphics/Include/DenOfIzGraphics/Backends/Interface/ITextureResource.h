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

#include "CommonData.h"

namespace DenOfIz
{

    struct DZ_API SamplerDesc
    {
        Filter             MagFilter     = Filter::Linear;
        Filter             MinFilter     = Filter::Linear;
        SamplerAddressMode AddressModeU  = SamplerAddressMode::Repeat;
        SamplerAddressMode AddressModeV  = SamplerAddressMode::Repeat;
        SamplerAddressMode AddressModeW  = SamplerAddressMode::Repeat;
        float              MaxAnisotropy = 0.0f;
        CompareOp          CompareOp     = CompareOp::Always;
        MipmapMode         MipmapMode    = MipmapMode::Linear;
        float              MipLodBias    = 0.0f;
        float              MinLod        = 0.0f;
        float              MaxLod        = 0.0f;
        InteropString      DebugName;
    };

    struct DZ_API TextureDesc
    {
        TextureAspect              Aspect = TextureAspect::Color;
        Format                     Format = Format::Undefined;
        uint32_t Descriptor;

        HeapType              HeapType        = HeapType::GPU;
        MSAASampleCount       MSAASampleCount = MSAASampleCount::_0;
        uint32_t              InitialUsage;
        uint32_t              Usages;
        SamplerDesc           Sampler; // Requires `| Descriptor::Sampler`

        uint32_t Width = 1;
        // if Height is > 1, it is a 2D texture
        uint32_t Height = 1;
        // if Depth is > 1, it is a 3D texture
        uint32_t      Depth     = 1;
        uint32_t      ArraySize = 1;
        uint32_t      MipLevels = 1;
        InteropString DebugName;
    };

    class DZ_API ITextureResource
    {
    public:
        virtual ~ITextureResource( )                        = default;
        virtual uint32_t InitialState( ) const = 0;
        virtual Format                GetFormat( ) const    = 0;
    };
    template class DZ_API InteropArray<ITextureResource *>;

    DZ_API void ValidateTextureDesc( TextureDesc &desc );

    class DZ_API ISampler
    {
    public:
        virtual ~ISampler( ) = default;
    };
} // namespace DenOfIz
