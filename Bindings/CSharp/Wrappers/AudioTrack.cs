using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class AudioTrack : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(AudioTrack wrapper) => wrapper?.Handle ?? 0;

        public AudioTrack(AudioMixer mixer)
        {
            Handle = Methods.DenOfIz_AudioTrack_Create(mixer);
            _ownsHandle = true;
        }

        internal AudioTrack(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public bool SetAudio(AudioSource source)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_SetAudio(Handle, source);
        }

        public bool Play(in AudioPlayDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_Play(Handle, in desc);
        }

        public bool Stop(int fadeOutMs)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_Stop(Handle, fadeOutMs);
        }

        public bool Pause()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_Pause(Handle);
        }

        public bool Resume()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_Resume(Handle);
        }

        public bool IsPlaying()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_IsPlaying(Handle);
        }

        public bool IsPaused()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_IsPaused(Handle);
        }

        public void SetGain(float gain)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioTrack_SetGain(Handle, gain);
        }

        public float GetGain()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_GetGain(Handle);
        }

        public void SetLoops(int loops)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioTrack_SetLoops(Handle, loops);
        }

        public int GetLoops()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_GetLoops(Handle);
        }

        public bool SetPositionMs(long ms)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_SetPositionMs(Handle, ms);
        }

        public long GetPositionMs()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_GetPositionMs(Handle);
        }

        public long GetRemainingMs()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_GetRemainingMs(Handle);
        }

        public bool Tag(StringView tag)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioTrack_Tag(Handle, tag);
        }

        public void Untag(StringView tag)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioTrack_Untag(Handle, tag);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_AudioTrack_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(AudioTrack));
            }
        }
    }
}
