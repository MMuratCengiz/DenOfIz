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

#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalTextureResource final : public ITextureResource
    {
        TextureDesc                 m_desc;
        MetalContext                *m_context{};
        bool                        isExternalResource = false; // Used for swap chain render targets, might need a better way

    public:
        MetalTextureResource(MetalContext *context, const TextureDesc &desc);
        ~MetalTextureResource() override = default;
    private:
        void Validate();
        void CreateTextureSrv( ) const;
        void CreateTextureUav( ) const;
    };

    class MetalSampler final : public ISampler
    {
    private:
        MetalContext                *m_context;
        SamplerDesc                 m_desc;

    public:
        MetalSampler(MetalContext *context, const SamplerDesc &desc);
        ~MetalSampler() override = default;
    };
} // namespace DenOfIz
