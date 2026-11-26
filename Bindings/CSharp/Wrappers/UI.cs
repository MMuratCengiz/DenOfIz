using System;
using System.Buffers;
using System.Numerics;
using System.Text;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class UI : IDisposable
    {
        private const int StackTextBytes = 1024;

        // Must match DENOFIZ_UI_MAX_ROUNDED_CLIPS.
        public const int MaxRoundedClips = 4;

        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;
        private Texture?[] _textures = Array.Empty<Texture?>();
        private Semaphore?[] _semaphores = Array.Empty<Semaphore?>();

        public static implicit operator ulong(UI? wrapper) => wrapper?.Handle ?? 0;

        public UI(in UIDesc desc)
        {
            Handle = Methods.DenOfIz_UI_Create(in desc);
            if (Handle == 0)
            {
                throw new InvalidOperationException("DenOfIz_UI_Create failed, see the native log.");
            }
            _ownsHandle = true;
            _textures = new Texture?[Math.Max(1u, desc.NumFrames)];
            _semaphores = new Semaphore?[Math.Max(1u, desc.NumFrames)];
        }

        internal UI(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public static uint HashId(ReadOnlySpan<char> name, uint seed = 0)
        {
            var maxBytes = Encoding.UTF8.GetMaxByteCount(name.Length);
            byte[]? rented = null;
            var bytes = maxBytes <= StackTextBytes ? stackalloc byte[StackTextBytes] : rented = ArrayPool<byte>.Shared.Rent(maxBytes);
            var length = Encoding.UTF8.GetBytes(name, bytes);
            var hash = HashId(bytes[..length], seed);
            if (rented != null)
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
            return hash;
        }

        public static uint HashId(ReadOnlySpan<byte> utf8, uint seed = 0)
        {
            var hash = 2166136261u ^ Fmix32(seed);
            foreach (var b in utf8)
            {
                hash ^= b;
                hash *= 16777619u;
            }
            hash = Fmix32(hash);
            return hash != 0 ? hash : 1;
        }

        public static uint CombineId(uint id, uint value)
        {
            var hash = Fmix32(id ^ Fmix32(value + 0x9E3779B9u));
            return hash != 0 ? hash : 1;
        }

        private static uint Fmix32(uint h)
        {
            h ^= h >> 16;
            h *= 0x85EBCA6Bu;
            h ^= h >> 13;
            h *= 0xC2B2AE35u;
            h ^= h >> 16;
            return h;
        }

        public void HandleEvent(in Event ev)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_HandleEvent(Handle, in ev);
        }

        public void SetViewportSize(uint pixelWidth, uint pixelHeight)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_SetViewportSize(Handle, pixelWidth, pixelHeight);
        }

        public Vector2 ViewportSize
        {
            get
            {
                ThrowIfDisposed();
                return Methods.DenOfIz_UI_GetViewportSize(Handle);
            }
        }

        public void SetDisplayScale(float displayScale)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_SetDisplayScale(Handle, displayScale);
        }

        public void SetScale(float scale)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_SetScale(Handle, scale);
        }

        public float PixelsPerPoint
        {
            get
            {
                ThrowIfDisposed();
                return Methods.DenOfIz_UI_GetPixelsPerPoint(Handle);
            }
        }

        public void SetDebugFlags(UIDebugFlagBits flags)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_SetDebugFlags(Handle, (uint)flags);
        }

        public void AddFont(ushort fontId, Font font)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_AddFont(Handle, fontId, font);
        }

        public void RemoveFont(ushort fontId)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_RemoveFont(Handle, fontId);
        }

        public unsafe void SetFontFallbacks(ushort fontId, ReadOnlySpan<ushort> fallbackIds)
        {
            ThrowIfDisposed();
            fixed (ushort* elements = fallbackIds)
            {
                var array = new UInt16Array { Elements = (IntPtr)elements, NumElements = (ulong)fallbackIds.Length };
                Methods.DenOfIz_UI_SetFontFallbacks(Handle, fontId, in array);
            }
        }

        public void BeginFrame(float deltaTime)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_BeginFrame(Handle, deltaTime);
        }

        public uint OpenElement(in UIElementDesc desc)
        {
            return Methods.DenOfIz_UI_OpenElement(Handle, in desc);
        }

        public void CloseElement()
        {
            Methods.DenOfIz_UI_CloseElement(Handle);
        }

        public uint Text(StringView text, in UITextDesc desc)
        {
            return Methods.DenOfIz_UI_Text(Handle, text, in desc);
        }

        public unsafe uint Text(ReadOnlySpan<byte> utf8, in UITextDesc desc)
        {
            fixed (byte* chars = utf8)
            {
                return Methods.DenOfIz_UI_Text(Handle, new StringView { Chars = (IntPtr)chars, NumChars = (ulong)utf8.Length }, in desc);
            }
        }

        public unsafe uint RichText(ReadOnlySpan<byte> utf8, ReadOnlySpan<UITextRun> runs, in UITextDesc desc)
        {
            fixed (byte* chars = utf8)
            fixed (UITextRun* elements = runs)
            {
                var array = new UITextRunArray { Elements = (IntPtr)elements, NumElements = (ulong)runs.Length };
                return Methods.DenOfIz_UI_RichText(Handle, new StringView { Chars = (IntPtr)chars, NumChars = (ulong)utf8.Length }, in array, in desc);
            }
        }

        public uint Text(ReadOnlySpan<char> text, in UITextDesc desc)
        {
            var maxBytes = Encoding.UTF8.GetMaxByteCount(text.Length);
            byte[]? rented = null;
            var bytes = maxBytes <= StackTextBytes ? stackalloc byte[StackTextBytes] : rented = ArrayPool<byte>.Shared.Rent(maxBytes);
            var length = Encoding.UTF8.GetBytes(text, bytes);
            var id = Text(bytes[..length], in desc);
            if (rented != null)
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
            return id;
        }

        public void EndFrame()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_EndFrame(Handle);
        }

        public void Render(uint frameIndex, CommandList commandList)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_Render(Handle, frameIndex, commandList);
        }

        public (Texture Texture, Semaphore Semaphore) RenderToTexture(uint frameIndex)
        {
            return RenderToTexture(frameIndex, new SemaphoreArray());
        }

        public unsafe (Texture Texture, Semaphore Semaphore) RenderToTexture(uint frameIndex, ReadOnlySpan<Semaphore> waitSemaphores)
        {
            Span<ulong> handles = stackalloc ulong[waitSemaphores.Length];
            for (var i = 0; i < waitSemaphores.Length; ++i)
            {
                handles[i] = waitSemaphores[i];
            }
            fixed (ulong* elements = handles)
            {
                return RenderToTexture(frameIndex, new SemaphoreArray { Elements = (IntPtr)elements, NumElements = (ulong)handles.Length });
            }
        }

        public (Texture Texture, Semaphore Semaphore) RenderToTexture(uint frameIndex, in SemaphoreArray waitSemaphores)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_UI_RenderToTexture(Handle, frameIndex, in waitSemaphores, out var result);
            if (frameIndex >= _textures.Length)
            {
                Array.Resize(ref _textures, (int)frameIndex + 1);
                Array.Resize(ref _semaphores, (int)frameIndex + 1);
            }
            var texture = _textures[frameIndex];
            if (texture == null || (ulong)texture != result.Texture)
            {
                texture = new Texture(result.Texture, ownsHandle: false);
                _textures[frameIndex] = texture;
            }
            var semaphore = _semaphores[frameIndex];
            if (semaphore == null || (ulong)semaphore != result.Semaphore)
            {
                semaphore = new Semaphore(result.Semaphore, ownsHandle: false);
                _semaphores[frameIndex] = semaphore;
            }
            return (texture, semaphore);
        }

        public UIInteraction GetInteraction(uint id)
        {
            Methods.DenOfIz_UI_GetInteraction(Handle, id, out var interaction);
            return interaction;
        }

        public bool TryGetElementRect(uint id, out UIRect rect)
        {
            return Methods.DenOfIz_UI_GetElementRect(Handle, id, out rect);
        }

        public bool IsPointerOverUI => Methods.DenOfIz_UI_IsPointerOverUI(Handle);

        public uint HoveredId => Methods.DenOfIz_UI_GetHoveredId(Handle);

        public Vector2 PointerPosition => Methods.DenOfIz_UI_GetPointerPosition(Handle);

        public SystemCursor Cursor => Methods.DenOfIz_UI_GetCursor(Handle);

        public uint Focus
        {
            get => Methods.DenOfIz_UI_GetFocus(Handle);
            set => Methods.DenOfIz_UI_SetFocus(Handle, value);
        }

        public void FocusNext(bool reverse = false)
        {
            Methods.DenOfIz_UI_FocusNext(Handle, reverse);
        }

        public bool TryGetScrollState(uint id, out UIScrollState state)
        {
            return Methods.DenOfIz_UI_GetScrollState(Handle, id, out state);
        }

        public void SetScrollOffset(uint id, Vector2 offset)
        {
            Methods.DenOfIz_UI_SetScrollOffset(Handle, id, offset);
        }

        public void ScrollIntoView(uint id)
        {
            Methods.DenOfIz_UI_ScrollIntoView(Handle, id);
        }

        public void ScrollCaretIntoView(uint textId, uint byteOffset)
        {
            Methods.DenOfIz_UI_ScrollCaretIntoView(Handle, textId, byteOffset);
        }

        public unsafe Vector2 MeasureText(ReadOnlySpan<byte> utf8, in UITextDesc desc, float maxWidth = 0)
        {
            fixed (byte* chars = utf8)
            {
                return Methods.DenOfIz_UI_MeasureText(Handle, new StringView { Chars = (IntPtr)chars, NumChars = (ulong)utf8.Length }, in desc, maxWidth);
            }
        }

        public Vector2 MeasureText(ReadOnlySpan<char> text, in UITextDesc desc, float maxWidth = 0)
        {
            var maxBytes = Encoding.UTF8.GetMaxByteCount(text.Length);
            byte[]? rented = null;
            var bytes = maxBytes <= StackTextBytes ? stackalloc byte[StackTextBytes] : rented = ArrayPool<byte>.Shared.Rent(maxBytes);
            var length = Encoding.UTF8.GetBytes(text, bytes);
            var size = MeasureText(bytes[..length], in desc, maxWidth);
            if (rented != null)
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
            return size;
        }

        public bool TryGetTextCaretRect(uint textId, uint byteOffset, out UIRect rect)
        {
            return Methods.DenOfIz_UI_GetTextCaretRect(Handle, textId, byteOffset, out rect);
        }

        public uint GetTextOffsetAtPoint(uint textId, Vector2 point)
        {
            return Methods.DenOfIz_UI_GetTextOffsetAtPoint(Handle, textId, point);
        }

        public uint GetTextRunAtPoint(uint textId, Vector2 point)
        {
            return Methods.DenOfIz_UI_GetTextRunAtPoint(Handle, textId, point);
        }

        public uint GetTextLineCount(uint textId)
        {
            return Methods.DenOfIz_UI_GetTextLineCount(Handle, textId);
        }

        public bool TryGetTextLine(uint textId, uint lineIndex, out UITextLine line)
        {
            return Methods.DenOfIz_UI_GetTextLine(Handle, textId, lineIndex, out line);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }
            if (_ownsHandle)
            {
                Methods.DenOfIz_UI_Destroy(Handle);
            }
            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(UI));
            }
        }
    }
}
