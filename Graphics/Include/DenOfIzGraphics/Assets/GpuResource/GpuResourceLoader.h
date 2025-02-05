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
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <future>
#include <queue>

namespace DenOfIz
{
    using UpdateHandle = uint64_t;

    struct DZ_API UpdateSourceDataDesc
    {
        InteropArray<Byte> Data;
        uint64_t           Offset;
        uint64_t           NumBytes;
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
        TextureDesc        Desc;
        InteropString      Filename;
        InteropArray<Byte> Data;
    };

    struct DZ_API TextureUpdateDesc
    {
        ITextureResource      *Texture;
        uint32_t               MipLevel;
        uint32_t               ArrayLayer;
        TextureSubresourceDesc SubresourceDesc;
        InteropArray<Byte>     Data;
        ResourceUsage          CurrentState;
    };

    struct DZ_API BufferUpdateDesc
    {
        IBufferResource     *Buffer;
        uint64_t             DstOffset;
        UpdateSourceDataDesc SourceData;
        ResourceUsage        CurrentUsage;
    };

    struct DZ_API FlushUpdatesDesc
    {
        InteropArray<ISemaphore *> WaitSemaphores;
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
        void            *Map( );
        void             Unmap( );
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
        explicit GpuResourceLoader( const GpuResourceLoaderDesc &desc );
        virtual ~GpuResourceLoader( ) = default;

        virtual IBufferResource  *LoadResource( const BufferLoadDesc *BufferDesc, UpdateHandle *Handle = nullptr );
        virtual ITextureResource *LoadResource( const TextureLoadDesc *TextureDesc, UpdateHandle *Handle = nullptr );

        virtual void               QueueResourceUpdate( const BufferUpdateDesc *UpdateDesc, UpdateHandle *Handle = nullptr );
        virtual void               QueueResourceUpdate( const TextureUpdateDesc *UpdateDesc, UpdateHandle *Handle = nullptr );
        virtual FlushUpdatesResult FlushResourceUpdates( const FlushUpdatesDesc *Desc );

        virtual bool IsUpdateComplete( const UpdateHandle *Handle );
        virtual void WaitForUpdate( const UpdateHandle *Handle );
        virtual int  PendingUpdates( ) const;
        virtual void WaitForAllUpdates( );

        virtual void WaitForCopyQueue( );

        virtual UpdateHandle GetLastCompletedHandle( ) const;
        virtual UpdateHandle GetLastSubmittedHandle( ) const;
        virtual bool         IsHandleSubmitted( const UpdateHandle *Handle ) const;
        virtual void         WaitForHandleSubmission( const UpdateHandle *Handle );
        virtual ISemaphore  *GetLastSubmittedSemaphore( uint32_t NodeIndex ) const;
    };
} // namespace DenOfIz
