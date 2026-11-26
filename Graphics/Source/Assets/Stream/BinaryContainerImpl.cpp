/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
