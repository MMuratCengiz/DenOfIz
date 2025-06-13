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

#include "DenOfIzGraphics/Utilities/DZArena.h"

using namespace DenOfIz;

DZArena::DZArena( const size_t initialCapacity ) : Buffer( nullptr ), Capacity( initialCapacity ), Used( 0 ), InitialCapacity( initialCapacity )
{
    Buffer = static_cast<Byte *>( malloc( initialCapacity ) );
}

DZArena::~DZArena( )
{
    if ( Buffer )
    {
        free( Buffer );
    }
}

void DZArena::Reset( )
{
    Used = 0;
}

void DZArena::Clear( )
{
    Reset( );
    if ( Capacity > InitialCapacity * 4 )
    {
        free( Buffer );
        Buffer   = static_cast<Byte *>( malloc( InitialCapacity ) );
        Capacity = InitialCapacity;
    }
}

Byte *DZArena::Allocate( const size_t size, const size_t alignment )
{
    size_t       alignedUsed  = Used + alignment - 1 & ~( alignment - 1 );
    const size_t requiredSize = alignedUsed + size;

    if ( requiredSize > Capacity )
    {
        Grow( requiredSize );
        alignedUsed = Used + alignment - 1 & ~( alignment - 1 );
    }

    Byte *result = Buffer + alignedUsed;
    Used         = alignedUsed + size;
    return result;
}

Byte *DZArena::GetWritePointer( ) const
{
    return Buffer + Used;
}

size_t DZArena::GetRemainingCapacity( ) const
{
    return Capacity - Used;
}

size_t DZArena::GetTotalCapacity( ) const
{
    return Capacity;
}

void DZArena::AdvanceCursor( const size_t bytes )
{
    const size_t newUsed = Used + bytes;
    if ( newUsed > Capacity )
    {
        Grow( newUsed );
    }
    Used = newUsed;
}

void DZArena::EnsureCapacity( const size_t requiredCapacity )
{
    if ( requiredCapacity > Capacity )
    {
        Grow( requiredCapacity );
    }
}

void DZArena::Write( const void *data, const size_t size )
{
    EnsureCapacity( Used + size );
    memcpy( Buffer + Used, data, size );
    Used += size;
}

void DZArena::Grow( const size_t requiredSize )
{
    size_t newCapacity = Capacity;
    while ( newCapacity < requiredSize )
    {
        newCapacity *= 2;
    }

    const auto newBuffer = static_cast<Byte *>( malloc( newCapacity ) );
    memcpy( newBuffer, Buffer, Used );
    free( Buffer );
    Buffer   = newBuffer;
    Capacity = newCapacity;
}

DZArenaCursor DZArenaCursor::Create( DZArena *arena )
{
    DZArenaCursor cursor{ };
    cursor.Arena    = arena;
    cursor.Position = arena->Used;
    return cursor;
}

void *DZArenaCursor::Allocate( const size_t size, const size_t alignment )
{
    const size_t alignedPosition = Position + alignment - 1 & ~( alignment - 1 );
    const size_t requiredSize    = alignedPosition + size;

    Arena->EnsureCapacity( requiredSize );
    void *result = Arena->Buffer + alignedPosition;
    Position     = alignedPosition + size;
    if ( Position > Arena->Used )
    {
        Arena->Used = Position;
    }
    return result;
}

void DZArenaCursor::Write( const void *data, const size_t size )
{
    Arena->EnsureCapacity( Position + size );
    memcpy( Arena->Buffer + Position, data, size );
    Position += size;
    if ( Position > Arena->Used )
    {
        Arena->Used = Position;
    }
}

Byte *DZArenaCursor::GetWritePointer( ) const
{
    return Arena->Buffer + Position;
}

void DZArenaCursor::AdvancePosition( const size_t bytes )
{
    Position += bytes;
    Arena->EnsureCapacity( Position );
    if ( Position > Arena->Used )
    {
        Arena->Used = Position;
    }
}

size_t DZArenaCursor::GetPosition( ) const
{
    return Position;
}

void DZArenaCursor::SetPosition( const size_t position )
{
    Position = position;
    if ( Position > Arena->Used )
    {
        Arena->EnsureCapacity( Position );
        Arena->Used = Position;
    }
}
