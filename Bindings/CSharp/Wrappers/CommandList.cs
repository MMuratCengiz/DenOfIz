using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class CommandList : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(CommandList wrapper) => wrapper?.Handle ?? 0;

        internal CommandList(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        internal void Invalidate()
        {
            Handle = 0;
        }

        public void Begin()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_Begin(Handle);
        }

        public void BeginRendering(in RenderingDesc renderingDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BeginRendering(Handle, in renderingDesc);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void BeginRendering(
            in RenderingAttachmentDesc rtAttachment,
            in RenderingAttachmentDesc depthAttachment,
            ViewMask viewMask = default,
            uint numLayers = 1)
        {
            ThrowIfDisposed();
            fixed (RenderingAttachmentDesc* ptr = &rtAttachment)
            {
                RenderingDesc desc = default;
                desc.RTAttachments.Elements = (IntPtr)ptr;
                desc.RTAttachments.NumElements = 1;
                desc.DepthAttachment = depthAttachment;
                desc.ViewMask = viewMask;
                desc.NumLayers = numLayers;
                Methods.DenOfIz_CommandList_BeginRendering(Handle, in desc);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public unsafe void BeginRendering(
            ReadOnlySpan<RenderingAttachmentDesc> rtAttachments,
            in RenderingAttachmentDesc depthAttachment,
            ViewMask viewMask = default,
            uint numLayers = 1)
        {
            ThrowIfDisposed();
            fixed (RenderingAttachmentDesc* ptr = rtAttachments)
            {
                RenderingDesc desc = default;
                desc.RTAttachments.Elements = (IntPtr)ptr;
                desc.RTAttachments.NumElements = (uint)rtAttachments.Length;
                desc.DepthAttachment = depthAttachment;
                desc.ViewMask = viewMask;
                desc.NumLayers = numLayers;
                Methods.DenOfIz_CommandList_BeginRendering(Handle, in desc);
            }
        }

        public void EndRendering()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_EndRendering(Handle);
        }

        public void End()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_End(Handle);
        }

        public void BindPipeline(Pipeline? pipeline)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BindPipeline(Handle, pipeline);
        }

        public void BindVertexBuffer(Buffer? buffer, ulong offset, uint stride, uint slot)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BindVertexBuffer(Handle, buffer, offset, stride, slot);
        }

        public void BindIndexBuffer(Buffer? buffer, IndexType indexType, ulong offset)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BindIndexBuffer(Handle, buffer, indexType, offset);
        }

        public void BindViewport(float x, float y, float width, float height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BindViewport(Handle, x, y, width, height);
        }

        public void BindScissorRect(float x, float y, float width, float height)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BindScissorRect(Handle, x, y, width, height);
        }

        public void BindGroup(BindGroup? bindGroup)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BindGroup(Handle, bindGroup);
        }

        public void SetRootConstants(uint binding, in ByteArray data)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_SetRootConstants(Handle, binding, in data);
        }

        public unsafe void SetRootConstants(uint binding, ReadOnlySpan<byte> data)
        {
            ThrowIfDisposed();
            fixed (byte* ptr = data)
            {
                ByteArray byteArray = default;
                byteArray.Elements = (IntPtr)ptr;
                byteArray.NumElements = (uint)data.Length;
                Methods.DenOfIz_CommandList_SetRootConstants(Handle, binding, in byteArray);
            }
        }

        public unsafe void SetRootConstants<T>(uint binding, in T data) where T : unmanaged
        {
            ThrowIfDisposed();
            fixed (T* ptr = &data)
            {
                ByteArray byteArray = default;
                byteArray.Elements = (IntPtr)ptr;
                byteArray.NumElements = (uint)sizeof(T);
                Methods.DenOfIz_CommandList_SetRootConstants(Handle, binding, in byteArray);
            }
        }

        public void PipelineBarrier(in PipelineBarrierDesc barrier)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_PipelineBarrier(Handle, in barrier);
        }

        public void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_DrawIndexed(Handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        public void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_Draw(Handle, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        public void CopyBufferRegion(in CopyBufferRegionDesc copyBufferRegionDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_CopyBufferRegion(Handle, in copyBufferRegionDesc);
        }

        public void CopyTextureRegion(in CopyTextureRegionDesc copyTextureRegionDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_CopyTextureRegion(Handle, in copyTextureRegionDesc);
        }

        public void CopyBufferToTexture(in CopyBufferToTextureDesc copyBufferToTexture)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_CopyBufferToTexture(Handle, in copyBufferToTexture);
        }

        public void CopyTextureToBuffer(in CopyTextureToBufferDesc copyTextureToBuffer)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_CopyTextureToBuffer(Handle, in copyTextureToBuffer);
        }

        public void UpdateTopLevelAS(in UpdateTopLevelASDesc updateDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_UpdateTopLevelAS(Handle, in updateDesc);
        }

        public void BuildTopLevelAS(in BuildTopLevelASDesc buildTopLevelASDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BuildTopLevelAS(Handle, in buildTopLevelASDesc);
        }

        public void BuildBottomLevelAS(in BuildBottomLevelASDesc buildBottomLevelASDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BuildBottomLevelAS(Handle, in buildBottomLevelASDesc);
        }

        public void DispatchRays(in DispatchRaysDesc dispatchRaysDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_DispatchRays(Handle, in dispatchRaysDesc);
        }

        public void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_Dispatch(Handle, groupCountX, groupCountY, groupCountZ);
        }

        public void DispatchMesh(uint groupCountX, uint groupCountY, uint groupCountZ)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_DispatchMesh(Handle, groupCountX, groupCountY, groupCountZ);
        }

        public void DrawIndirect(Buffer? buffer, ulong offset, uint drawCount, uint stride)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_DrawIndirect(Handle, buffer, offset, drawCount, stride);
        }

        public void DrawIndexedIndirect(Buffer? buffer, ulong offset, uint drawCount, uint stride)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_DrawIndexedIndirect(Handle, buffer, offset, drawCount, stride);
        }

        public void DispatchIndirect(Buffer? buffer, ulong offset)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_DispatchIndirect(Handle, buffer, offset);
        }

        public void BeginDebugMarker(float r, float g, float b, StringView name)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BeginDebugMarker(Handle, r, g, b, name);
        }

        public void EndDebugMarker()
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_EndDebugMarker(Handle);
        }

        public void InsertDebugMarker(float r, float g, float b, StringView name)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_InsertDebugMarker(Handle, r, g, b, name);
        }

        public void BeginQuery(QueryPool? queryPool, in QueryDesc queryDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_BeginQuery(Handle, queryPool, in queryDesc);
        }

        public void EndQuery(QueryPool? queryPool, in QueryDesc queryDesc)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_EndQuery(Handle, queryPool, in queryDesc);
        }

        public void ResolveQuery(QueryPool? queryPool, uint startQuery, uint queryCount)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_ResolveQuery(Handle, queryPool, startQuery, queryCount);
        }

        public void ResetQuery(QueryPool? queryPool, uint startQuery, uint queryCount)
        {
            ThrowIfDisposed();
            Methods.DenOfIz_CommandList_ResetQuery(Handle, queryPool, startQuery, queryCount);
        }

        public QueueType GetQueueType()
        {
            ThrowIfDisposed();
            QueueType outQueueType = default;
            Methods.DenOfIz_CommandList_GetQueueType(Handle, out outQueueType);
            return outQueueType;
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_CommandList_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(CommandList));
            }
        }
    }
}
