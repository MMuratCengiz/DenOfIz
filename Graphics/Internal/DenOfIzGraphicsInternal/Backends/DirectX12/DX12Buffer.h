/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <string>
#include "DX12Context.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"

namespace DenOfIz
{

    enum class DX12BufferViewType
    {
        ShaderResource,
        UnorderedAccess,
        ConstantBuffer,
        AccelerationStructure,
    };

    class DX12Buffer final : public IBuffer
    {
        DX12Context                               *m_context;
        DenOfIz_BufferDesc                         m_desc;
        std::string                                m_debugName;
        wil::com_ptr<ID3D12Resource2>              m_resource;
        wil::com_ptr<D3D12MA::Allocation>          m_allocation;
        uint32_t                                   m_numBytes     = 0;
        const void                                *m_data         = nullptr;
        void                                      *m_mappedMemory = nullptr;
        uint32_t                                   m_state;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> m_cpuHandles;
        bool                                       allocated = false;

    public:
        DX12Buffer( DX12Context *context, DenOfIz_BufferDesc desc );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DenOfIz_ResourceBindingType type, uint32_t offset = 0 );
        void CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DX12BufferViewType type, uint32_t offset = 0 );

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle( DX12BufferViewType type ) const;
        [[nodiscard]] ID3D12Resource2            *Resource( ) const;
        void                                     *MapMemory( ) override;
        void                                      UnmapMemory( ) override;

        ~DX12Buffer( ) override;
        size_t      NumBytes( ) const override;
        const void *Data( ) const override;

        [[nodiscard]] DenOfIz_ByteArray GetData( ) const override;
        void                            SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) override;
        void                            WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) override;

    private:
        void CreateViewInternal( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DX12BufferViewType type, uint32_t offset ) const;
    };

} // namespace DenOfIz
