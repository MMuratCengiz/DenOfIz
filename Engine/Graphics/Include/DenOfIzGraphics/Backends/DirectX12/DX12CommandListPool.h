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

#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include "DX12CommandList.h"

namespace DenOfIz
{
    class DX12CommandListPool : public ICommandListPool
    {
    private:
        DX12Context                                         *m_context;
        std::vector<wil::com_ptr<ID3D12CommandAllocator>>    m_commandAllocators;
        std::vector<wil::com_ptr<ID3D12GraphicsCommandList>> m_dx12CommandLists;
        std::vector<std::unique_ptr<DX12CommandList>>        m_commandLists;
        CommandListPoolDesc                                  m_desc;

    public:
        DX12CommandListPool( DX12Context *context, CommandListPoolDesc desc ) : m_desc( desc )
        {
            DZ_NOT_NULL( context );
            DZ_ASSERTM( desc.NumCommandLists > 0, "CommandListCount must be greater than 0" );

            m_context = context;
            for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
            {
                D3D12_COMMAND_LIST_TYPE commandListType = DX12EnumConverter::ConvertQueueType( desc.QueueType );

                wil::com_ptr<ID3D12CommandAllocator> commandAllocator;
                DX_CHECK_RESULT( context->D3DDevice->CreateCommandAllocator( commandListType, IID_PPV_ARGS( commandAllocator.put( ) ) ) );

                wil::com_ptr<ID3D12GraphicsCommandList> dx12CommandList;
                DX_CHECK_RESULT( context->D3DDevice->CreateCommandList( 0, commandListType, commandAllocator.get( ), nullptr, IID_PPV_ARGS( dx12CommandList.put( ) ) ) );
                dx12CommandList->Close( );

                m_dx12CommandLists.push_back( std::move( dx12CommandList ) );
                m_commandAllocators.push_back( std::move( commandAllocator ) );
            }

            CommandListDesc commandListCreateInfo{ };
            commandListCreateInfo.QueueType = m_desc.QueueType;

            for ( uint32_t i = 0; i < desc.NumCommandLists; i++ )
            {
                m_commandLists.push_back( std::make_unique<DX12CommandList>( m_context, m_commandAllocators[ i ], m_dx12CommandLists[ i ], commandListCreateInfo ) );
            }
        }

        virtual std::vector<ICommandList *> GetCommandLists( ) override
        {
            std::vector<ICommandList *> commandLists;
            for ( auto &commandList : m_commandLists )
            {
                commandLists.push_back( commandList.get( ) );
            }
            return commandLists;
        }

        ~DX12CommandListPool( ) override = default;
    };
} // namespace DenOfIz
