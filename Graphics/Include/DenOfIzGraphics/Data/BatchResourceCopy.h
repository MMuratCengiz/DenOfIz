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
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <future>
#include "Texture.h"

namespace DenOfIz
{
    struct DZ_API CopyToGpuBufferDesc
    {
        IBufferResource   *DstBuffer;
        InteropArray<Byte> Data;
    };

    struct DZ_API CopyDataToTextureDesc
    {
        ITextureResource  *DstTexture;
        InteropArray<Byte> Data;
        uint32_t           MipLevel;
        uint32_t           ArrayLayer;
        uint32_t           RowPitch;
        uint32_t           SlicePitch;
    };

    struct DZ_API LoadTextureDesc
    {
        InteropString     File;
        ITextureResource *DstTexture;
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
        DZ_API void                           LoadTexture( const LoadTextureDesc &loadDesc );
        [[nodiscard]] DZ_API IBufferResource *CreateUniformBuffer( const InteropArray<Byte> &, uint32_t numBytes );
        [[nodiscard]] DZ_API IBufferResource *CreateGeometryVertexBuffer( const GeometryData &geometryData );
        [[nodiscard]] DZ_API IBufferResource *CreateGeometryIndexBuffer( const GeometryData &geometryData );
        DZ_API void                           Submit( ISemaphore *notify = nullptr );

    private:
        void                     CleanResources( );
        void                     LoadTextureInternal( const Texture &texture, ITextureResource *dstTexture );
        void                     CopyTextureToMemoryAligned( const Texture &texture, const MipData &mipData, Byte *dst ) const;
        [[nodiscard]] uint32_t   GetSubresourceAlignment( uint32_t bitSize ) const;
        static std::string       NextId( const std::string &prefix );
    };
} // namespace DenOfIz
