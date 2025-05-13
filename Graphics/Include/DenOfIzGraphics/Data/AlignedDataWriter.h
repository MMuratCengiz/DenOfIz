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

#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Utilities/Common.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
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
        AlignedDataWriter( );
        ~AlignedDataWriter( ) = default;
        /// Adds numBytes amount of (Byte) 0
        void AddPadding( const uint32_t &numBytes ) const;

        [[nodiscard]] InteropArray<Byte> Data( const uint32_t &totalAlignment = 256 ) const;
        // IBufferResource* must be on CPU_GPU heap
        void WriteToBuffer( IBufferResource *buffer, const uint32_t &bufferOffset = 0 ) const;
    };
} // namespace DenOfIz
