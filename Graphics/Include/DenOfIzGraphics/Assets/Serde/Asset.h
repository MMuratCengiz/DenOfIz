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
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_AssetHeader
    {
        uint64_t           Magic;
        uint32_t           Version;
        uint64_t           NumBytes;
        DenOfIz_StringView Path;
    } DenOfIz_AssetHeader;

    typedef struct DenOfIz_AssetDataStream
    {
        uint64_t Offset;
        uint64_t NumBytes;
    } DenOfIz_AssetDataStream;

#ifdef __cplusplus
}
#endif
