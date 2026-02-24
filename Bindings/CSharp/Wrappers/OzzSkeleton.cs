using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class OzzSkeleton : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(OzzSkeleton wrapper) => wrapper?.Handle ?? 0;

        public OzzSkeleton(StringView skeletonFilePath)
        {
            Handle = Methods.DenOfIz_Ozz_CreateSkeleton(skeletonFilePath);
            _ownsHandle = true;
        }

        internal OzzSkeleton(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public bool IsValid()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_IsSkeletonValid(Handle);
        }

        public OzzContext NewContext()
        {
            ThrowIfDisposed();
            return new OzzContext(Methods.DenOfIz_Ozz_NewContext(Handle), ownsHandle: false);
        }

        public void DestroyContext(OzzContext? context)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Ozz_DestroyContext(Handle, context);
        }

        public bool LoadAnimation(StringView animationFilePath, OzzContext? context)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_LoadAnimation(Handle, animationFilePath, context);
        }

        public bool LoadAnimationFromBinaryContainer(BinaryContainer? animationData, OzzContext? context)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_LoadAnimationFromBinaryContainer(Handle, animationData, context);
        }

        public bool RunSamplingJob(in SamplingJobDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunSamplingJob(Handle, in desc);
        }

        public bool RunSamplingJobLocal(in SamplingJobLocalDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunSamplingJobLocal(Handle, in desc);
        }

        public bool RunBlendingJob(in BlendingJobDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunBlendingJob(Handle, in desc);
        }

        public bool RunLocalToModelJob(in LocalToModelJobDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunLocalToModelJob(Handle, in desc);
        }

        public bool RunLocalToModelFromTRS(in LocalToModelFromTRSDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunLocalToModelFromTRS(Handle, in desc);
        }

        public IkAimJobResult RunIkAimJob(in IkAimJobDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunIkAimJob(Handle, in desc);
        }

        public TrackTriggeringResult RunTrackTriggeringJob(in TrackTriggeringJobDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_RunTrackTriggeringJob(Handle, in desc);
        }

        public StringViewArray GetJointNames()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_GetJointNames(Handle);
        }

        public int GetNumSoaJoints()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_GetNumSoaJoints(Handle);
        }

        public int GetNumJoints()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_GetNumJoints(Handle);
        }

        public Int32Array GetJointParents()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_GetJointParents(Handle);
        }

        public int FindJoint(StringView name)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_FindJoint(Handle, name);
        }

        public int FindJoint(string name)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_FindJoint(Handle, StringView.Intern(name));
        }

        public OzzJointArray GetJoints()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_GetJoints(Handle);
        }

        public OzzJointTransformArray GetRestPoses()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Ozz_GetRestPoses(Handle);
        }

        public static OzzSkeleton CreateFromBinaryContainer(BinaryContainer? skeletonData)
        {
            return new OzzSkeleton(Methods.DenOfIz_Ozz_CreateSkeletonFromBinaryContainer(skeletonData), ownsHandle: true);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Ozz_DestroySkeleton(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(OzzSkeleton));
            }
        }
    }
}
