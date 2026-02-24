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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12IndirectSignatureCache.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Context.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12IndirectSignatureCache::DX12IndirectSignatureCache( ID3D12Device *device ) : m_device( device )
{
    m_dispatchSignature = CreateDispatchSignature( );
    PrefillCommonSignatures( );
}

void DX12IndirectSignatureCache::PrefillCommonSignatures( )
{
    {
        constexpr uint32_t drawStrides[] = {
            sizeof( D3D12_DRAW_ARGUMENTS ),
            64,
            128,
        };

        for ( uint32_t stride : drawStrides )
        {
            m_drawSignatures[ stride ] = CreateDrawSignature( stride );
        }
    }
    {
        constexpr uint32_t drawIndexedStrides[] = {
            sizeof( D3D12_DRAW_INDEXED_ARGUMENTS ),
            64,
            128,
        };

        for ( uint32_t stride : drawIndexedStrides )
        {
            m_drawIndexedSignatures[ stride ] = CreateDrawIndexedSignature( stride );
        }
    }
}

ID3D12CommandSignature *DX12IndirectSignatureCache::GetDrawSignature( uint32_t stride )
{
    if ( stride == 0 )
    {
        stride = sizeof( D3D12_DRAW_ARGUMENTS );
    }

    {
        std::lock_guard lock( m_cacheMutex );
        const auto      it = m_drawSignatures.find( stride );
        if ( it != m_drawSignatures.end( ) )
        {
            return it->second.get( );
        }
    }

    const auto signature = CreateDrawSignature( stride );

    {
        std::lock_guard lock( m_cacheMutex );
        m_drawSignatures[ stride ] = signature;
    }

    return signature.get( );
}

ID3D12CommandSignature *DX12IndirectSignatureCache::GetDrawIndexedSignature( uint32_t stride )
{
    if ( stride == 0 )
    {
        stride = sizeof( D3D12_DRAW_INDEXED_ARGUMENTS );
    }

    {
        std::lock_guard lock( m_cacheMutex );
        const auto      it = m_drawIndexedSignatures.find( stride );
        if ( it != m_drawIndexedSignatures.end( ) )
        {
            return it->second.get( );
        }
    }

    const auto signature = CreateDrawIndexedSignature( stride );

    {
        std::lock_guard lock( m_cacheMutex );
        m_drawIndexedSignatures[ stride ] = signature;
    }

    return signature.get( );
}

ID3D12CommandSignature *DX12IndirectSignatureCache::GetDispatchSignature( ) const
{
    return m_dispatchSignature.get( );
}

wil::com_ptr<ID3D12CommandSignature> DX12IndirectSignatureCache::CreateDrawSignature( uint32_t stride ) const
{
    D3D12_INDIRECT_ARGUMENT_DESC drawArg = { };
    drawArg.Type                         = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC desc = { };
    desc.ByteStride                   = stride;
    desc.NumArgumentDescs             = 1;
    desc.pArgumentDescs               = &drawArg;
    desc.NodeMask                     = 0;

    wil::com_ptr<ID3D12CommandSignature> signature;
    DX_CHECK_RESULT( m_device->CreateCommandSignature( &desc, nullptr, IID_PPV_ARGS( signature.put( ) ) ) );

    return signature;
}

wil::com_ptr<ID3D12CommandSignature> DX12IndirectSignatureCache::CreateDrawIndexedSignature( uint32_t stride ) const
{
    D3D12_INDIRECT_ARGUMENT_DESC drawIndexedArg = { };
    drawIndexedArg.Type                         = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

    D3D12_COMMAND_SIGNATURE_DESC desc = { };
    desc.ByteStride                   = stride;
    desc.NumArgumentDescs             = 1;
    desc.pArgumentDescs               = &drawIndexedArg;
    desc.NodeMask                     = 0;

    wil::com_ptr<ID3D12CommandSignature> signature;
    DX_CHECK_RESULT( m_device->CreateCommandSignature( &desc, nullptr, IID_PPV_ARGS( signature.put( ) ) ) );

    return signature;
}

wil::com_ptr<ID3D12CommandSignature> DX12IndirectSignatureCache::CreateDispatchSignature( ) const
{
    D3D12_INDIRECT_ARGUMENT_DESC dispatchArg = { };
    dispatchArg.Type                         = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC desc = { };
    desc.ByteStride                   = sizeof( D3D12_DISPATCH_ARGUMENTS );
    desc.NumArgumentDescs             = 1;
    desc.pArgumentDescs               = &dispatchArg;
    desc.NodeMask                     = 0;

    wil::com_ptr<ID3D12CommandSignature> signature;
    DX_CHECK_RESULT( m_device->CreateCommandSignature( &desc, nullptr, IID_PPV_ARGS( signature.put( ) ) ) );

    return signature;
}
