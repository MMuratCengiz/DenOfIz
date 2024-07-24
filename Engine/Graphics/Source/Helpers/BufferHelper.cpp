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

#include <DenOfIzGraphics/Helpers/BufferHelper.h>
#include <atomic>

using namespace DenOfIz;

VertexIndexBufferPair BufferHelper::CreateGeometryBuffers(const GeometryBuffersDesc& desc)
{
    VertexIndexBufferPair result{};
    BatchResourceCopy *queue = desc.Queue;
    ILogicalDevice *device = desc.Device;
    const GeometryData geometryData = desc.GeometryData;

    BufferDesc vBufferDesc{};
    vBufferDesc.HeapType     = HeapType::GPU;
    vBufferDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
    vBufferDesc.InitialState = ResourceState::CopyDst;
    vBufferDesc.NumBytes     = geometryData.SizeOfVertices();
    result.VertexBuffer      = device->CreateBufferResource(NextId("Vertex"), vBufferDesc);

    BufferDesc iBufferDesc{};
    iBufferDesc.HeapType     = HeapType::GPU;
    iBufferDesc.Descriptor   = ResourceDescriptor::IndexBuffer;
    iBufferDesc.InitialState = ResourceState::CopyDst;
    iBufferDesc.NumBytes     = geometryData.SizeOfIndices();
    result.IndexBuffer       = device->CreateBufferResource(NextId("Index"), iBufferDesc);

    CopyToGpuBufferDesc vbCopyDesc{};
    vbCopyDesc.DstBuffer = result.VertexBuffer.get();
    vbCopyDesc.Data      = geometryData.Vertices.data();
    vbCopyDesc.NumBytes  = geometryData.SizeOfVertices();
    queue->CopyToGPUBuffer(vbCopyDesc);

    CopyToGpuBufferDesc ibCopyDesc{};
    ibCopyDesc.DstBuffer = result.IndexBuffer.get();
    ibCopyDesc.Data      = geometryData.Indices.data();
    ibCopyDesc.NumBytes  = geometryData.SizeOfIndices();
    queue->CopyToGPUBuffer(ibCopyDesc);

    return result;
}

std::string BufferHelper::NextId(const std::string &prefix)
{
    static std::atomic<unsigned int> idCounter(0);
    int                              next = idCounter.fetch_add(1, std::memory_order_relaxed);
    return prefix + "_BufferHelperResource#" + std::to_string(next);
}
