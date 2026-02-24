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

#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_OzzSkeleton )
    DENOFIZ_DEFINE_HANDLE( DenOfIz_OzzContext )

    typedef struct DenOfIz_OzzJointTransform
    {
        DenOfIz_Float3 Translation;
        DenOfIz_Float4 Rotation;
        DenOfIz_Float3 Scale;
    } DenOfIz_OzzJointTransform;

    typedef struct DenOfIz_OzzJointTransformArray
    {
        DenOfIz_OzzJointTransform *Elements;
        size_t                     NumElements;
    } DenOfIz_OzzJointTransformArray;

    typedef struct DenOfIz_OzzJoint
    {
        DenOfIz_StringView        Name;
        int32_t                   Index;
        int32_t                   ParentIndex;
        DenOfIz_OzzJointTransform RestPose;
    } DenOfIz_OzzJoint;

    typedef struct DenOfIz_OzzJointArray
    {
        DenOfIz_OzzJoint *Elements;
        size_t            NumElements;
    } DenOfIz_OzzJointArray;

    typedef struct DenOfIz_SamplingJobDesc
    {
        DenOfIz_OzzContext    Context;
        float                 Ratio;
        DenOfIz_Float4x4Array OutTransforms;
    } DenOfIz_SamplingJobDesc;

    typedef struct DenOfIz_SamplingJobLocalDesc
    {
        DenOfIz_OzzContext             Context;
        float                          Ratio;
        DenOfIz_OzzJointTransformArray OutTransforms;
    } DenOfIz_SamplingJobLocalDesc;

    typedef struct DenOfIz_LocalToModelFromTRSDesc
    {
        DenOfIz_OzzJointTransformArray LocalTransforms;
        DenOfIz_Float4x4Array          OutTransforms;
    } DenOfIz_LocalToModelFromTRSDesc;

    typedef struct DenOfIz_BlendingJobLayerDesc
    {
        DenOfIz_Float4x4Array Transforms;
        float                 Weight;
    } DenOfIz_BlendingJobLayerDesc;

    typedef struct DenOfIz_BlendingJobLayerDescArray
    {
        DenOfIz_BlendingJobLayerDesc *Elements;
        uint32_t                      NumElements;
    } DenOfIz_BlendingJobLayerDescArray;

    typedef struct DenOfIz_BlendingJobDesc
    {
        DenOfIz_OzzContext                Context;
        DenOfIz_BlendingJobLayerDescArray Layers;
        float                             Threshold;
        DenOfIz_Float4x4Array             OutTransforms;
    } DenOfIz_BlendingJobDesc;

    typedef struct DenOfIz_LocalToModelJobDesc
    {
        DenOfIz_OzzContext    Context;
        DenOfIz_Float4x4Array OutTransforms;
    } DenOfIz_LocalToModelJobDesc;

    typedef struct DenOfIz_SkinningJobDesc
    {
        DenOfIz_OzzContext    Context;
        DenOfIz_Float4x4Array JointTransforms;
        DenOfIz_FloatArray    Vertices;
        DenOfIz_FloatArray    Weights;
        DenOfIz_UInt16Array   Indices;
        int                   InfluenceCount;
        DenOfIz_FloatArray    OutVertices;
        DenOfIz_FloatArray    OutNormals;
        DenOfIz_FloatArray    OutTangents;
    } DenOfIz_SkinningJobDesc;

    typedef struct DenOfIz_IkTwoBoneJobResult
    {
        bool           Success;
        DenOfIz_Float4 StartJointCorrection;
        DenOfIz_Float4 MidJointCorrection;
        bool           Reached;
    } DenOfIz_IkTwoBoneJobResult;

    typedef struct DenOfIz_IkTwoBoneJobDesc
    {
        DenOfIz_Float4x4 StartJointMatrix;
        DenOfIz_Float4x4 MidJointMatrix;
        DenOfIz_Float4x4 EndJointMatrix;
        DenOfIz_Float3   Target;
        DenOfIz_Float3   PoleVector;
        DenOfIz_Float3   MidAxis;
        float            Weight;
        float            TwistAngle;
        float            Soften;
    } DenOfIz_IkTwoBoneJobDesc;

    typedef struct DenOfIz_IkAimJobResult
    {
        bool           Success;
        DenOfIz_Float4 JointCorrection;
    } DenOfIz_IkAimJobResult;

    typedef struct DenOfIz_IkAimJobDesc
    {
        DenOfIz_OzzContext Context;
        int                JointIndex;
        DenOfIz_Float3     Target;
        DenOfIz_Float3     Forward;
        DenOfIz_Float3     Up;
        float              Weight;
    } DenOfIz_IkAimJobDesc;

    typedef enum DenOfIz_TrackSamplingResultType
    {
        DenOfIz_TrackSamplingResultType_Float,
        DenOfIz_TrackSamplingResultType_Float2,
        DenOfIz_TrackSamplingResultType_Float3,
        DenOfIz_TrackSamplingResultType_Float4,
        DenOfIz_TrackSamplingResultType_Quaternion
    } DenOfIz_TrackSamplingResultType;

    typedef struct DenOfIz_TrackSamplingResult
    {
        bool                            Success;
        float                           FloatValue;
        DenOfIz_Float2                  Float2Value;
        DenOfIz_Float3                  Float3Value;
        DenOfIz_Float4                  Float4Value;
        DenOfIz_Float4                  QuaternionValue;
        DenOfIz_TrackSamplingResultType Type;
    } DenOfIz_TrackSamplingResult;

    typedef struct DenOfIz_TrackSamplingJobDesc
    {
        DenOfIz_OzzContext              Context;
        int                             TrackIndex;
        DenOfIz_TrackSamplingResultType Type;
        float                           Ratio;
    } DenOfIz_TrackSamplingJobDesc;

    typedef struct DenOfIz_TrackTriggeringEdge
    {
        float Ratio;
        bool  Rising;
    } DenOfIz_TrackTriggeringEdge;

    typedef struct DenOfIz_TrackTriggeringEdgeArray
    {
        DenOfIz_TrackTriggeringEdge *Elements;
        size_t                       NumElements;
    } DenOfIz_TrackTriggeringEdgeArray;

    typedef struct DenOfIz_TrackTriggeringResult
    {
        bool                             Success;
        DenOfIz_TrackTriggeringEdgeArray Edges;
    } DenOfIz_TrackTriggeringResult;

    typedef struct DenOfIz_TrackTriggeringJobDesc
    {
        DenOfIz_OzzContext Context;
        int                TrackIndex;
        float              PreviousRatio;
        float              Ratio;
    } DenOfIz_TrackTriggeringJobDesc;

    DZ_API DenOfIz_OzzSkeleton DenOfIz_Ozz_CreateSkeleton( DenOfIz_StringView skeletonFilePath );
    DZ_API DenOfIz_OzzSkeleton DenOfIz_Ozz_CreateSkeletonFromBinaryContainer( DenOfIz_BinaryContainer skeletonData );
    DZ_API void                DenOfIz_Ozz_DestroySkeleton( DenOfIz_OzzSkeleton skeleton );
    DZ_API bool                DenOfIz_Ozz_IsSkeletonValid( DenOfIz_OzzSkeleton skeleton );

    DZ_API DenOfIz_OzzContext DenOfIz_Ozz_NewContext( DenOfIz_OzzSkeleton skeleton );
    DZ_API void               DenOfIz_Ozz_DestroyContext( DenOfIz_OzzSkeleton skeleton, DenOfIz_OzzContext context );

    DZ_API bool DenOfIz_Ozz_LoadAnimation( DenOfIz_OzzSkeleton skeleton, DenOfIz_StringView animationFilePath, DenOfIz_OzzContext context );
    DZ_API bool DenOfIz_Ozz_LoadAnimationFromBinaryContainer( DenOfIz_OzzSkeleton skeleton, DenOfIz_BinaryContainer animationData, DenOfIz_OzzContext context );
    DZ_API void DenOfIz_Ozz_UnloadAnimation( DenOfIz_OzzContext context );

    DZ_API void DenOfIz_Ozz_LoadTrackFloat( const DenOfIz_FloatArray *keys, float duration, DenOfIz_OzzContext context );
    DZ_API void DenOfIz_Ozz_LoadTrackFloat2( const DenOfIz_Float2Array *keys, const DenOfIz_FloatArray *timestamps, DenOfIz_OzzContext context );
    DZ_API void DenOfIz_Ozz_LoadTrackFloat3( const DenOfIz_Float3Array *keys, const DenOfIz_FloatArray *timestamps, DenOfIz_OzzContext context );
    DZ_API void DenOfIz_Ozz_LoadTrackFloat4( const DenOfIz_Float4Array *keys, const DenOfIz_FloatArray *timestamps, DenOfIz_OzzContext context );

    DZ_API bool                          DenOfIz_Ozz_RunSamplingJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_SamplingJobDesc *desc );
    DZ_API bool                          DenOfIz_Ozz_RunSamplingJobLocal( DenOfIz_OzzSkeleton skeleton, const DenOfIz_SamplingJobLocalDesc *desc );
    DZ_API bool                          DenOfIz_Ozz_RunBlendingJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_BlendingJobDesc *desc );
    DZ_API bool                          DenOfIz_Ozz_RunLocalToModelJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_LocalToModelJobDesc *desc );
    DZ_API bool                          DenOfIz_Ozz_RunLocalToModelFromTRS( DenOfIz_OzzSkeleton skeleton, const DenOfIz_LocalToModelFromTRSDesc *desc );
    DZ_API bool                          DenOfIz_Ozz_RunSkinningJob( const DenOfIz_SkinningJobDesc *desc );
    DZ_API DenOfIz_IkTwoBoneJobResult    DenOfIz_Ozz_RunIkTwoBoneJob( const DenOfIz_IkTwoBoneJobDesc *desc );
    DZ_API DenOfIz_IkAimJobResult        DenOfIz_Ozz_RunIkAimJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_IkAimJobDesc *desc );
    DZ_API DenOfIz_TrackSamplingResult   DenOfIz_Ozz_RunTrackSamplingJob( const DenOfIz_TrackSamplingJobDesc *desc );
    DZ_API DenOfIz_TrackTriggeringResult DenOfIz_Ozz_RunTrackTriggeringJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_TrackTriggeringJobDesc *desc );

    DZ_API int                            DenOfIz_Ozz_GetNumJoints( DenOfIz_OzzSkeleton skeleton );
    DZ_API int                            DenOfIz_Ozz_GetNumSoaJoints( DenOfIz_OzzSkeleton skeleton );
    DZ_API DenOfIz_StringViewArray        DenOfIz_Ozz_GetJointNames( DenOfIz_OzzSkeleton skeleton );
    DZ_API DenOfIz_Int32Array             DenOfIz_Ozz_GetJointParents( DenOfIz_OzzSkeleton skeleton );
    DZ_API int32_t                        DenOfIz_Ozz_FindJoint( DenOfIz_OzzSkeleton skeleton, DenOfIz_StringView name );
    DZ_API DenOfIz_OzzJointArray          DenOfIz_Ozz_GetJoints( DenOfIz_OzzSkeleton skeleton );
    DZ_API DenOfIz_OzzJointTransformArray DenOfIz_Ozz_GetRestPoses( DenOfIz_OzzSkeleton skeleton );

    DZ_API float              DenOfIz_Ozz_GetAnimationDuration( DenOfIz_OzzContext context );
    DZ_API DenOfIz_StringView DenOfIz_Ozz_GetAnimationName( DenOfIz_OzzContext context );
    DZ_API int                DenOfIz_Ozz_GetAnimationNumTracks( DenOfIz_OzzContext context );

#ifdef __cplusplus
}
#endif
