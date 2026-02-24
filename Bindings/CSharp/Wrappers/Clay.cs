using System;
using System.Numerics;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class Clay : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;
        private Texture?[]? _cachedTextures;
        private Semaphore?[]? _cachedSemaphores;

        public static implicit operator ulong(Clay wrapper) => wrapper?.Handle ?? 0;

        public Clay(in ClayDesc desc)
        {
            Handle = Methods.DenOfIz_Clay_Create(in desc);
            _ownsHandle = true;
            _cachedTextures = new Texture?[desc.NumFrames];
            _cachedSemaphores = new Semaphore?[desc.NumFrames];
        }

        internal Clay(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public void SetViewportSize(float width, float height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_SetViewportSize(Handle, width, height);
        }

        public ClayDimensions GetViewportSize()
        {
            ThrowIfDisposed();
            ClayDimensions outSize = default;
            Methods.DenOfIz_Clay_GetViewportSize(Handle, out outSize);
            return outSize;
        }

        public void SetDpiScale(float dpiScale)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_SetDpiScale(Handle, dpiScale);
        }

        public void SetPointerState(Vector2 position, ClayPointerState state)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_SetPointerState(Handle, position, state);
        }

        public void UpdateScrollContainers(bool enableDragScrolling, Vector2 scrollDelta, float deltaTime)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_UpdateScrollContainers(Handle, enableDragScrolling, scrollDelta, deltaTime);
        }

        public void SetDebugModeEnabled(bool enabled)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_SetDebugModeEnabled(Handle, enabled);
        }

        public bool IsDebugModeEnabled()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_IsDebugModeEnabled(Handle);
        }

        public void BeginLayout()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_BeginLayout(Handle);
        }

        public void EndLayout(uint frameIndex, float deltaTime, CommandList? commandList)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_EndLayout(Handle, frameIndex, deltaTime, commandList);
        }

        public (Texture Texture, Semaphore Semaphore) EndAndRenderLayout(uint frameIndex, float deltaTime)
        {
            ThrowIfDisposed();
            ClayRenderResult outResult = default;
            Methods.DenOfIz_Clay_EndAndRenderLayout(Handle, frameIndex, deltaTime, out outResult);

            if (_cachedTextures != null && _cachedSemaphores != null)
            {
                _cachedTextures[frameIndex] ??= new Texture(outResult.Texture, ownsHandle: false);
                _cachedSemaphores[frameIndex] ??= new Semaphore(outResult.Semaphore, ownsHandle: false);
                return (_cachedTextures[frameIndex]!, _cachedSemaphores[frameIndex]!);
            }

            return (
                new Texture(outResult.Texture, ownsHandle: false),
                new Semaphore(outResult.Semaphore, ownsHandle: false)
            );
        }

        public void HandleEvent(in Event ev)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_HandleEvent(Handle, in ev);
        }

        public void OpenElement(in ClayElementDeclaration declaration)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_OpenElement(Handle, in declaration);
        }

        public void CloseElement()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_CloseElement(Handle);
        }

        public void Text(StringView text, in ClayTextDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_Text(Handle, text, in desc);
        }

        public void Text(string text, in ClayTextDesc desc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_Text(Handle, StringView.Intern(text), in desc);
        }

        public void Texture(Texture? texture, float width, float height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_Texture(Handle, texture, width, height);
        }

        public uint HashString(StringView str, uint index, uint baseId)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_HashString(Handle, str, index, baseId);
        }

        public uint HashString(string str, uint index, uint baseId)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_HashString(Handle, StringView.Intern(str), index, baseId);
        }

        public bool PointerOver(uint id)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_PointerOver(Handle, id);
        }

        public ClayBoundingBox GetElementBoundingBox(uint id)
        {
            ThrowIfDisposed();
            ClayBoundingBox outBox = default;
            Methods.DenOfIz_Clay_GetElementBoundingBox(Handle, id, out outBox);
            return outBox;
        }

        public ClayDimensions MeasureText(StringView text, ushort fontId, ushort fontSize)
        {
            ThrowIfDisposed();
            ClayDimensions outDimensions = default;
            Methods.DenOfIz_Clay_MeasureText(Handle, text, fontId, fontSize, out outDimensions);
            return outDimensions;
        }

        public ClayDimensions MeasureText(string text, ushort fontId, ushort fontSize)
        {
            ThrowIfDisposed();
            ClayDimensions outDimensions = default;
            Methods.DenOfIz_Clay_MeasureText(Handle, StringView.Intern(text), fontId, fontSize, out outDimensions);
            return outDimensions;
        }

        public void AddFont(ushort fontId, Font? font)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_AddFont(Handle, fontId, font);
        }

        public void RemoveFont(ushort fontId)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_RemoveFont(Handle, fontId);
        }

        public ClayScrollContainerData GetScrollContainerData(uint id)
        {
            ThrowIfDisposed();
            ClayScrollContainerData outData = default;
            Methods.DenOfIz_Clay_GetScrollContainerData(Handle, id, out outData);
            return outData;
        }

        public void SetScrollPosition(uint id, Vector2 position)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_Clay_SetScrollPosition(Handle, id, position);
        }

        public bool ElementClicked(uint id)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_ElementClicked(Handle, id);
        }

        public bool ElementPressed(uint id)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_ElementPressed(Handle, id);
        }

        public float GetCursorOffsetAtIndex(StringView text, uint charIndex, ushort fontId, ushort fontSize)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_GetCursorOffsetAtIndex(Handle, text, charIndex, fontId, fontSize);
        }

        public float GetCursorOffsetAtIndex(string text, uint charIndex, ushort fontId, ushort fontSize)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_GetCursorOffsetAtIndex(Handle, StringView.Intern(text), charIndex, fontId, fontSize);
        }

        public uint GetCharIndexAtOffset(StringView text, float pixelOffset, ushort fontId, ushort fontSize)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_GetCharIndexAtOffset(Handle, text, pixelOffset, fontId, fontSize);
        }

        public uint GetCharIndexAtOffset(string text, float pixelOffset, ushort fontId, ushort fontSize)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_GetCharIndexAtOffset(Handle, StringView.Intern(text), pixelOffset, fontId, fontSize);
        }

        public float GetDpiScale()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_GetDpiScale(Handle);
        }

        public float PointsToPixels(float points)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_PointsToPixels(Handle, points);
        }

        public float PixelsToPoints(float pixels)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_Clay_PixelsToPoints(Handle, pixels);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_Clay_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(Clay));
            }
        }
    }
}
