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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalBufferResource final : public IBufferResource
    {
    private:
        MetalContext    *m_context{ };
        BufferDesc       m_desc;
        id<MTLBuffer>    m_buffer{ };
        MTLResourceUsage m_usage{ };
        MTLDataType      m_dataType{ };
        uint32_t         m_numBytes     = 0;
        void            *m_mappedMemory = nullptr;

    public:
        MetalBufferResource( MetalContext *context, const BufferDesc &desc );
        ~MetalBufferResource( ) override;
        const id<MTLBuffer>    &Instance( ) const;
        const MTLResourceUsage &Usage( ) const;
        const MTLDataType      &Type( ) const;
        void                   *MapMemory( ) override;
        void                    UnmapMemory( ) override;

        [[nodiscard]] size_t                NumBytes( ) const override;
        [[nodiscard]] const void           *Data( ) const override;
        [[nodiscard]] BitSet<ResourceUsage> InitialState( ) const override;

        // Interop API
        [[nodiscard]] InteropArray<Byte> GetData( ) const override;
        void                             SetData( const InteropArray<Byte> &data, bool keepMapped ) override;
        void                             WriteData( const InteropArray<Byte> &data, uint32_t bufferOffset ) override;
    };

} // namespace DenOfIz
