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

#include "DenOfIzGraphicsInternal/Utilities/StringStore.h"

using namespace DenOfIz;

DenOfIz_StringView StringStore::Store( const std::string &str )
{
    return Store( str.c_str( ), str.size( ) );
}

DenOfIz_StringView StringStore::Store( const char *cString )
{
    if ( !cString )
    {
        return { };
    }
    return Store( cString, strlen( cString ) );
}

DenOfIz_StringView StringStore::Store( const char *str, const size_t len )
{
    if ( !str || len == 0 )
    {
        return { };
    }

    const size_t bytesNeeded = len + 1;

    if ( m_bytesRemaining < bytesNeeded )
    {
        AllocateNewBlock( bytesNeeded );
    }

    char *storage = m_currentPtr;
    memcpy( storage, str, len );
    storage[ len ] = '\0';

    m_currentPtr += bytesNeeded;
    m_bytesRemaining -= bytesNeeded;

    return DenOfIz_StringView( storage, len );
}

void StringStore::AllocateNewBlock( const size_t minSize )
{
    const size_t newBlockSize = std::max( s_defaultBlockSize, minSize );

    auto newBlock = std::make_unique<char[]>( newBlockSize );

    m_currentPtr     = newBlock.get( );
    m_bytesRemaining = newBlockSize;

    m_blocks.push_back( std::move( newBlock ) );
}
