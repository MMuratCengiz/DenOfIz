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
#include "IDescriptorTable.h"
#include "IFence.h"
#include "ILock.h"
#include "IPipeline.h"
#include "IResource.h"
#include "ISemaphore.h"
#include "ISwapChain.h"
#include "PipelineBarrier.h"

namespace DenOfIz
{

    struct RenderingAttachmentInfo
    {
        ImageLayout Layout = ImageLayout::Undefined;
        LoadOp LoadOp = LoadOp::Clear;
        StoreOp StoreOp = StoreOp::Store;

        IImageResource *Resource = nullptr;

        std::array<float, 4> ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        std::array<float, 2> ClearDepth{ 1.0f, 0.0f };
    };

    struct RenderingInfo
    {
        std::vector<RenderingAttachmentInfo> RTAttachments;
        RenderingAttachmentInfo DepthAttachment;
        RenderingAttachmentInfo StencilAttachment;

        float RenderAreaWidth = 0.0f;
        float RenderAreaHeight = 0.0f;
        float RenderAreaOffsetX = 0.0f;
        float RenderAreaOffsetY = 0.0f;
        uint32_t LayerCount = 1;
    };

    struct ExecuteInfo
    {
        IFence *Notify = nullptr;
        std::vector<ISemaphore *> WaitOnLocks = {};
        std::vector<ISemaphore *> SignalLocks = {};
    };

    struct CommandListCreateInfo
    {
        QueueType QueueType = QueueType::Graphics;
    };

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;

        virtual void Begin() = 0;
        virtual void BeginRendering(const RenderingInfo &renderingInfo) = 0;
        virtual void EndRendering() = 0;
        virtual void Execute(const ExecuteInfo &submitInfo) = 0;
        virtual void Present(ISwapChain *swapChain, uint32_t imageIndex, std::vector<ISemaphore *> waitOnLocks) = 0;
        virtual void BindPipeline(IPipeline *pipeline) = 0;
        virtual void BindVertexBuffer(IBufferResource *buffer) = 0;
        virtual void BindIndexBuffer(IBufferResource *buffer, const IndexType &indexType) = 0;
        virtual void BindViewport(float x, float y, float width, float height) = 0;
        virtual void BindScissorRect(float x, float y, float width, float height) = 0;
        virtual void BindDescriptorTable(IDescriptorTable *table) = 0;
        virtual void BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void *data) = 0;
        virtual void BindBufferResource(IBufferResource *resource) = 0;
        virtual void BindImageResource(IImageResource *resource) = 0;
        virtual void SetDepthBias(float constantFactor, float clamp, float slopeFactor) = 0;
        virtual void SetPipelineBarrier(const PipelineBarrier &barrier) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
        virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void TransitionImageLayout(IImageResource *image, ImageLayout oldLayout, ImageLayout newLayout) = 0;
    };

} // namespace DenOfIz
