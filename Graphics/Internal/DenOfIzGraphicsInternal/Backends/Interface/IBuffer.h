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

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"

namespace DenOfIz
{

    class IBuffer
    {
    public:
        virtual ~IBuffer( ) = default;

        // Allowed only on CPU visible resources
        virtual void *MapMemory( )   = 0;
        virtual void  UnmapMemory( ) = 0;
        //--
        [[nodiscard]] virtual size_t      NumBytes( ) const = 0;
        [[nodiscard]] virtual const void *Data( ) const     = 0;

        // Interop API
        [[nodiscard]] virtual DenOfIz_ByteArray GetData( ) const                                              = 0;
        virtual void                            SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) = 0;
        /// Write data always maps memory, user should properly align bufferOffset if necessary
        virtual void WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) = 0;
    };
} // namespace DenOfIz
