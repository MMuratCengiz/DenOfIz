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

    struct SamplerDesc
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
        std::string        DebugName;
    };

    struct TextureDesc
    {
        TextureAspect              Aspect = TextureAspect::Color;
        Format                     Format = Format::Undefined;
        BitSet<ResourceDescriptor> Descriptor;

        HeapType        HeapType        = HeapType::GPU;
        MSAASampleCount MSAASampleCount = MSAASampleCount::_0;
        ResourceState   InitialState;
        SamplerDesc     Sampler; // Requires `| Descriptor::Sampler`

        uint32_t Width = 1;
        // if Height is > 1, it is a 2D texture
        uint32_t Height = 1;
        // if Depth is > 1, it is a 3D texture
        uint32_t    Depth     = 1;
        uint32_t    ArraySize = 1;
        uint32_t    MipLevels = 1;
        std::string DebugName;
    };

    class ITextureResource
    {
    public:
        virtual ~ITextureResource( )                        = default;
        virtual BitSet<ResourceState> InitialState( ) const = 0;
        virtual Format                GetFormat( ) const    = 0;
    };

    static void ValidateTextureDesc( TextureDesc &desc )
    {
        if ( desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) && desc.MSAASampleCount != MSAASampleCount::_0 )
        {
            LOG( WARNING ) << "MSAA textures cannot be used as UAVs. Resetting MSAASampleCount to 0.";
            desc.MSAASampleCount = MSAASampleCount::_0;
        }

        if ( desc.MSAASampleCount != MSAASampleCount::_0 && desc.MipLevels > 1 )
        {
            LOG( WARNING ) << "Mip mapped textures cannot be sampled. Resetting MSAASampleCount to 0.";
            desc.MSAASampleCount = MSAASampleCount::_0;
        }

        if ( desc.ArraySize > 1 && desc.Depth > 1 )
        {
            LOG( WARNING ) << "Array textures cannot have depth. Resetting depth to 1.";
            desc.Depth = 1;
        }

        if ( !desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) && !desc.Descriptor.IsSet( ResourceDescriptor::Texture ) &&
             !desc.Descriptor.IsSet( ResourceDescriptor::TextureCube ) )
        {
            LOG( WARNING ) << "Descriptor does not specify a texture: [ResourceDescriptor::(RWTexture/Texture/TextureCube)].";
        }

        if ( desc.Descriptor.IsSet( ResourceDescriptor::TextureCube ) && desc.ArraySize != 6 )
        {
            LOG( WARNING ) << "TextureCube does not have an array size of 6. ";
        }

        if ( desc.Descriptor.IsSet( ResourceDescriptor::TextureCube ) && desc.Height != desc.Width )
        {
            LOG( WARNING ) << "TextureCube does not have equal width and height.";
        }
    }

    class ISampler
    {
    public:
        virtual ~ISampler( ) = default;
    };
} // namespace DenOfIz
