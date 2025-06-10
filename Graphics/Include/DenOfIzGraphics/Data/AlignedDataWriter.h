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

#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"

namespace DenOfIz
{
    struct DZ_API AlignedDataWriterDesc
    {
        uint32_t Alignment;
    };

    /// Very simple class to help write data for non Cpp languages
    class AlignedDataWriter : public BinaryWriter
    {
        BinaryContainer m_container;

    public:
        DZ_API AlignedDataWriter( );
        DZ_API ~AlignedDataWriter( ) = default;
        /// Adds numBytes amount of (Byte) 0
        DZ_API void                        AddPadding( const uint32_t &numBytes ) const;
        DZ_API [[nodiscard]] ByteArrayView Data( ) const;
    };
} // namespace DenOfIz
