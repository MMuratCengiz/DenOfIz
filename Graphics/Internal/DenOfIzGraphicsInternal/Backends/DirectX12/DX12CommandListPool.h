/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandListPool.h>
#include "DX12CommandList.h"

namespace DenOfIz
{
    class DX12CommandListPool : public ICommandListPool
    {
        DX12Context                                         *m_context;
        std::vector<wil::com_ptr<ID3D12CommandAllocator>>    m_commandAllocators;
        std::vector<wil::com_ptr<ID3D12GraphicsCommandList>> m_dx12CommandLists;
        std::vector<std::unique_ptr<DX12CommandList>>        m_commandLists;
        std::vector<DenOfIz_CommandList>                     m_commandListPtrs;
        DenOfIz_CommandListPoolDesc                          m_desc;

    public:
        DX12CommandListPool( DX12Context *context, DenOfIz_CommandListPoolDesc desc );
        DenOfIz_CommandListArray GetCommandLists( ) override;
        ~DX12CommandListPool( ) override = default;
    };
} // namespace DenOfIz
