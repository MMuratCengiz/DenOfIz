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

    struct DZ_API SamplingJobDesc
    {
        OzzContext              *Context       = nullptr;
        float                    Ratio         = 0.0f;
        InteropArray<Float_4x4> *OutTransforms = nullptr;
    };

    struct DZ_API BlendingJobDesc
    {
        struct Layer
        {
            const InteropArray<Float_4x4> *Transforms = nullptr;
            float                          Weight     = 0.0f;
        };

        OzzContext              *Context = nullptr;
        InteropArray<Layer>      Layers;
        InteropArray<Float_4x4> *OutTransforms = nullptr;
        float                    Threshold     = 0.1f;
    };
    template class DZ_API InteropArray<BlendingJobDesc::Layer>;

    struct DZ_API LocalToModelJobDesc
    {
        OzzContext              *Context       = nullptr;
        InteropArray<Float_4x4> *InTransforms  = nullptr;
        InteropArray<Float_4x4> *OutTransforms = nullptr;
    };

    struct DZ_API SkinningJobDesc
    {
        OzzContext                    *Context           = nullptr;
        const InteropArray<Float_4x4> *InJointTransforms = nullptr;
        const InteropArray<float>     *InVertices        = nullptr;
        const InteropArray<float>     *InWeights         = nullptr;
        const InteropArray<uint16_t>  *InIndices         = nullptr;
        InteropArray<float>           *OutVertices       = nullptr;
        int                            VertexCount       = 0;
        int                            InfluenceCount    = 0;
    };

    struct DZ_API IkTwoBoneJobDesc
    {
        Float_4x4 StartJointMatrix{ };
        Float_4x4 MidJointMatrix{ };
        Float_4x4 EndJointMatrix{ };
        Float_3   Target{ 0, 0, 1 };
        Float_3   PoleVector{ 0, 0, 1 };
        Float_3   MidAxis{ 0, 0, 1 };
        float     Weight                  = 0;
        float     TwistAngle              = 0;
        float     Soften                  = 0;
        Float_4  *OutStartJointCorrection = nullptr;
        Float_4  *OutMidJointCorrection   = nullptr;
        bool     *OutReached              = nullptr;
    };

    struct DZ_API IkAimJobDesc
    {
        OzzContext *Context    = nullptr;
        int         JointIndex = -1;
        Float_3     Target{ 0, 0, 1 };
        Float_3     Forward{ 0, 0, 1 };
        Float_3     Up{ 0, 1, 0 };
        float       Weight             = 1.0f;
        Float_4    *OutJointCorrection = nullptr;
    };

    struct DZ_API TrackSamplingJobDesc
    {
        enum class ValueType
        {
            Float,
            Float2,
            Float3,
            Float4,
            Quaternion
        };

        OzzContext *Context    = nullptr;
        int         TrackIndex = -1;
        ValueType   Type       = ValueType::Float;
        float       Ratio      = 0.0f;
        void       *OutResult  = nullptr;
    };

    struct DZ_API TrackTriggeringJobDesc
    {
        OzzContext          *Context       = nullptr;
        int                  TrackIndex    = -1;
        float                PreviousRatio = 0.0f;
        float                Ratio         = 0.0f;
        InteropArray<float> *OutTriggered  = nullptr;
    };

    class DZ_API OzzAnimation
    {
        class Impl;
        Impl *m_impl;

    public:
        explicit OzzAnimation( const SkeletonAsset *skeleton );
        ~OzzAnimation( );

        [[nodiscard]] OzzContext *NewContext( ) const;
        void                      DestroyContext( OzzContext *context ) const;

        void        LoadAnimation( const AnimationAsset *animation, OzzContext *context ) const;
        static void UnloadAnimation( OzzContext *context );

        static void LoadTrack( const InteropArray<float> &keys, float duration, OzzContext *context );
        static void LoadTrack( const InteropArray<Float_2> &keys, const InteropArray<float> &timestamps, OzzContext *context );
        static void LoadTrack( const InteropArray<Float_3> &keys, const InteropArray<float> &timestamps, OzzContext *context );
        static void LoadTrack( const InteropArray<Float_4> &keys, const InteropArray<float> &timestamps, OzzContext *context );

        bool               RunSamplingJob( const SamplingJobDesc &desc ) const;
        [[nodiscard]] bool RunBlendingJob( const BlendingJobDesc &desc ) const;
        [[nodiscard]] bool RunLocalToModelJob( const LocalToModelJobDesc &desc ) const;
        static bool               RunSkinningJob( const SkinningJobDesc &desc );
        static bool        RunIkTwoBoneJob( const IkTwoBoneJobDesc &desc );
        [[nodiscard]] bool RunIkAimJob( const IkAimJobDesc &desc ) const;
        static bool        RunTrackSamplingJob( const TrackSamplingJobDesc &desc );
        static bool               RunTrackTriggeringJob( const TrackTriggeringJobDesc &desc );

        void              GetJointNames( InteropArray<InteropString> &outNames ) const;
        [[nodiscard]] int GetJointCount( ) const;
        static float      GetAnimationDuration( OzzContext *context );
    };
} // namespace DenOfIz
