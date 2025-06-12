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
#include <queue>
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    struct DZ_API UpdateHandle
    {
        uint64_t Value;
    };

    struct DZ_API UpdateSourceDataDesc
    {
        ByteArrayView Data;
        uint64_t      Offset;
        uint64_t      NumBytes;
    };

    struct DZ_API TextureSubresourceDesc
    {
        uint32_t DstRowStride;
        uint32_t RowCount;
        uint32_t SrcRowStride;
        uint32_t DstSliceStride;
        uint32_t SrcSliceStride;
    };

    struct DZ_API GpuResourceLoaderDesc
    {
        ILogicalDevice *Device;
        uint64_t        StagingBufferNumBytes;
        uint32_t        NumStagingBuffers;
    };

    struct DZ_API BufferLoadDesc
    {
        BufferDesc           Desc;
        UpdateSourceDataDesc SourceData;
    };

    struct DZ_API TextureLoadDesc
    {
        TextureDesc   Desc;
        InteropString Filename;
        ByteArrayView Data;
    };

    struct DZ_API TextureUpdateDesc
    {
        ITextureResource      *Texture;
        uint32_t               MipLevel;
        uint32_t               ArrayLayer;
        TextureSubresourceDesc SubresourceDesc;
        ByteArrayView          Data;
        uint32_t               CurrentState;
    };

    struct DZ_API BufferUpdateDesc
    {
        IBufferResource     *Buffer;
        uint64_t             DstOffset;
        UpdateSourceDataDesc SourceData;
        uint32_t             CurrentUsage;
    };

    struct DZ_API FlushUpdatesDesc
    {
        ISemaphoreArray WaitSemaphores;
    };

    struct DZ_API FlushUpdatesResult
    {
        IFence     *Fence;
        ISemaphore *Semaphore;
    };

    class StagingBuffer : public NonCopyable
    {
        std::unique_ptr<IBufferResource> m_buffer;
        uint64_t                         m_currentOffset;
        uint64_t                         m_totalNumBytes;
        UpdateHandle                     m_lastHandle;

    public:
        StagingBuffer( ILogicalDevice *device, uint64_t numBytes );
        ~StagingBuffer( ) = default;
        bool             CanFit( uint64_t size ) const;
        void            *Map( ) const;
        void             Unmap( ) const;
        void             Reset( );
        IBufferResource *GetBuffer( ) const;
        uint64_t         GetOffset( ) const;
        UpdateHandle     GetLastHandle( ) const;
        void             Advance( uint64_t size, UpdateHandle handle );
    };

    class GpuResourceLoader
    {
        ILogicalDevice                             *m_device;
        std::vector<std::unique_ptr<StagingBuffer>> m_stagingBuffers;
        std::unique_ptr<ICommandListPool>           m_commandPool;
        std::vector<ICommandList *>                 m_commandLists;
        std::unique_ptr<IFence>                     m_executeFence;
        std::unique_ptr<ISemaphore>                 m_copyCompleteSemaphore;
        UpdateHandle                                m_nextHandle;

        struct UpdateOperation
        {
            UpdateHandle Handle;
            IFence      *Fence;
        };
        std::queue<UpdateOperation> m_pendingOperations;
        std::mutex                  m_operationMutex;

    public:
        DZ_API explicit GpuResourceLoader( const GpuResourceLoaderDesc &desc );
        DZ_API ~GpuResourceLoader( ) = default;

        DZ_API IBufferResource  *LoadResource( const BufferLoadDesc *BufferDesc, UpdateHandle *Handle = nullptr );
        DZ_API ITextureResource *LoadResource( const TextureLoadDesc *TextureDesc, UpdateHandle *Handle = nullptr );

        DZ_API void               QueueResourceUpdate( const BufferUpdateDesc *UpdateDesc, UpdateHandle *Handle = nullptr );
        DZ_API void               QueueResourceUpdate( const TextureUpdateDesc *UpdateDesc, UpdateHandle *Handle = nullptr );
        DZ_API FlushUpdatesResult FlushResourceUpdates( const FlushUpdatesDesc *Desc );

        DZ_API bool IsUpdateComplete( const UpdateHandle *Handle );
        DZ_API void WaitForUpdate( const UpdateHandle *Handle );
        DZ_API int  PendingUpdates( ) const;
        DZ_API void WaitForAllUpdates( );
        DZ_API void WaitForCopyQueue( );

        DZ_API UpdateHandle GetLastCompletedHandle( ) const;
        DZ_API UpdateHandle GetLastSubmittedHandle( ) const;
        DZ_API bool         IsHandleSubmitted( const UpdateHandle *Handle ) const;
        DZ_API void         WaitForHandleSubmission( const UpdateHandle *Handle );
        DZ_API ISemaphore  *GetLastSubmittedSemaphore( uint32_t NodeIndex ) const;
    };
} // namespace DenOfIz
