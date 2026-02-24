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
#include <directx/d3dx12.h>
#include <mutex>
#include <wil/com.h>

using namespace Microsoft::WRL;

namespace DenOfIz
{

    struct DescriptorHandle
    {
        bool                          GpuVisible = false;
        CD3DX12_CPU_DESCRIPTOR_HANDLE Cpu{ };
        CD3DX12_GPU_DESCRIPTOR_HANDLE Gpu{ };
    };

    class DX12DescriptorHeap
    {
        std::mutex                 m_mutex;
        ID3D12Device              *m_device;
        D3D12_DESCRIPTOR_HEAP_TYPE m_type;

        wil::com_ptr<ID3D12DescriptorHeap> m_heap;
        DescriptorHandle                   m_startHandle;
        uint32_t                           m_maxDescriptors;
        uint32_t                           m_descriptorSize;
        uint32_t                           m_usedDescriptors;
        uint32_t                          *m_flags;
        bool                               m_shaderVisible;
        static constexpr uint32_t          DESCRIPTOR_HEAP_BLOCK_SIZE = 32;

    public:
        DX12DescriptorHeap( ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible );

        DescriptorHandle                                         AllocateDescriptors( uint32_t count );
        void                                                     FreeDescriptors( const DescriptorHandle &handle, uint32_t count );
        [[nodiscard]] uint32_t                                   GetDescriptorSize( ) const;
        [[nodiscard]] ID3D12DescriptorHeap                      *GetActiveHeap( ) const;
        [[nodiscard]] const std::vector<ID3D12DescriptorHeap *> &GetActiveHeaps( ) const;

        static uint32_t RoundUp( const uint32_t size, const uint32_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT );
        ~DX12DescriptorHeap( );

    private:
        void     CreateHeap( );
        uint32_t ConsumeDescriptorHandles( uint32_t descriptorCount ) const;
        void     ReturnDescriptorHandles( uint32_t startIndex, uint32_t descriptorCount ) const;
    };

} // namespace DenOfIz
