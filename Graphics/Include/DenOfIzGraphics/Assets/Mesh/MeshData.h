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
    enum class DZ_API VertexAttributeType
    {
        Position,
        Normal,
        Tangent,
        Bitangent,
        Color,
        UV0,
        UV1,
        BoneIndices,
        BoneWeights
    };

    // Vertex Stream represents a group of vertex attributes that are stored together
    struct DZ_API VertexStreamDesc
    {
        BitSet<VertexAttributeType> Attributes;       // Which attributes are in this stream
        InteropArray<uint32_t>      AttributeOffsets; // Offset of each attribute in the stream
        uint32_t                    Stride;           // Total stride of the stream
    };

    struct DZ_API VertexStream
    {
        VertexStreamDesc   Desc;
        InteropArray<Byte> Data;
        uint32_t           VertexCount;
    };

    struct DZ_API JointPose
    {
        Float4 Position;
        Float4 Rotation;
        Float4 Scale;
    };

    struct DZ_API JointKeyframe
    {
        float     Timestamp;
        JointPose Pose;
    };

    struct DZ_API JointAnimTrack
    {
        InteropString               JointName;
        InteropArray<JointKeyframe> Keyframes;
    };

    struct DZ_API AnimationClip
    {
        InteropString                Name;
        float                        Duration;
        InteropArray<JointAnimTrack> Tracks;
    };

    struct DZ_API Joint
    {
        InteropString          Name;
        Matrix4x4              InverseBindMatrix; // 4x4 matrix
        Matrix4x4              LocalTransform;    // 4x4 matrix
        Matrix4x4              GlobalTransform;
        int32_t                ParentIndex;
        InteropArray<uint32_t> ChildIndices;
    };

    struct DZ_API Skeleton
    {
        InteropArray<Joint> Joints;
        uint32_t            RootJointIndex;
    };

    struct DZ_API MeshSection
    {
        uint32_t StartIndex;
        uint32_t IndexCount;
        uint32_t VertexOffset;
        uint32_t VertexCount;
        uint32_t MaterialIndex;
    };

    struct DZ_API MaterialDesc
    {
        InteropString AlbedoMap;
        InteropString NormalMap;
        InteropString MetallicRoughnessMap;
        InteropString EmissiveMap;

        float BaseColorFactor[ 4 ];
        float MetallicFactor;
        float RoughnessColor;
        float EmissiveFactor[ 3 ];
        bool  AlphaBlend;
    };

    struct DZ_API MeshVertex
    {
        Float4 Position;
        Float4 Normal;
        Float2 TexCoord;
        Float4 Tangent;
        Float4 Bitangent;
        Float4 Color;
        Int4   BoneIndices;
        Float4 BoneWeights;
    };

    struct DZ_API SubMesh
    {
        uint32_t BaseVertex;
        uint32_t BaseIndex;
        uint32_t NumVertices;
        uint32_t NumIndices;
        uint32_t MaterialIndex;
    };

    struct DZ_API MeshBufferSizes
    {
        uint32_t TotalVertices;
        uint32_t TotalIndices;
        uint32_t NumSubMeshes;
        uint32_t NumJoints;
        uint32_t NumAnimations;
    };

    struct DZ_API AnimationKey
    {
        float  Time;
        Float4 Position;
        Float4 Rotation; // Quaternion
        Float4 Scale;
    };

    struct DZ_API AnimationChannel
    {
        InteropString              BoneName;
        InteropArray<AnimationKey> Keys;
    };

    struct DZ_API Animation
    {
        InteropString                  Name;
        float                          Duration;
        float                          TicksPerSecond;
        InteropArray<AnimationChannel> Channels;
    };

    struct DZ_API JointNode
    {
        uint32_t               JointIndex;
        int32_t                ParentIndex; // -1 for root
        InteropArray<uint32_t> ChildIndices;
    };

    struct DZ_API BoneTransform
    {
        Matrix4x4 Transform;
        Matrix4x4 InverseBindMatrix;
        Matrix4x4 FinalTransform;
    };

    // clang-format off
    class DZ_API MeshStreamCallback{
    public:
        virtual ~MeshStreamCallback( ){}
        virtual void OnBegin( const MeshBufferSizes &sizes ) { }
        virtual void OnVertexData( const InteropArray<MeshVertex> &vertices, uint32_t startIndex ) { }
        virtual void OnIndexData( const InteropArray<uint32_t> &indices, uint32_t startIndex ) { }
        virtual void OnJointData(const InteropArray<Joint>& jointData) {}
        virtual void OnJointHierarchy(const InteropArray<JointNode>& hierarchy) {}
        virtual void OnInverseBindMatrices(const InteropArray<float>& matrices) {}
        virtual void OnAnimationData( const InteropArray<Animation> &animations ) { }
        virtual void OnComplete( const InteropArray<SubMesh> &subMeshes ) { }
    };
    // clang-format on
} // namespace DenOfIz
