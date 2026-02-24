using System;
using System.Numerics;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class TextRenderer : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(TextRenderer wrapper) => wrapper?.Handle ?? 0;

        public TextRenderer(in TextRendererDesc desc)
        {
            Handle = Methods.DenOfIz_TextRenderer_Create(in desc);
            _ownsHandle = true;
        }

        internal TextRenderer(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void SetProjectionMatrix(in Matrix4x4 projectionMatrix)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextRenderer_SetProjectionMatrix(Handle, in projectionMatrix);
        }

        public void SetViewport(in Viewport viewport)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextRenderer_SetViewport(Handle, in viewport);
        }

        public ushort AddFont(Font? font, ushort fontId)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextRenderer_AddFont(Handle, font, fontId);
        }

        public Font GetFont(ushort fontId)
        {
            ThrowIfDisposed();
            return new Font(Methods.DenOfIz_TextRenderer_GetFont(Handle, fontId), ownsHandle: false);
        }

        public void RemoveFont(ushort fontId)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextRenderer_RemoveFont(Handle, fontId);
        }

        public void BeginBatch()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextRenderer_BeginBatch(Handle);
        }

        public void AddText(in TextRenderDesc textDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextRenderer_AddText(Handle, in textDesc);
        }

        public void EndBatch(CommandList? commandList)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_TextRenderer_EndBatch(Handle, commandList);
        }

        public Vector2 MeasureText(StringView text, in TextRenderDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_TextRenderer_MeasureText(Handle, text, in desc);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_TextRenderer_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(TextRenderer));
            }
        }
    }
}
