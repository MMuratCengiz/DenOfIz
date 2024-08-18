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
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include <memory>

namespace DenOfIz
{
    struct VertexIndexBufferPairHolder
    {
        std::unique_ptr<IBufferResource> VertexBuffer;
        std::unique_ptr<IBufferResource> IndexBuffer;

        void Into( std::unique_ptr<IBufferResource> &vertexBuffer, std::unique_ptr<IBufferResource> &indexBuffer )
        {
            vertexBuffer = std::move( VertexBuffer );
            indexBuffer  = std::move( IndexBuffer );
        }
    };

    struct UniformBufferHolder
    {
        std::unique_ptr<IBufferResource> Buffer;

        void Into( std::unique_ptr<IBufferResource> &buffer )
        {
            buffer = std::move( Buffer );
        }
    };

    struct SamplerHolder
    {
        std::unique_ptr<ISampler> Sampler;

        void Into( std::unique_ptr<ISampler> &sampler )
        {
            sampler = std::move( Sampler );
        }
    };

    struct TextureHolder
    {
        std::unique_ptr<ITextureResource> Texture;

        void Into( std::unique_ptr<ITextureResource> &texture )
        {
            texture = std::move( Texture );
        }
    };
    /// \brief Wrapper class for BatchResourceCopy which also provides helper functions for creating resources.
    /// \details Synchronization between direct queue must be done.
    /// \note This class transitions resources to ShaderResource state after copying data to GPU.
    class BatchResourceCopyHelper
    {
        ILogicalDevice                   *m_device;
        BatchResourceCopy                *m_batchCopy;
        std::unique_ptr<ICommandListPool> m_syncCommandPool;
        ICommandList                     *m_syncCommandList;
        std::unique_ptr<ISemaphore>       m_batchCopyWait;
        std::unique_ptr<IFence>           m_syncWait;

    public:
        BatchResourceCopyHelper( ILogicalDevice *device, BatchResourceCopy *batchCopy );

        void                                      Begin( ) const;
        [[nodiscard]] UniformBufferHolder         CreateUniformBuffer( const void *data, uint32_t numBytes ) const;
        [[nodiscard]] VertexIndexBufferPairHolder CreateGeometryBuffers( const GeometryData &GeometryData ) const;
        [[nodiscard]] SamplerHolder               CreateSampler( const SamplerDesc &desc ) const;
        [[nodiscard]] TextureHolder               CreateTexture( const std::string &path ) const;
        void                                      Submit( ) const;

    private:
        static std::string NextId( const std::string &prefix );
    };
} // namespace DenOfIz
