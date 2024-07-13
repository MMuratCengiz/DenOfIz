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

#include "IBufferResource.h"
#include "IRootSignature.h"
#include "ITextureResource.h"

namespace DenOfIz
{
    struct ResourceBindGroupDesc
    {
        IRootSignature *RootSignature;
    };

    struct UpdateDesc
    {
        std::vector<IBufferResource *>  Buffers;
        std::vector<ITextureResource *> Textures;
        std::vector<ISampler *>         Samplers;
    };

    class IResourceBindGroup
    {
    public:
        virtual ~IResourceBindGroup() = default;
        virtual void Update(UpdateDesc desc)
        {
            for (auto buffer : desc.Buffers)
            {
                BindBuffer(buffer);
            }
            for (auto texture : desc.Textures)
            {
                BindTexture(texture);
            }
            for (auto sampler : desc.Samplers)
            {
                BindSampler(sampler);
            }
        }

    protected:
        virtual void BindTexture(ITextureResource *resource) = 0;
        virtual void BindBuffer(IBufferResource *resource)   = 0;
        virtual void BindSampler(ISampler *sampler)          = 0;
    };

} // namespace DenOfIz
