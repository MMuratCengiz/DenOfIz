/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ISemaphore.h>
#include "DX12Context.h"

namespace DenOfIz
{

    class DX12Semaphore final : public ISemaphore
    {
        DX12Context              *m_context;
        wil::com_ptr<ID3D12Fence> m_fence;
        uint64_t                  m_fenceValue;

    public:
        explicit DX12Semaphore( DX12Context *context );
        [[nodiscard]] ID3D12Fence *GetFence( ) const;
        [[nodiscard]] uint64_t     GetCurrentValue( ) const;
        ~DX12Semaphore( ) override;
        void Notify( ) override;
        bool IsCompleted( ) const override;
        void NotifyCommandQueue( ID3D12CommandQueue *commandQueue );
    };
} // namespace DenOfIz
