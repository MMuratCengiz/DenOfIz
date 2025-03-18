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

#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>
#include <DenOfIzGraphics/Assets/Asset.h>

namespace DenOfIz
{
    struct DZ_API TextureAsset : AssetDataHeader
    {
        static constexpr uint32_t Latest = 1;
        static constexpr uint64_t Magic  = 0x445A544558; // 'DZTEX'

        InteropString       Name;
        // TODO
        TextureAsset( ) : AssetDataHeader( Magic, Latest, 0 )
        {
        }
    };
} // namespace DenOfIz
