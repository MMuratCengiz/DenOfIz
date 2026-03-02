using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class AudioMixer : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(AudioMixer wrapper) => wrapper?.Handle ?? 0;

        public AudioMixer(in AudioSpec spec)
        {
            Handle = Methods.DenOfIz_AudioMixer_Create(in spec);
            _ownsHandle = true;
        }

        internal AudioMixer(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void SetGain(float gain)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioMixer_SetGain(Handle, gain);
        }

        public float GetGain()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioMixer_GetGain(Handle);
        }

        public void StopAll(int fadeOutMs)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioMixer_StopAll(Handle, fadeOutMs);
        }

        public void PauseAll()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioMixer_PauseAll(Handle);
        }

        public void ResumeAll()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_AudioMixer_ResumeAll(Handle);
        }

        public static int GetNumDecoders()
        {
            return Methods.DenOfIz_AudioMixer_GetNumDecoders();
        }

        public static StringView GetDecoder(int index)
        {
            return Methods.DenOfIz_AudioMixer_GetDecoder(index);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_AudioMixer_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(AudioMixer));
            }
        }
    }
}
