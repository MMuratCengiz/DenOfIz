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

#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{
    class DZ_API OzzContext
    {
        OzzContext( ) = default;
        friend class OzzAnimation;
    };

    struct DZ_API SamplingJobResult
    {
        bool                    Success{ false };
        InteropArray<Float_4x4> Transforms{ };
    };

    struct DZ_API SamplingJobDesc
    {
        OzzContext *Context = nullptr;
        float       Ratio   = 0.0f;
    };

    struct DZ_API BlendingJobResult
    {
        bool                    Success{ false };
        InteropArray<Float_4x4> Transforms{ };
    };

    struct DZ_API BlendingJobLayerDesc
    {
        InteropArray<Float_4x4> Transforms{ }; // Todo this could potentially be expensive copying
        float                   Weight = 0.0f;
    };
    template class DZ_API InteropArray<BlendingJobLayerDesc>;

    struct DZ_API BlendingJobDesc
    {
        OzzContext                        *Context = nullptr;
        InteropArray<BlendingJobLayerDesc> Layers;
        float                              Threshold = 0.1f;
    };

    struct DZ_API LocalToModelJobResult
    {
        bool                    Success{ false };
        InteropArray<Float_4x4> Transforms{ };
    };

    struct DZ_API LocalToModelJobDesc
    {
        OzzContext *Context = nullptr;
    };

    struct DZ_API SkinningJobResult
    {
        bool                   Success{ false };
        InteropArray<float>    Vertices{ };
        InteropArray<float>    Weights{ };
        InteropArray<uint16_t> Indices{ };
    };
    struct DZ_API SkinningJobDesc
    {
        OzzContext                   *Context = nullptr;
        const InteropArray<Float_4x4> JointTransforms;
        const InteropArray<float>     Vertices;
        const InteropArray<float>     Weights;
        const InteropArray<uint16_t>  Indices;
        int                           InfluenceCount = 0;
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
        bool                Success{ false };
        InteropArray<float> Triggered;
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

        DZ_API static void LoadTrack( const InteropArray<float> &keys, float duration, OzzContext *context );
        DZ_API static void LoadTrack( const InteropArray<Float_2> &keys, const InteropArray<float> &timestamps, OzzContext *context );
        DZ_API static void LoadTrack( const InteropArray<Float_3> &keys, const InteropArray<float> &timestamps, OzzContext *context );
        DZ_API static void LoadTrack( const InteropArray<Float_4> &keys, const InteropArray<float> &timestamps, OzzContext *context );

        DZ_API [[nodiscard]] SamplingJobResult     RunSamplingJob( const SamplingJobDesc &desc ) const;
        DZ_API [[nodiscard]] BlendingJobResult     RunBlendingJob( const BlendingJobDesc &desc ) const;
        DZ_API [[nodiscard]] LocalToModelJobResult RunLocalToModelJob( const LocalToModelJobDesc &desc ) const;
        DZ_API static SkinningJobResult            RunSkinningJob( const SkinningJobDesc &desc );
        DZ_API static IkTwoBoneJobResult           RunIkTwoBoneJob( const IkTwoBoneJobDesc &desc );
        DZ_API [[nodiscard]] IkAimJobResult        RunIkAimJob( const IkAimJobDesc &desc ) const;
        DZ_API static TrackSamplingResult          RunTrackSamplingJob( const TrackSamplingJobDesc &desc );
        DZ_API static TrackTriggeringResult        RunTrackTriggeringJob( const TrackTriggeringJobDesc &desc );

        DZ_API void              GetJointNames( InteropArray<InteropString> &outNames ) const;
        DZ_API [[nodiscard]] int GetJointCount( ) const;
        DZ_API static float      GetAnimationDuration( OzzContext *context );
    };
} // namespace DenOfIz
