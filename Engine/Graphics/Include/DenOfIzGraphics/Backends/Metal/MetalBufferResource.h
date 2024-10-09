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

#include <DenOfIzCore/Utilities.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalBufferResource : public IBufferResource
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
        const id<MTLBuffer> &Instance( ) const
        {
            return m_buffer;
        }
        const MTLResourceUsage &Usage( ) const
        {
            return m_usage;
        }
        const MTLDataType &Type( ) const
        {
            return m_dataType;
        }
        void *MapMemory( ) override;
        void  UnmapMemory( ) override;
    };

} // namespace DenOfIz
