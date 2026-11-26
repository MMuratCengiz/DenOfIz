/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
