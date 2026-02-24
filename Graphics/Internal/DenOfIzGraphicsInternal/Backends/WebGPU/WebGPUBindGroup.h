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

#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroup.h"
#include "WebGPUBindGroupLayout.h"
#include "WebGPUBuffer.h"
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUBindGroup final : public IBindGroup
    {
        WebGPUContext                                   *m_context;
        DenOfIz_BindGroupDesc                            m_desc;
        WGPUBindGroup                                    m_bindGroup       = nullptr;
        WebGPUBindGroupLayout                           *m_bindGroupLayout = nullptr;
        std::vector<WGPUBindGroupEntry>                  m_entries;
        std::unordered_map<uint32_t, WGPUBindGroupEntry> m_bindingMap;
        std::vector<WebGPUBuffer *>                      m_boundBuffers;
        bool                                             m_isUpdating = false;

    public:
        WebGPUBindGroup( WebGPUContext *context, const DenOfIz_BindGroupDesc &desc );
        ~WebGPUBindGroup( ) override;

        IBindGroup *BeginUpdate( ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IBindGroup *Srv( const uint32_t binding, ITexture *resource ) override;
        IBindGroup *SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources ) override;
        IBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Uav( const uint32_t binding, ITexture *resource ) override;
        IBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void        EndUpdate( ) override;

        [[nodiscard]] WGPUBindGroup GetBindGroup( ) const;
        [[nodiscard]] uint32_t      GetRegisterSpace( ) const;
        void                        SyncBuffers( ) const;

    private:
        void                             CreateBindGroup( );
        [[nodiscard]] WGPUBindGroupEntry CreateBindingEntry( uint32_t binding, const WGPUBuffer &buffer, uint64_t offset = 0, uint64_t size = 0 ) const;
        [[nodiscard]] WGPUBindGroupEntry CreateBindingEntry( uint32_t binding, const WGPUTextureView &textureView ) const;
        [[nodiscard]] WGPUBindGroupEntry CreateBindingEntry( uint32_t binding, const WGPUSampler &sampler ) const;
    };

} // namespace DenOfIz
