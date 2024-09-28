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
    struct BufferView
    {
        uint64_t Offset;
        uint64_t Stride;
    };

    struct BufferDesc
    {
        uint32_t                   Alignment = 0; // None or Constants.BufferAlignment(Api Dependant)
        uint32_t                   NumBytes{ };
        BufferView                 BufferView{ }; // For Structured Buffers
        Format                     Format = Format::Undefined;
        BitSet<ResourceDescriptor> Descriptor;
        BitSet<ResourceState>      InitialState;
        HeapType                   HeapType;
        std::string                DebugName;
    };

    class IBufferResource
    {
    protected:
        uint32_t              m_numBytes     = 0;
        const void           *m_data         = nullptr;
        void                 *m_mappedMemory = nullptr;
        BitSet<ResourceState> m_state;

    public:
        IBufferResource( const BufferDesc &desc ) : m_numBytes( desc.NumBytes ), m_state( desc.InitialState )
        {
        }
        virtual ~IBufferResource( ) = default;

        // Allowed only on CPU visible resources
        virtual void *MapMemory( )   = 0;
        virtual void  UnmapMemory( ) = 0;
        //--
        [[nodiscard]] virtual BitSet<ResourceState> InitialState( ) const
        {
            return m_state;
        }

        [[nodiscard]] uint32_t GetSize( ) const
        {
            return m_numBytes;
        }

        [[nodiscard]] const void *GetData( ) const
        {
            return m_data;
        }
    };
} // namespace DenOfIz
