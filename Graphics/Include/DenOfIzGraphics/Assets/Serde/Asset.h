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

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{
    struct DZ_API AssetPath
    {
        InteropString Protocol;
        InteropString Bundle; // Empty for default bundle
        InteropString Path;

        static AssetPath Parse(const InteropString& uri);
        static AssetPath Create(const InteropString& path, const InteropString& bundle = "");
        InteropString ToString() const;
    };

    struct DZ_API AssetHeader
    {
        uint64_t Magic;
        uint32_t Version;
        uint64_t NumBytes;
    };

    struct DZ_API AssetDataStream
    {
        uint64_t NumBytes = 0;
    };

    struct DZ_API UserProperty
    {
        enum class Type
        {
            String,
            Int,
            Float,
            Bool,
            Vector2,
            Vector3,
            Vector4,
            Color,
            Transform
        };

        InteropString Name;
        Type          PropertyType;

        InteropString StringValue;
        int32_t       IntValue{ };
        float         FloatValue{ };
        bool          BoolValue{ };
        Float2        Vector2Value{ };
        Float3        Vector3Value{ };
        Float4        Vector4Value{ };
        Float4        ColorValue{ };
        Float4x4      TransformValue;
    };
} // namespace DenOfIz
