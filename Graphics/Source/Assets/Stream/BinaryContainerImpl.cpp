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

#include "DenOfIzGraphicsInternal/Assets/Stream/BinaryContainerImpl.h"

using namespace DenOfIz;

BinaryContainerImpl::BinaryContainerImpl( )  = default;
BinaryContainerImpl::~BinaryContainerImpl( ) = default;

std::iostream *BinaryContainerImpl::Stream( )
{
    return &m_stream;
}

DenOfIz_ByteArrayView BinaryContainerImpl::GetData( ) const
{
    m_cachedData = m_stream.str( );
    return DenOfIz_ByteArrayView( reinterpret_cast<const Byte *>( m_cachedData.c_str( ) ), m_cachedData.size( ) );
}
