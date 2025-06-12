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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct DZ_API MaterialAsset : AssetHeader, NonCopyable
    {
        DZArena _Arena{ sizeof( MaterialAsset ) };

        static constexpr uint32_t Latest = 1;

        InteropString Name;
        InteropString ShaderRef;

        AssetUri AlbedoMapRef;
        AssetUri NormalMapRef;
        AssetUri MetallicRoughnessMapRef;
        AssetUri EmissiveMapRef;
        AssetUri OcclusionMapRef;

        Float_4 BaseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        float   MetallicFactor  = 0.0f;
        float   RoughnessFactor = 0.0f;
        Float_3 EmissiveFactor  = { 0.0f, 0.0f, 0.0f };

        bool AlphaBlend  = false;
        bool DoubleSided = false;

        UserPropertyArray Properties{ };
        MaterialAsset( ) : AssetHeader( 0x445A4D4154 /*DZMAT*/, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzmat";
        }
    };
} // namespace DenOfIz
