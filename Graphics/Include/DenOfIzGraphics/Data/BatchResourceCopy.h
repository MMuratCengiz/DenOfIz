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

#include <future>
#include "Texture.h"

#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Data/Geometry.h"

namespace DenOfIz
{
    struct DZ_API CopyToGpuBufferDesc
    {
        IBufferResource *DstBuffer;
        uint64_t         DstBufferOffset = 0;
        ByteArrayView    Data;
    };

    struct DZ_API CopyDataToTextureDesc
    {
        ITextureResource *DstTexture;
        ByteArrayView     Data;

        bool     AutoAlign  = false;
        uint32_t Width      = 0; // Only required when AutoAlign = true
        uint32_t Height     = 0; // Only required when AutoAlign = true
        uint32_t ArrayLayer = 0;
        uint32_t MipLevel   = 0;
    };

    struct DZ_API LoadTextureDesc
    {
        InteropString     File;
        ITextureResource *DstTexture;
    };

    struct DZ_API LoadAssetTextureDesc // Rename to LoadTextureAssetDesc
    {
        TextureAssetReader *Reader;
        ITextureResource   *DstTexture;
    };

    struct DZ_API LoadAssetStreamToBufferDesc
    {
        AssetDataStream  Stream;
        IBufferResource *DstBuffer{ };
        uint64_t         DstBufferOffset{ };
        BinaryReader    *Reader;
    };

    struct DZ_API CreateAssetTextureDesc
    {
        TextureAssetReader *Reader;
        uint32_t            AdditionalDescriptors;
        uint32_t            AdditionalUsages;
        InteropString       DebugName;
    };

    /// <code>
    /// { // Scope batch resource copy to make sure it waits for the copy to finish, will clean up resources too
    ///     BatchResourceCopy batchResourceCopy( logicalDevice );
    ///     batchResourceCopy.Begin( );
    ///     m_sphere = std::make_unique<SphereAsset>( m_logicalDevice, &batchResourceCopy );
    ///     batchResourceCopy.Submit( );
    /// }
    /// </code>
    class BatchResourceCopy : public NonCopyable
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

        std::unique_ptr<ICommandQueue> m_copyQueue;
        std::unique_ptr<ICommandQueue> m_syncQueue;

    public:
        DZ_API explicit BatchResourceCopy( ILogicalDevice *device, bool issueBarriers = true );
        DZ_API ~BatchResourceCopy( );

        DZ_API void                           Begin( ) const;
        DZ_API void                           CopyToGPUBuffer( const CopyToGpuBufferDesc &copyDesc );
        DZ_API void                           CopyBufferRegion( const CopyBufferRegionDesc &copyDesc ) const;
        DZ_API void                           CopyTextureRegion( const CopyTextureRegionDesc &copyDesc ) const;
        DZ_API void                           CopyDataToTexture( const CopyDataToTextureDesc &copyDesc );
        DZ_API ITextureResource              *CreateAndLoadTexture( const InteropString &file );
        DZ_API ITextureResource              *CreateAndLoadAssetTexture( const CreateAssetTextureDesc &loadDesc );
        DZ_API void                           LoadTexture( const LoadTextureDesc &loadDesc );
        DZ_API void                           LoadAssetTexture( const LoadAssetTextureDesc &loadDesc );
        DZ_API void                           LoadAssetStreamToBuffer( const LoadAssetStreamToBufferDesc &loadDesc );
        [[nodiscard]] DZ_API IBufferResource *CreateUniformBuffer( const ByteArrayView &, uint32_t numBytes );
        [[nodiscard]] DZ_API IBufferResource *CreateGeometryVertexBuffer( const GeometryData &geometryData );
        [[nodiscard]] DZ_API IBufferResource *CreateGeometryIndexBuffer( const GeometryData &geometryData );
        DZ_API void                           Submit( ISemaphore *notify = nullptr );

    private:
        void                   CleanResources( );
        void                   LoadTextureInternal( const Texture &texture, ITextureResource *dstTexture );
        void                   AlignDataForTexture( const Byte *src, uint32_t width, uint32_t height, uint32_t bitsize, Byte *dst ) const;
        void                   CopyTextureToMemoryAligned( const Texture &texture, const TextureMip &mipData, Byte *dst ) const;
        [[nodiscard]] uint32_t GetSubresourceAlignment( uint32_t bitSize ) const;
        static std::string     NextId( const std::string &prefix );
    };
} // namespace DenOfIz
