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

namespace DenOfIz
{

    struct DZ_API BoxCollider
    {
        Float_3 HalfExtents;
    };

    struct DZ_API SphereCollider
    {
        float Radius;
    };

    struct DZ_API CapsuleCollider
    {
        float Radius;
        float Height;
    };

    struct DZ_API MeshCollider
    {
        AssetDataStream VertexStream;
        AssetDataStream IndexStream;
    };

    enum class PhysicsColliderType
    {
        Box,
        Sphere,
        Capsule,
        ConvexHull,
        TriangleMesh
    };

    struct DZ_API PhysicsCollider
    {
        PhysicsColliderType Type = PhysicsColliderType::Box;
        InteropString       Name;
        Float_4x4           Transform;
        float               Friction    = 0.5f;
        float               Restitution = 0.0f;
        bool                IsTrigger   = false; // Is it just a trigger volume?

        // Specific collider data (no unions for SWIG compatibility), use one matching with type
        BoxCollider     Box{ };
        SphereCollider  Sphere{ };
        CapsuleCollider Capsule{ };
        MeshCollider    Mesh; // Used for both ConvexHull and TriangleMesh
        // --
    };

    struct DZ_API PhysicsColliderArray
    {
        PhysicsCollider *Elements;
        uint32_t         NumElements;
    };

    struct DZ_API PhysicsAsset : AssetHeader, NonCopyable
    {
        DZArena _Arena{ sizeof( PhysicsAsset ) };

        static constexpr uint32_t Latest = 1;

        InteropString        Name;
        PhysicsColliderArray Colliders;
        UserPropertyArray    UserProperties;

        PhysicsAsset( ) : AssetHeader( 0x445A50585953 /*DZPHYS*/, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzphys";
        }
    };
} // namespace DenOfIz
