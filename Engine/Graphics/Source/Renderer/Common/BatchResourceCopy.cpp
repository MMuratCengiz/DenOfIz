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

#include <DenOfIzGraphics/Renderer/Common/BatchResourceCopy.h>

using namespace DenOfIz;

BatchResourceCopy::BatchResourceCopy(ILogicalDevice *device) : m_device(device)
{
    m_commandListPool = m_device->CreateCommandListPool({ QueueType::Copy });
    DZ_ASSERTM(m_commandListPool->GetCommandLists().size() > 0, "Command list pool did not produce any command lists.");

    m_copyCommandList = m_commandListPool->GetCommandLists()[ 0 ];
    m_executeFence    = m_device->CreateFence();
}

BatchResourceCopy::~BatchResourceCopy()
{
    m_executeFence.reset();
    m_commandListPool.reset();
}

void BatchResourceCopy::Begin()
{
    m_copyCommandList->Begin();
}

void BatchResourceCopy::CopyToGPUBuffer(const CopyToGpuBufferDesc &copyInfo)
{
    BufferDesc stagingBufferDesc{};
    stagingBufferDesc.HeapType     = HeapType::CPU_GPU;
    stagingBufferDesc.InitialState = ResourceState::CopySrc;
    stagingBufferDesc.NumBytes     = copyInfo.NumBytes;

    auto stagingBuffer = m_device->CreateBufferResource("StagingBuffer", stagingBufferDesc);

    stagingBuffer->MapMemory();
    stagingBuffer->CopyData(copyInfo.Data, copyInfo.NumBytes);
    stagingBuffer->UnmapMemory();

    CopyBufferRegionDesc copyBufferRegionDesc{};
    copyBufferRegionDesc.DstBuffer = copyInfo.DstBuffer;
    copyBufferRegionDesc.SrcBuffer = stagingBuffer.get();
    copyBufferRegionDesc.NumBytes  = copyInfo.NumBytes;

    CopyBufferRegion(copyBufferRegionDesc);

    std::lock_guard<std::mutex> lock(m_resourceCleanLock);
    m_resourcesToClean.push_back(std::move(stagingBuffer));
}

void BatchResourceCopy::CopyBufferRegion(const CopyBufferRegionDesc &copyInfo)
{
    m_copyCommandList->CopyBufferRegion(copyInfo);
}

void BatchResourceCopy::CopyTextureRegion(const CopyTextureRegionDesc &copyInfo)
{
    m_copyCommandList->CopyTextureRegion(copyInfo);
}

void BatchResourceCopy::End(ISemaphore *notify)
{
    ExecuteDesc desc{};

    desc.Notify = m_executeFence.get();
    if ( notify )
    {
        desc.NotifySemaphores.push_back(notify);
    }

    m_copyCommandList->Execute(desc);
    m_cleanResourcesFuture = std::async(std::launch::async, [ this ]() { CleanResources(); });
}

void BatchResourceCopy::CleanResources()
{
    std::lock_guard<std::mutex> lock(m_resourceCleanLock);
    m_executeFence->Wait();
    m_resourcesToClean.clear();
}
