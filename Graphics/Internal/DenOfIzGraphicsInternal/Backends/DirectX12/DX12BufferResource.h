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

#include "DX12Context.h"
#include "DenOfIzGraphics/Backends/Interface/IBufferResource.h"

namespace DenOfIz
{

    enum class DX12BufferViewType
    {
        ShaderResource,
        UnorderedAccess,
        ConstantBuffer,
        AccelerationStructure,
    };

    class DX12BufferResource final : public IBufferResource
    {
        DX12Context                               *m_context;
        BufferDesc                                 m_desc;
        wil::com_ptr<ID3D12Resource2>              m_resource;
        wil::com_ptr<D3D12MA::Allocation>          m_allocation;
        uint32_t                                   m_numBytes     = 0;
        const void                                *m_data         = nullptr;
        void                                      *m_mappedMemory = nullptr;
        uint32_t                                   m_state;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> m_cpuHandles;
        bool                                       allocated = false;

    public:
        DX12BufferResource( DX12Context *context, BufferDesc desc );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ResourceBindingType type, uint32_t offset = 0 );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DX12BufferViewType type, uint32_t offset = 0 );

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle( DX12BufferViewType type ) const;
        [[nodiscard]] ID3D12Resource2            *Resource( ) const;
        void                                     *MapMemory( ) override;
        void                                      UnmapMemory( ) override;

        ~DX12BufferResource( ) override;
        uint32_t    InitialState( ) const override;
        size_t      NumBytes( ) const override;
        const void *Data( ) const override;

        // Interop API
        [[nodiscard]] ByteArray GetData( ) const override;
        void                    SetData( const ByteArrayView &data, bool keepMapped ) override;
        void                    WriteData( const ByteArrayView &data, uint32_t bufferOffset ) override;

    private:
        void CreateViewInternal( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DX12BufferViewType type, uint32_t offset ) const;
    };

} // namespace DenOfIz
