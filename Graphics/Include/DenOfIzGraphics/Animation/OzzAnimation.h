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

#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct DZ_API OzzContext{ };

    struct DZ_API SamplingJobDesc
    {
        OzzContext     *Context = nullptr;
        float           Ratio   = 0.0f;
        Float_4x4Array *OutTransforms{ };
    };

    struct DZ_API BlendingJobLayerDesc
    {
        Float_4x4Array Transforms{ }; // Todo this could potentially be expensive copying
        float          Weight = 0.0f;
    };

    struct DZ_API BlendingJobLayerDescArray
    {
        BlendingJobLayerDesc *Elements;
        uint32_t              NumElements;
    };

    struct DZ_API BlendingJobDesc
    {
        OzzContext               *Context = nullptr;
        BlendingJobLayerDescArray Layers{ };
        float                     Threshold = 0.1f;
        Float_4x4Array           *OutTransforms{ };
    };

    struct DZ_API LocalToModelJobDesc
    {
        OzzContext     *Context = nullptr;
        Float_4x4Array *OutTransforms{ };
    };

    struct DZ_API SkinningJobDesc
    {
        OzzContext          *Context = nullptr;
        const Float_4x4Array JointTransforms;
        const FloatArray     Vertices;
        const FloatArray     Weights;
        const UInt16Array    Indices;
        int                  InfluenceCount = 0;

        FloatArray *OutVertices{ };
        FloatArray *OutNormals{ };
        FloatArray *OutTangents{ };
    };

    struct DZ_API IkTwoBoneJobResult
    {
        bool    Success{ false };
        Float_4 StartJointCorrection{ };
        Float_4 MidJointCorrection{ };
        bool    Reached{ };
    };

    struct DZ_API IkTwoBoneJobDesc
    {
        Float_4x4 StartJointMatrix{ };
        Float_4x4 MidJointMatrix{ };
        Float_4x4 EndJointMatrix{ };
        Float_3   Target{ 0, 0, 1 };
        Float_3   PoleVector{ 0, 0, 1 };
        Float_3   MidAxis{ 0, 0, 1 };
        float     Weight     = 0;
        float     TwistAngle = 0;
        float     Soften     = 0;
    };

    struct DZ_API IkAimJobResult
    {
        bool    Success{ false };
        Float_4 JointCorrection{ };
    };

    struct DZ_API IkAimJobDesc
    {
        OzzContext *Context    = nullptr;
        int         JointIndex = -1;
        Float_3     Target{ 0, 0, 1 };
        Float_3     Forward{ 0, 0, 1 };
        Float_3     Up{ 0, 1, 0 };
        float       Weight = 1.0f;
    };

    enum class TrackSamplingResultType
    {
        Float,
        Float2,
        Float3,
        Float4,
        Quaternion
    };
    struct DZ_API TrackSamplingResult
    {
        bool                    Success{ false };
        float                   FloatValue{ 0.0f };
        Float_2                 Float2Value{ };
        Float_3                 Float3Value{ };
        Float_4                 Float4Value{ };
        Float_4                 QuaternionValue{ };
        TrackSamplingResultType Type{ TrackSamplingResultType::Float };
    };
    struct DZ_API TrackSamplingJobDesc
    {
        OzzContext             *Context    = nullptr;
        int                     TrackIndex = -1;
        TrackSamplingResultType Type       = TrackSamplingResultType::Float;
        float                   Ratio      = 0.0f;
    };

    struct DZ_API TrackTriggeringResult
    {
    private:
        std::vector<float> m_triggered;
        friend class OzzAnimation;
    public:
        bool       Success{ false };
        FloatArray Triggered;
    };

    struct DZ_API TrackTriggeringJobDesc
    {
        OzzContext *Context       = nullptr;
        int         TrackIndex    = -1;
        float       PreviousRatio = 0.0f;
        float       Ratio         = 0.0f;
    };

    class OzzAnimation
    {
        class Impl;
        Impl *m_impl;

    public:
        DZ_API explicit OzzAnimation( const SkeletonAsset *skeleton );
        DZ_API ~OzzAnimation( );

        DZ_API [[nodiscard]] OzzContext *NewContext( ) const;
        DZ_API void                      DestroyContext( OzzContext *context ) const;

        DZ_API void        LoadAnimation( const AnimationAsset *animation, OzzContext *context ) const;
        DZ_API static void UnloadAnimation( OzzContext *context );

        DZ_API static void LoadTrack( const FloatArray &keys, float duration, OzzContext *context );
        DZ_API static void LoadTrack( const Float_2Array &keys, const FloatArray &timestamps, OzzContext *context );
        DZ_API static void LoadTrack( const Float_3Array &keys, const FloatArray &timestamps, OzzContext *context );
        DZ_API static void LoadTrack( const Float_4Array &keys, const FloatArray &timestamps, OzzContext *context );

        DZ_API [[nodiscard]] bool           RunSamplingJob( const SamplingJobDesc &desc ) const;
        DZ_API [[nodiscard]] bool           RunBlendingJob( const BlendingJobDesc &desc ) const;
        DZ_API [[nodiscard]] bool           RunLocalToModelJob( const LocalToModelJobDesc &desc ) const;
        DZ_API static bool                  RunSkinningJob( const SkinningJobDesc &desc );
        DZ_API static IkTwoBoneJobResult    RunIkTwoBoneJob( const IkTwoBoneJobDesc &desc );
        DZ_API [[nodiscard]] IkAimJobResult RunIkAimJob( const IkAimJobDesc &desc ) const;
        DZ_API static TrackSamplingResult   RunTrackSamplingJob( const TrackSamplingJobDesc &desc );
        DZ_API static TrackTriggeringResult RunTrackTriggeringJob( const TrackTriggeringJobDesc &desc );

        DZ_API InteropStringArray GetJointNames( ) const;
        DZ_API [[nodiscard]] int  GetNumSoaJoints( ) const;
        DZ_API [[nodiscard]] int  GetNumJoints( ) const;
        DZ_API static float       GetAnimationDuration( OzzContext *context );
    };
} // namespace DenOfIz
