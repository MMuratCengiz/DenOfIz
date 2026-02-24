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

#include <string>
#include <vector>
#include <webgpu/webgpu.h>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUBuffer final : public IBuffer
    {
        WebGPUContext     *m_context;
        DenOfIz_BufferDesc m_desc;
        std::string        m_debugName;
        WGPUBuffer         m_buffer     = nullptr;
        void              *m_mappedData = nullptr;
        bool               m_isMapped   = false;

        std::vector<uint8_t> m_shadowBuffer;
        bool                 m_useShadowBuffer = false;

    public:
        WebGPUBuffer( WebGPUContext *context, const DenOfIz_BufferDesc &desc );
        ~WebGPUBuffer( ) override;

        void       *MapMemory( ) override;
        void        UnmapMemory( ) override;
        size_t      NumBytes( ) const override;
        const void *Data( ) const override;

        DenOfIz_ByteArray GetData( ) const override;
        void              SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) override;
        void              WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) override;

        WGPUBuffer GetBuffer( ) const;
        size_t     GetNumBytes( ) const;
        void       SyncShadowBuffer( ) const;
    };

} // namespace DenOfIz
