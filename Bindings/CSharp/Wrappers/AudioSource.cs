using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class AudioSource : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(AudioSource wrapper) => wrapper?.Handle ?? 0;

        public AudioSource(AudioMixer mixer, StringView filePath, bool predecode)
        {
            Handle = Methods.DenOfIz_AudioSource_LoadFromFile(mixer, filePath, predecode);
            _ownsHandle = true;
        }

        internal AudioSource(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public long GetDuration()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioSource_GetDuration(Handle);
        }

        public AudioSpec GetFormat()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioSource_GetFormat(Handle);
        }

        public StringView GetTitle()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioSource_GetTitle(Handle);
        }

        public StringView GetArtist()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioSource_GetArtist(Handle);
        }

        public StringView GetAlbum()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_AudioSource_GetAlbum(Handle);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_AudioSource_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(AudioSource));
            }
        }
    }
}
