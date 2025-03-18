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
#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{

    enum class ColorFormat
    {
        RGBA,
        RGB,
        RG,
        R,
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
        bool Position : 1     = true;
        bool Normal : 1       = true;
        bool UV : 1           = true;
        bool Color : 1        = false;
        bool Tangent : 1      = true;
        bool Bitangent : 1    = true;
        bool BlendIndices : 1 = true;
        bool BlendWeights : 1 = true;
    };

    // Not all the fields here have values, it is configured VertexEnabledAttributes
    struct DZ_API MeshVertex
    {
        Float4               Position;
        Float4               Normal;
        InteropArray<Float2> UVs;
        Float4               Tangent;
        Float4               Bitangent;
        InteropArray<Float4> Colors;
        UInt4                BoneIndices;
        Float4               BoneWeights;
    };

    struct DZ_API UVChannel
    {
        InteropString SemanticName; // e.g. "DIFFUSE", "LIGHTMAP", "DETAIL"
        uint32_t      Index{ };     // UV index for this channel
    };

    struct DZ_API VertexAttributeConfig
    {
        uint32_t                  NumPositionComponents = 3;
        uint32_t                  NumUVAttributes       = 2;
        InteropArray<UVChannel>   UVChannels;
        InteropArray<ColorFormat> ColorFormats;
        uint32_t                  MaxBoneInfluences = 4;
    };

    struct DZ_API MorphTarget
    {
        InteropString   Name;
        AssetDataStream PositionDeltas;
        AssetDataStream NormalDeltas;
        AssetDataStream TangentDeltas;
        float           Weight = 0.0f;
    };

    // Organized morph target collection
    struct DZ_API MorphTargetSet
    {
        InteropString             Name;
        InteropArray<MorphTarget> Targets;
        InteropArray<float>       DefaultWeights;
    };

    struct DZ_API SubMeshInstance
    {
        Float4x4 Transform;
        uint32_t CustomDataOffset{ };
        uint32_t CustomDataSize{ };
    };

    struct DZ_API BoxBoundingVolume
    {
        Float3 Min;
        Float3 Max;
    };

    struct DZ_API SphereBoundingVolume
    {
        Float3 Center;
        float  Radius;
    };

    struct DZ_API CapsuleBoundingVolume
    {
        Float3 Start;
        Float3 End;
        float  Radius;
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

    struct DZ_API SubMeshData
    {
        InteropString           Name;
        PrimitiveTopology       Topology = PrimitiveTopology::Triangle;
        VertexEnabledAttributes EnabledAttributes{ };
        VertexAttributeConfig   AttributeConfig{ };
        AssetDataStream         VertexStream{ };
        IndexType               IndexType = IndexType::Uint32;
        AssetDataStream         IndexStream{ };
        Float3                  MinBounds{ 0.0f, 0.0f, 0.0f };
        Float3                  MaxBounds{ 0.0f, 0.0f, 0.0f };
        InteropString           MaterialRef{ };

        // Enhanced features
        uint32_t                      LODLevel = 0;    // Level of detail (0 = highest)
        InteropArray<SubMeshInstance> Instances;       // Instancing data
        InteropArray<MorphTarget>     MorphTargets;    // Backward compatibility
        InteropArray<MorphTargetSet>  MorphTargetSets; // Organized morph targets
        InteropArray<BoundingVolume>  BoundingVolumes; // Additional bounding volumes
    };
    template class DZ_API InteropArray<SubMeshData>;

    struct DZ_API JointData
    {
        InteropString          Name;
        Float4x4               InverseBindMatrix;
        Float4x4               LocalTransform;
        Float4x4               GlobalTransform;
        int32_t                ParentIndex = -1;
        InteropArray<uint32_t> ChildIndices;
    };

    struct DZ_API MeshData : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString             Name;
        uint32_t                  NumLODs = 1;
        InteropArray<SubMeshData> SubMeshes;
        InteropArray<JointData>   Joints;
        uint64_t                  StreamDataNumBytes = 0; // Aggregation of various data that is too large and should instead be streamed

        InteropArray<BoundingVolume> BoundingVolumes;
        InteropArray<UserProperty>   UserProperties;

        MeshData( ) : AssetHeader( 0x445A4D455348 /*DZMESH*/, Latest, 0 )
        {
        }
    };
} // namespace DenOfIz
