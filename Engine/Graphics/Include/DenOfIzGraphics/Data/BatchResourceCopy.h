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
#include "DenOfIzCore/Utilities.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
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

    class BatchResourceCopy
    {
    private:
        ILogicalDevice *m_device;

        std::unique_ptr<ICommandListPool> m_commandListPool;
        ICommandList                     *m_copyCommandList;

        std::unique_ptr<IFence>                       m_executeFence;
        std::mutex                                    m_resourceCleanLock;
        std::vector<std::unique_ptr<IBufferResource>> m_resourcesToClean;
        std::vector<Byte *>                           m_freeTextures;
        std::future<void>                             m_cleanResourcesFuture;

    public:
        explicit BatchResourceCopy( ILogicalDevice *device );
        ~        BatchResourceCopy( );

        void                              Begin( ) const;
        void                              CopyToGPUBuffer( const CopyToGpuBufferDesc &copyDesc );
        void                              CopyBufferRegion( const CopyBufferRegionDesc &copyDesc ) const;
        void                              CopyTextureRegion( const CopyTextureRegionDesc &copyDesc ) const;
        void                              CopyDataToTexture( const CopyDataToTextureDesc &copyDesc );
        std::unique_ptr<ITextureResource> CreateAndLoadTexture( const std::string &resourceName, const std::string &file );
        void                              LoadTexture( const LoadTextureDesc &loadDesc );
        void                              End( ISemaphore *notify );
        void                              CleanResources( );

    private:
        void     LoadTextureInternal( const Texture &texture, ITextureResource *dstTexture );
        void     CopyTextureToMemoryAligned( const Texture &texture, const MipData &mipData, Byte *dst ) const;
        uint32_t GetSubresourceAlignment( uint32_t bitSize ) const;
    };
} // namespace DenOfIz
