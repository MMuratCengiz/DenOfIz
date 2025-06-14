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
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    struct DZ_API StructuredBufferDesc
    {
        uint64_t Offset;
        uint64_t NumElements;
        uint64_t Stride;
    };

    struct DZ_API BufferDesc
    {
        uint32_t             Alignment = 0; // None or Constants.BufferAlignment(Api Dependant)
        size_t               NumBytes{ };
        StructuredBufferDesc StructureDesc{ }; // For Structured Buffers, set `ResourceDescriptor::StructuredBuffer`
        Format               Format = Format::Undefined;
        uint32_t             Descriptor;
        uint32_t             InitialUsage = ResourceUsage::Common;
        uint32_t             Usages;
        HeapType             HeapType;
        InteropString        DebugName;
    };

    class DZ_API IBufferResource
    {
    public:
        virtual ~IBufferResource( ) = default;

        // Allowed only on CPU visible resources
        virtual void *MapMemory( )   = 0;
        virtual void  UnmapMemory( ) = 0;
        //--
        [[nodiscard]] virtual uint32_t    InitialState( ) const = 0;
        [[nodiscard]] virtual size_t      NumBytes( ) const     = 0;
        [[nodiscard]] virtual const void *Data( ) const         = 0;

        // Interop API
        [[nodiscard]] virtual ByteArray GetData( ) const                                      = 0;
        virtual void                    SetData( const ByteArrayView &data, bool keepMapped ) = 0;
        /// Write data always maps memory, user should properly align bufferOffset if necessary
        virtual void WriteData( const ByteArrayView &data, uint32_t bufferOffset ) = 0;
    };

    struct BufferSlice
    {
        IBufferResource *Buffer;
        uint64_t         Offset;
        uint64_t         NumBytes;
    };
} // namespace DenOfIz
