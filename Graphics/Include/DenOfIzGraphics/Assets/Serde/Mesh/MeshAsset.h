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
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{

    enum class ColorFormat
    {
        RGBA,
        RGB,
        RG,
        R,
    };

    struct DZ_API ColorFormatArray
    {
        ColorFormat *Elements;
        uint32_t     NumElements;

        DZ_ARRAY_METHODS( ColorFormatArray, ColorFormat )
    };
    /// The data will be structured in the following way:
    /// if Position is true => VertexAttributeConfig.NumPositionComponents x float
    /// if Normal is true => 3 x floats
    /// if UV is true => VertexAttributeConfig.UVChannels.NumElements( ) * ( 2 x float )
    /// if Color is true => for each VertexAttributeConfig.ColorFormat, number of color channels * float
    /// if Tangent is true => 4 x float
    /// if Bitangent is true => 4 x float
    /// if BlendIndices is true => 4 x uint32_t
    /// if BlendWeights is true => 4 x float
    struct DZ_API VertexEnabledAttributes
    {
        bool Position     = true;
        bool Normal       = true;
        bool UV           = true;
        bool Color        = false;
        bool Tangent      = true;
        bool Bitangent    = true;
        bool BlendIndices = true;
        bool BlendWeights = true;
    };

    // Not all the fields here have values, it is configured VertexEnabledAttributes
    struct DZ_API MeshVertex
    {
        Float_4      Position{ };
        Float_4      Normal{ };
        Float_2Array UVs;
        Float_4Array Colors;
        Float_4      Tangent{ };
        Float_4      Bitangent{ };
        UInt32_4     BlendIndices{ };
        Float_4      BoneWeights{ };

        void Dispose( ) const
        {
            UVs.Dispose( );
            Colors.Dispose( );
        }
    };

    struct DZ_API MeshVertexArray
    {
        MeshVertex *Elements;
        uint32_t    NumElements;

        void Dispose( ) const
        {
            for ( uint32_t i = 0; i < NumElements; ++i )
            {
                Elements[ i ].Dispose( );
            }
            delete[] Elements;
        }
    };

    struct DZ_API MorphTargetDeltaAttributes
    {
        bool Position = true;
        bool Normal   = true;
        bool Tangent  = true;
    };

    struct DZ_API MorphTargetDelta
    {
        Float_4 Position;
        Float_4 Normal;
        Float_4 Tangent;
    };

    struct DZ_API MorphTargetDeltaArray
    {
        MorphTargetDelta *Elements;
        uint32_t          NumElements;
    };

    struct DZ_API UVChannel
    {
        InteropString SemanticName; // e.g. "DIFFUSE", "LIGHTMAP", "DETAIL"
        uint32_t      Index{ };
    };

    struct DZ_API UVChannelArray
    {
        UVChannel *Elements;
        uint32_t   NumElements;

        DZ_ARRAY_METHODS( UVChannelArray, UVChannel )
    };

    struct DZ_API VertexAttributeConfig
    {
        uint32_t         NumPositionComponents = 4; // TODO not yet implemented
        uint32_t         NumUVAttributes       = 2;
        UVChannelArray   UVChannels;
        ColorFormatArray ColorFormats;
        uint32_t         MaxBoneInfluences = 4;
    };

    struct DZ_API BoxBoundingVolume
    {
        Float_3 Min;
        Float_3 Max;
    };

    struct DZ_API SphereBoundingVolume
    {
        Float_3 Center;
        float   Radius;
    };

    struct DZ_API CapsuleBoundingVolume
    {
        Float_3 Start;
        Float_3 End;
        float   Radius;
    };

    struct DZ_API ConvexHullBoundingVolume
    {
        AssetDataStream VertexStream;
    };

    enum class BoundingVolumeType
    {
        Box,
        Sphere,
        Capsule,
        ConvexHull
    };

    struct DZ_API BoundingVolume
    {
        BoundingVolumeType Type = BoundingVolumeType::Box;
        InteropString      Name;

        BoxBoundingVolume        Box{ };
        SphereBoundingVolume     Sphere{ };
        CapsuleBoundingVolume    Capsule{ };
        ConvexHullBoundingVolume ConvexHull;
    };

    struct DZ_API BoundingVolumeArray
    {
        BoundingVolume *Elements;
        uint32_t        NumElements;

        DZ_ARRAY_METHODS( BoundingVolumeArray, BoundingVolume )
    };

    struct DZ_API MorphTarget
    {
        InteropString   Name;
        AssetDataStream VertexDeltaStream;
        float           DefaultWeight = 0.0f;
    };

    struct DZ_API MorphTargetArray
    {
        MorphTarget *Elements;
        uint32_t     NumElements;

        DZ_ARRAY_METHODS( MorphTargetArray, MorphTarget )
    };

    struct DZ_API SubMeshData
    {
        InteropString       Name;
        PrimitiveTopology   Topology    = PrimitiveTopology::Triangle;
        uint64_t            NumVertices = 0;
        AssetDataStream     VertexStream{ };
        uint64_t            NumIndices = 0;
        IndexType           IndexType  = IndexType::Uint32;
        AssetDataStream     IndexStream{ };
        Float_3             MinBounds{ 0.0f, 0.0f, 0.0f };
        Float_3             MaxBounds{ 0.0f, 0.0f, 0.0f };
        AssetUri            MaterialRef{ };
        uint32_t            LODLevel = 0;
        BoundingVolumeArray BoundingVolumes;

        void Dispose( ) const
        {
            BoundingVolumes.Dispose( );
        }
    };

    struct DZ_API SubMeshDataArray
    {
        SubMeshData *Elements;
        uint32_t     NumElements;

        static SubMeshDataArray Create( uint32_t NumElements )
        {
            SubMeshDataArray Result;
            Result.Elements    = new SubMeshData[ NumElements ];
            Result.NumElements = NumElements;
            return Result;
        }

        void Dispose( ) const
        {
            for ( uint32_t i = 0; i < NumElements; ++i )
            {
                Elements[ i ].Dispose( );
            }
            delete[] Elements;
        }
    };

    struct DZ_API JointData
    {
        InteropString Name;
        Float_4x4     InverseBindMatrix;
        Float_4x4     LocalTransform;
        Float_4x4     GlobalTransform;
        int32_t       ParentIndex = -1;
        UInt32Array   ChildIndices;

        void Dispose( ) const
        {
            ChildIndices.Dispose( );
        }
    };

    struct DZ_API MeshAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString              Name;
        uint32_t                   NumLODs = 1;
        VertexEnabledAttributes    EnabledAttributes{ };
        VertexAttributeConfig      AttributeConfig{ };
        SubMeshDataArray           SubMeshes;
        MorphTargetDeltaAttributes MorphTargetDeltaAttributes{ };
        MorphTargetArray           MorphTargets;
        AssetUriArray              AnimationRefs; // Array of all available animations for this mesh
        AssetUri                   SkeletonRef{ };
        UserPropertyArray          UserProperties;

        MeshAsset( ) : AssetHeader( 0x445A4D455348 /*DZMESH*/, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzmesh";
        }

        void Dispose( ) const
        {
            SubMeshes.Dispose( );
            MorphTargets.Dispose( );
            AnimationRefs.Dispose( );
            UserProperties.Dispose( );
        }
    };
} // namespace DenOfIz
