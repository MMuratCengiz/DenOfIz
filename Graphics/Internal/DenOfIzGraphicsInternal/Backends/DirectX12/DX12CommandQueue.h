/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h>
#include "DX12CommandList.h"
#include "DX12Context.h"

namespace DenOfIz
{
    class DX12CommandQueue final : public ICommandQueue
    {
        DX12Context                     *m_context;
        DenOfIz_CommandQueueDesc         m_desc;
        wil::com_ptr<ID3D12CommandQueue> m_commandQueue;
        wil::com_ptr<ID3D12Fence1>       m_fence;
        std::atomic<uint64_t>            m_fenceValue{ 0 };

    public:
        DX12CommandQueue( DX12Context *context, const DenOfIz_CommandQueueDesc &desc );
        ~DX12CommandQueue( ) override;

        void WaitIdle( ) override;
        void ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        // 0 when the queue could not be signaled.
        uint64_t      SignalFence( );
        ID3D12Fence1 *GetFence( ) const;

        ID3D12CommandQueue *GetCommandQueue( ) const;
        DenOfIz_QueueType   GetQueueType( ) const;
    };
} // namespace DenOfIz
