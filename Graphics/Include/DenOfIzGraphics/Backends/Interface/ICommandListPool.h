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

#include "ICommandList.h"

namespace DenOfIz
{
#define DZ_MAX_COMMAND_LISTS 32
    struct DZ_API CommandLists
    {
        size_t        NumElements = 0;
        ICommandList *Array[ DZ_MAX_COMMAND_LISTS ];

        void SetElement( size_t index, ICommandList *value )
        {
            Array[ index ] = value;
        }
        const ICommandList *GetElement( size_t index )
        {
            return Array[ index ];
        }
    };

    struct DZ_API CommandListPoolDesc
    {
        QueueType QueueType;
        // Rename to NumCommandLists
        uint32_t NumCommandLists = 1;
    };

    class DZ_API ICommandListPool
    {
    public:
        virtual CommandLists GetCommandLists( ) = 0;
        virtual ~ICommandListPool( )            = default;
    };
} // namespace DenOfIz
