using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public static class Ozz
    {
        public static void UnloadAnimation(OzzContext? context)
        {
            Methods.DenOfIz_Ozz_UnloadAnimation(context);
        }

        public static void LoadTrackFloat(in FloatArray keys, float duration, OzzContext? context)
        {
            Methods.DenOfIz_Ozz_LoadTrackFloat(in keys, duration, context);
        }

        public static void LoadTrackFloat2(in Float2Array keys, in FloatArray timestamps, OzzContext? context)
        {
            Methods.DenOfIz_Ozz_LoadTrackFloat2(in keys, in timestamps, context);
        }

        public static void LoadTrackFloat3(in Float3Array keys, in FloatArray timestamps, OzzContext? context)
        {
            Methods.DenOfIz_Ozz_LoadTrackFloat3(in keys, in timestamps, context);
        }

        public static void LoadTrackFloat4(in Float4Array keys, in FloatArray timestamps, OzzContext? context)
        {
            Methods.DenOfIz_Ozz_LoadTrackFloat4(in keys, in timestamps, context);
        }

        public static bool RunSkinningJob(in SkinningJobDesc desc)
        {
            return Methods.DenOfIz_Ozz_RunSkinningJob(in desc);
        }

        public static IkTwoBoneJobResult RunIkTwoBoneJob(in IkTwoBoneJobDesc desc)
        {
            return Methods.DenOfIz_Ozz_RunIkTwoBoneJob(in desc);
        }

        public static TrackSamplingResult RunTrackSamplingJob(in TrackSamplingJobDesc desc)
        {
            return Methods.DenOfIz_Ozz_RunTrackSamplingJob(in desc);
        }

        public static float GetAnimationDuration(OzzContext? context)
        {
            return Methods.DenOfIz_Ozz_GetAnimationDuration(context);
        }

        public static StringView GetAnimationName(OzzContext? context)
        {
            return Methods.DenOfIz_Ozz_GetAnimationName(context);
        }

        public static int GetAnimationNumTracks(OzzContext? context)
        {
            return Methods.DenOfIz_Ozz_GetAnimationNumTracks(context);
        }
    }
}
