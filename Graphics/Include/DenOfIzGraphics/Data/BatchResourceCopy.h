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

#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Data/Geometry.h>
#include <DenOfIzGraphics/Renderer/Assets/AssetData.h>
#include <future>
#include "Texture.h"

namespace DenOfIz
{
    struct CopyToGpuBufferDesc
    {
        IBufferResource *DstBuffer;
        const void      *Data;
        size_t           NumBytes;
    };

    struct CopyDataToTextureDesc
    {
        ITextureResource *DstTexture;
        Byte             *Data;
        size_t            NumBytes;
        uint32_t          MipLevel;
        uint32_t          ArrayLayer;
        uint32_t          RowPitch;
        uint32_t          SlicePitch;
    };

    struct LoadTextureDesc
    {
        std::string       File;
        ITextureResource *DstTexture;
    };

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

    class BatchResourceCopy
    {
        ILogicalDevice *m_device;

        std::unique_ptr<ICommandListPool> m_commandListPool;
        ICommandList                     *m_copyCommandList;

        std::unique_ptr<IFence>                       m_executeFence;
        std::mutex                                    m_resourceCleanLock;
        std::vector<std::unique_ptr<IBufferResource>> m_resourcesToClean;
        std::vector<Byte *>                           m_freeTextures;
        std::future<void>                             m_cleanResourcesFuture;
        // Syncing
        std::unique_ptr<ICommandListPool> m_syncCommandPool;
        ICommandList                     *m_syncCommandList;
        std::unique_ptr<ISemaphore>       m_batchCopyWait;
        std::unique_ptr<IFence>           m_syncWait;
        bool                              m_issueBarriers;

    public:
        explicit BatchResourceCopy( ILogicalDevice *device, bool issueBarriers = true );
        ~BatchResourceCopy( );

        void                                      Begin( ) const;
        void                                      CopyToGPUBuffer( const CopyToGpuBufferDesc &copyDesc );
        void                                      CopyBufferRegion( const CopyBufferRegionDesc &copyDesc ) const;
        void                                      CopyTextureRegion( const CopyTextureRegionDesc &copyDesc ) const;
        void                                      CopyDataToTexture( const CopyDataToTextureDesc &copyDesc );
        std::unique_ptr<ITextureResource>         CreateAndLoadTexture( const std::string &file );
        void                                      LoadTexture( const LoadTextureDesc &loadDesc );
        [[nodiscard]] UniformBufferHolder         CreateAndStoreUniformBuffer( const void *data, uint32_t numBytes );
        [[nodiscard]] VertexIndexBufferPairHolder CreateAndStoreGeometryBuffers( const GeometryData &GeometryData );
        [[nodiscard]] std::unique_ptr<AssetData>  CreateGeometryAssetData( const GeometryData &GeometryData );
        [[nodiscard]] SamplerHolder               CreateAndStoreSampler( const SamplerDesc &desc ) const;
        [[nodiscard]] TextureHolder               CreateAndStoreTexture( const std::string &path );
        void                                      Submit( ISemaphore *notify = nullptr );

        /// <summary> A synchronized batch resource copy operation, ensures copying is finalized. </summary>
        static void SyncOp( ILogicalDevice *device, const std::function<void( BatchResourceCopy * )> &op );

    private:
        void                   CleanResources( );
        void                   LoadTextureInternal( const Texture &texture, ITextureResource *dstTexture );
        void                   CopyTextureToMemoryAligned( const Texture &texture, const MipData &mipData, Byte *dst ) const;
        [[nodiscard]] uint32_t GetSubresourceAlignment( uint32_t bitSize ) const;
        static std::string     NextId( const std::string &prefix );
    };
} // namespace DenOfIz
