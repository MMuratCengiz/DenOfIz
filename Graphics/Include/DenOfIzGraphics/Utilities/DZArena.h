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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Common_Macro.h"
#include "Interop.h"

namespace DenOfIz
{
    class DZ_API DZArena : public NonCopyable
    {
        friend struct DZArenaCursor;

    public:
        DZArena( size_t initialCapacity );
        ~DZArena( );

        void   Reset( );
        void   Clear( );
        Byte  *Allocate( size_t size, size_t alignment = alignof( std::max_align_t ) );
        Byte  *GetWritePointer( );
        size_t GetRemainingCapacity( ) const;
        void   AdvanceCursor( size_t bytes );
        void   EnsureCapacity( size_t requiredCapacity );
        void   Write( const void *data, size_t size );

    private:
        void Grow( size_t requiredSize );

        Byte  *Buffer;
        size_t Capacity;
        size_t Used;
        size_t InitialCapacity;
    };

    struct DZ_API DZArenaCursor
    {
        DZArena *Arena;
        size_t   Position;

        static DZArenaCursor Create( DZArena *arena );

        void  *Allocate( size_t size, size_t alignment = alignof( std::max_align_t ) );
        void   Write( const void *data, size_t size );
        Byte  *GetWritePointer( );
        void   AdvancePosition( size_t bytes );
        size_t GetPosition( ) const;
        void   SetPosition( size_t position );
    };
} // namespace DenOfIz
