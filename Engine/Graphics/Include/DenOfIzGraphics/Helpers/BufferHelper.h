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

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <memory>

namespace DenOfIz
{
    struct VertexIndexBufferPair
    {
        std::unique_ptr<IBufferResource> VertexBuffer;
        std::unique_ptr<IBufferResource> IndexBuffer;

        void Into(std::unique_ptr<IBufferResource>& vertexBuffer, std::unique_ptr<IBufferResource>& indexBuffer)
        {
            vertexBuffer = std::move(VertexBuffer);
            indexBuffer = std::move(IndexBuffer);
        }
    };

    struct GeometryBuffersDesc
    {
        BatchResourceCopy* Queue;
        ILogicalDevice* Device;
        const GeometryData& GeometryData;
    };

    class BufferHelper
    {
    public:
        BufferHelper() = delete;

        [[nodiscard]] static VertexIndexBufferPair CreateGeometryBuffers(const GeometryBuffersDesc& desc);

    private:
        static std::string NextId(const std::string &prefix);
    };
} // namespace DenOfIzGraphics
