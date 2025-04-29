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

#include <DenOfIzGraphics/Assets/Serde/Asset.h>
#include <DenOfIzGraphics/Assets/Shaders/ShaderReflectionHelper.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    struct DZ_API ShaderStageAsset
    {
        ShaderStage          Stage;
        InteropString        EntryPoint;
        InteropArray<Byte>   DXIL;
        InteropArray<Byte>   MSL;
        InteropArray<Byte>   SPIRV;
        InteropArray<Byte>   Reflection; // Generated Dxc reflection
        RayTracingShaderDesc RayTracing;
    };
    template class DZ_API InteropArray<ShaderStageAsset>;

    struct DZ_API ShaderAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropArray<ShaderStageAsset> Stages;
        ShaderReflectDesc              ReflectDesc;
        ShaderRayTracingDesc           RayTracing;
        InteropArray<UserProperty>     UserProperties;

        ShaderAsset( ) : AssetHeader( 0x44414853445A /*DZSHAD*/, Latest, 0 )
        {
        }
    };
} // namespace DenOfIz
