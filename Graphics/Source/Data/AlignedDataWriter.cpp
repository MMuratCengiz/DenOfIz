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

#include "DenOfIzGraphics/Data/AlignedDataWriter.h"

using namespace DenOfIz;

AlignedDataWriter::AlignedDataWriter( ) : BinaryWriter( m_container )
{
}


void AlignedDataWriter::AddPadding( const uint32_t &numBytes ) const
{
    for ( int i = 0; i < numBytes; i++ )
    {
        WriteByte( 0 );
    }
}

InteropArray<Byte> AlignedDataWriter::Data( const uint32_t &totalAlignment ) const
{
    InteropArray<Byte> result = m_container.GetData( );
    for ( auto i = result.NumElements( ); i < totalAlignment; i++ )
    {
        result.AddElement( 0 );
    }
    return result;
}

void AlignedDataWriter::WriteToBuffer( IBufferResource *buffer, const uint32_t &bufferOffset ) const
{
    buffer->WriteData( Data(  ), bufferOffset );
}