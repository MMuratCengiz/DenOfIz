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

#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <algorithm>
#include "MetalBufferResource.h"
#include "MetalContext.h"
#include "MetalRootSignature.h"
#include "MetalTextureResource.h"

namespace DenOfIz
{
    // For DirectX12 this is kind of a dummy class as resources are bound to heaps. At a given point we only use 2 heaps one for CBV/SRV/UAV and one for Sampler.
    class MetalResourceBindGroup : public IResourceBindGroup
    {
    private:
        MetalContext       *m_context;
        MetalRootSignature *m_rootSignature;
        UpdateDesc          m_updateDesc;

    public:
        MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc );
        void Update( const UpdateDesc &desc ) override;

        const std::vector<UpdateDescItem<IBufferResource>>  &Buffers( ) const;
        const std::vector<UpdateDescItem<ITextureResource>> &Textures( ) const;
        const std::vector<UpdateDescItem<ISampler>>         &Samplers( ) const;

    protected:
        void BindTexture( const std::string &name, ITextureResource *resource ) override;
        void BindBuffer( const std::string &name, IBufferResource *resource ) override;
        void BindSampler( const std::string &name, ISampler *sampler ) override;
    };

} // namespace DenOfIz
