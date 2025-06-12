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

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct DZ_API AssetUri
    {
        InteropString Scheme{ "asset" };
        InteropString Path{ };

        AssetUri( )                                                    = default;
        AssetUri( const AssetUri &other )                              = default;
        AssetUri                   &operator=( const AssetUri &other ) = default;
        static AssetUri             Parse( const InteropString &uri );
        static AssetUri             Create( const InteropString &path );
        [[nodiscard]] InteropString ToInteropString( ) const; // Not using ToString() intentionally to avoid conflict with bindings
        [[nodiscard]] bool          Equals( const AssetUri &other ) const;
    };

    struct DZ_API AssetUriArray
    {
        AssetUri *Elements;
        uint32_t  NumElements;
    };

    struct DZ_API AssetHeader
    {
        uint64_t Magic    = 0;
        uint32_t Version  = 0;
        uint64_t NumBytes = 0;
        AssetUri Uri{ };

        AssetHeader( uint64_t magic, uint32_t version, uint64_t numBytes ) : Magic( magic ), Version( version ), NumBytes( numBytes )
        {
        }
        AssetHeader( ) = default;
    };

    struct DZ_API AssetDataStream
    {
        uint64_t Offset   = 0;
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
            Float2,
            Float3,
            Float4,
            Color,
            Float4x4
        };

        InteropString Name;
        Type          PropertyType;

        InteropString StringValue;
        int32_t       IntValue{ };
        float         FloatValue{ };
        bool          BoolValue{ };
        Float_2       Vector2Value{ };
        Float_3       Vector3Value{ };
        Float_4       Vector4Value{ };
        Float_4       ColorValue{ };
        Float_4x4     TransformValue;
    };

    struct DZ_API UserPropertyArray
    {
        UserProperty *Elements    = nullptr;
        uint32_t      NumElements = 0;
    };
} // namespace DenOfIz
