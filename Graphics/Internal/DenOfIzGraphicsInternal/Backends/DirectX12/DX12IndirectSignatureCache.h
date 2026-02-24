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

#include <DenOfIzGraphics/Utilities/Common_Macro.h>
#include <DenOfIzGraphics/Utilities/Common_Windows.h>
#include <directx/d3d12.h>
#include <mutex>
#include <unordered_map>
#include <wil/com.h>

namespace DenOfIz
{
    class DX12IndirectSignatureCache
    {
        ID3D12Device *m_device;

        std::unordered_map<uint32_t, wil::com_ptr<ID3D12CommandSignature>> m_drawSignatures;
        std::unordered_map<uint32_t, wil::com_ptr<ID3D12CommandSignature>> m_drawIndexedSignatures;
        wil::com_ptr<ID3D12CommandSignature>                               m_dispatchSignature;

        mutable std::mutex m_cacheMutex;

    public:
        DX12IndirectSignatureCache( ID3D12Device *device );
        ~DX12IndirectSignatureCache( ) = default;

        ID3D12CommandSignature *GetDrawSignature( uint32_t stride );
        ID3D12CommandSignature *GetDrawIndexedSignature( uint32_t stride );
        ID3D12CommandSignature *GetDispatchSignature( ) const;

    private:
        wil::com_ptr<ID3D12CommandSignature> CreateDrawSignature( uint32_t stride ) const;
        wil::com_ptr<ID3D12CommandSignature> CreateDrawIndexedSignature( uint32_t stride ) const;
        wil::com_ptr<ID3D12CommandSignature> CreateDispatchSignature( ) const;

        void PrefillCommonSignatures( );
    };
} // namespace DenOfIz
