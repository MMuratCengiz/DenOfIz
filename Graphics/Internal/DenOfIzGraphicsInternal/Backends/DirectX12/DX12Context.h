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
#ifdef BUILD_DX12

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Utilities/Common_Windows.h>
#include <array>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <directx/dxgicommon.h>
#include <directx/dxgiformat.h>
#include <dxgi1_6.h>
#include <wil/com.h>
#include <wil/result.h>
#include <wrl/client.h>
#include <wrl/event.h>
#include "DX12DescriptorHeap.h"
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"

#ifndef NDEBUG
#define D3D12MA_DEBUG_LOG( format, ... )                                                                                                                                           \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        wprintf( format, __VA_ARGS__ );                                                                                                                                            \
        wprintf( L"\n" );                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )
#include <dxgidebug.h>
#endif

/*
 *
 * D3D12 Memory Allocator has 2 warnings that clutter output, not sure if important
 * When building a shared library the following warnings are generated:
 * warning C4251: 'D3D12MA::IUnknownImpl::m_RefCount': 'std::atomic<unsigned int>' needs to have dll-interface to be used by clients of 'D3D12MA::IUnknownImpl'
 * warning C4251: 'D3D12MA::Allocation::m_PackedData': 'D3D12MA::Allocation::PackedData' needs to have dll-interface to be used by clients of 'D3D12MA::Allocation'
 */
#pragma warning( push )
#pragma warning( disable : 4251 )
#include <D3D12MemAlloc.h>
#pragma warning( pop )
#include <DenOfIzGraphics/Utilities/Common.h>

#define DX_CHECK_RESULT( result )                                                                                                                                                  \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( FAILED( result ) )                                                                                                                                                    \
        {                                                                                                                                                                          \
            spdlog::error("DirectX12 Layer Error: {}", result);                                                                                                                   \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )

#define DZ_WS_STRING( var, c_str )                                                                                                                                                 \
    std::string  var##_str = c_str;                                                                                                                                                \
    std::wstring var( var##_str.begin( ), var##_str.end( ) )

namespace DenOfIz
{
    struct DX12Capabilities
    {
        bool EnhancedBarriers = false;
    };

    struct DX12Context : private NonCopyable
    {
        bool IsDeviceLost = false;

        // Release Last
        wil::com_ptr<IDXGIAdapter1>      Adapter;
        wil::com_ptr<IDXGIFactory7>      DXGIFactory;
        wil::com_ptr<ID3D12Device9>      D3DDevice;
        wil::com_ptr<D3D12MA::Allocator> DX12MemoryAllocator;
        wil::com_ptr<ID3D12CommandQueue> GraphicsCommandQueue;
        wil::com_ptr<ID3D12Fence1>       GraphicsCommandQueueFence;
        wil::com_ptr<ID3D12CommandQueue> ComputeCommandQueue;
        wil::com_ptr<ID3D12Fence1>       ComputeCommandQueueFence;
        wil::com_ptr<ID3D12CommandQueue> CopyCommandQueue;
        wil::com_ptr<ID3D12Fence1>       CopyCommandQueueFence;

        wil::com_ptr<ID3D12CommandAllocator>     CopyCommandListAllocator;
        wil::com_ptr<ID3D12GraphicsCommandList4> CopyCommandList;
        wil::com_ptr<ID3D12CommandAllocator>     BarrierCommandListAllocator;
        wil::com_ptr<ID3D12GraphicsCommandList7> BarrierCommandList;

        std::array<std::unique_ptr<DX12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> CpuDescriptorHeaps;
        std::unique_ptr<DX12DescriptorHeap>                                                   RtvDescriptorHeap;
        std::unique_ptr<DX12DescriptorHeap>                                                   DsvDescriptorHeap;
        std::unique_ptr<DX12DescriptorHeap>                                                   ShaderVisibleCbvSrvUavDescriptorHeap;
        std::unique_ptr<DX12DescriptorHeap>                                                   ShaderVisibleSamplerDescriptorHeap;

        PhysicalDevice   SelectedDeviceInfo;
        DX12Capabilities DX12Capabilities;
    };
} // namespace DenOfIz

#endif
