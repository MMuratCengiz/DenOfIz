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

#include <array>
#include "IBufferResource.h"
#include "IFence.h"
#include "ILock.h"
#include "IPipeline.h"
#include "IRayTracingAccelerationStructure.h"
#include "IResourceBindGroup.h"
#include "ISemaphore.h"
#include "ISwapChain.h"
#include "ITextureResource.h"
#include "PipelineBarrierDesc.h"

namespace DenOfIz
{

    struct RenderingAttachmentDesc
    {
        LoadOp  LoadOp  = LoadOp::Clear;
        StoreOp StoreOp = StoreOp::Store;

        ITextureResource *Resource = nullptr;

        std::array<float, 4> ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        std::array<float, 2> ClearDepth{ 1.0f, 0.0f };
    };

    struct RenderingDesc
    {
        std::vector<RenderingAttachmentDesc> RTAttachments;
        RenderingAttachmentDesc              DepthAttachment;
        RenderingAttachmentDesc              StencilAttachment;

        float    RenderAreaWidth   = 0.0f;
        float    RenderAreaHeight  = 0.0f;
        float    RenderAreaOffsetX = 0.0f;
        float    RenderAreaOffsetY = 0.0f;
        uint32_t LayerCount        = 1;
    };

    struct CopyBufferRegionDesc
    {
        IBufferResource *DstBuffer = nullptr;
        uint64_t         DstOffset = 0;
        IBufferResource *SrcBuffer = nullptr;
        uint64_t         SrcOffset = 0;
        uint64_t         NumBytes  = 0;
    };

    struct CopyTextureRegionDesc
    {
        ITextureResource *SrcTexture    = nullptr;
        ITextureResource *DstTexture    = nullptr;
        uint32_t          SrcX          = 0;
        uint32_t          SrcY          = 0;
        uint32_t          SrcZ          = 0;
        uint32_t          DstX          = 0;
        uint32_t          DstY          = 0;
        uint32_t          DstZ          = 0;
        uint32_t          Width         = 0;
        uint32_t          Height        = 0;
        uint32_t          Depth         = 0;
        uint32_t          SrcMipLevel   = 0;
        uint32_t          DstMipLevel   = 0;
        uint32_t          SrcArrayLayer = 0;
        uint32_t          DstArrayLayer = 0;
    };

    struct CopyBufferToTextureDesc
    {
        ITextureResource *DstTexture = nullptr;
        IBufferResource  *SrcBuffer  = nullptr;
        size_t            SrcOffset  = 0;
        Format            Format     = Format::R8G8B8A8Unorm;
        uint32_t          MipLevel   = 0;
        uint32_t          ArrayLayer = 0;
        // Information below is optional, 0 tries to calculate it automatically, sometimes it may be needed to be set manually
        uint32_t RowPitch   = 0;
        uint32_t NumRows = 0;
    };

    struct CopyTextureToBufferDesc
    {
        IBufferResource  *DstBuffer  = nullptr;
        ITextureResource *SrcTexture = nullptr;
        uint32_t          DstOffset  = 0;
        uint32_t          SrcX       = 0;
        uint32_t          SrcY       = 0;
        uint32_t          SrcZ       = 0;
        Format            Format     = Format::R8G8B8A8Unorm;
        uint32_t          MipLevel   = 0;
        uint32_t          ArrayLayer = 0;
    };

    struct ExecuteDesc
    {
        IFence                   *Notify           = nullptr;
        std::vector<ISemaphore *> WaitOnSemaphores = { };
        std::vector<ISemaphore *> NotifySemaphores = { };
    };

    struct CommandListDesc
    {
        QueueType QueueType = QueueType::Graphics;
    };

    class ICommandList
    {
    public:
        virtual ~ICommandList( ) = default;

        virtual void Begin( )                                                                                                                                   = 0;
        virtual void BeginRendering( const RenderingDesc &renderingInfo )                                                                                       = 0;
        virtual void EndRendering( )                                                                                                                            = 0;
        virtual void Execute( const ExecuteDesc &submitInfo )                                                                                                   = 0;
        virtual void Present( ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks )                                               = 0;
        virtual void BindPipeline( IPipeline *pipeline )                                                                                                        = 0;
        virtual void BindVertexBuffer( IBufferResource *buffer )                                                                                                = 0;
        virtual void BindIndexBuffer( IBufferResource *buffer, const IndexType &indexType )                                                                     = 0;
        virtual void BindViewport( float x, float y, float width, float height )                                                                                = 0;
        virtual void BindScissorRect( float x, float y, float width, float height )                                                                             = 0;
        virtual void BindResourceGroup( IResourceBindGroup *bindGroup )                                                                                         = 0;
        virtual void SetDepthBias( float constantFactor, float clamp, float slopeFactor )                                                                       = 0;
        virtual void PipelineBarrier( const PipelineBarrierDesc &barrier )                                                                                      = 0;
        virtual void DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0 ) = 0;
        virtual void Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0 )                                 = 0;
        // List of copy commands
        virtual void CopyBufferRegion( const CopyBufferRegionDesc &copyBufferRegionInfo )      = 0;
        virtual void CopyTextureRegion( const CopyTextureRegionDesc &copyTextureRegionInfo )   = 0;
        virtual void CopyBufferToTexture( const CopyBufferToTextureDesc &copyBufferToTexture ) = 0;
        virtual void CopyTextureToBuffer( const CopyTextureToBufferDesc &copyTextureToBuffer ) = 0;
        // --
        virtual void Dispatch( uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ ) = 0;
    };

} // namespace DenOfIz
