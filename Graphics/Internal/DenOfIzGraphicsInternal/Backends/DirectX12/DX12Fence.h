/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IFence.h>
#include "DX12Context.h"

namespace DenOfIz
{

    class DX12Fence final : public IFence
    {
        constexpr static UINT64   INITIAL_FENCE_VALUE = 0;
        DX12Context              *m_context;
        wil::com_ptr<ID3D12Fence> m_fence;
        UINT32                    m_fenceValue = 1;
        Wrappers::Event           m_fenceEvent;
        bool                      m_submitted = false;

    public:
        explicit DX12Fence( DX12Context *context );
        [[nodiscard]] ID3D12Fence *GetFence( ) const;
        ~DX12Fence( ) override;
        void Wait( ) override;
        void Reset( ) override;
        void NotifyCommandQueue( ID3D12CommandQueue *commandQueue );
    };

} // namespace DenOfIz
