/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once
#ifdef BUILD_DX12

#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Utilities/Common_Windows.h>
#include <array>
#include <atomic>
#include <mutex>
#include <vector>
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
#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync.h"

#include <dxgidebug.h>

#ifndef NDEBUG
#define D3D12MA_DEBUG_LOG( format, ... )                                                                                                                                           \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        wprintf( format, __VA_ARGS__ );                                                                                                                                            \
        wprintf( L"\n" );                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )
#endif

// D3D12MA members trigger C4251 (needs dll-interface) when building a shared library.
#pragma warning( push )
#pragma warning( disable : 4251 )
#include <D3D12MemAlloc.h>
#pragma warning( pop )
#include <DenOfIzGraphics/Utilities/Common.h>

#include "DX12IndirectSignatureCache.h"

#define DX_CHECK_RESULT( result )                                                                                                                                                  \
    do                                                                                                                                                                             \
    {                                                                                                                                                                              \
        if ( FAILED( result ) )                                                                                                                                                    \
        {                                                                                                                                                                          \
            spdlog::error( "DirectX12 Layer Error: {}", result );                                                                                                                  \
        }                                                                                                                                                                          \
    }                                                                                                                                                                              \
    while ( false )

#define DZ_WS_STRING( var, c_str )                                                                                                                                                 \
    std::string  var##_str = c_str;                                                                                                                                                \
    std::wstring var( var##_str.begin( ), var##_str.end( ) )

namespace DenOfIz
{
    class DX12CommandQueue;

    struct DX12Capabilities
    {
        bool EnhancedBarriers                 = false;
        bool CastingFullyTypedFormatSupported = false;
    };

    struct DX12Context
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
        std::atomic<uint64_t> GraphicsCommandQueueFenceValue{ 0 };
        std::atomic<uint64_t> ComputeCommandQueueFenceValue{ 0 };
        std::atomic<uint64_t> CopyCommandQueueFenceValue{ 0 };
        std::mutex                      CommandQueuesMutex;
        std::vector<DX12CommandQueue *> CommandQueues;

        wil::com_ptr<ID3D12CommandAllocator>     CopyCommandListAllocator;
        wil::com_ptr<ID3D12GraphicsCommandList4> CopyCommandList;
        wil::com_ptr<ID3D12CommandAllocator>     BarrierCommandListAllocator;
        wil::com_ptr<ID3D12GraphicsCommandList7> BarrierCommandList;

        std::array<std::unique_ptr<DX12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> CpuDescriptorHeaps;
        std::unique_ptr<DX12DescriptorHeap>                                                   RtvDescriptorHeap;
        std::unique_ptr<DX12DescriptorHeap>                                                   DsvDescriptorHeap;
        std::unique_ptr<DX12DescriptorHeap>                                                   ShaderVisibleCbvSrvUavDescriptorHeap;
        std::unique_ptr<DX12DescriptorHeap>                                                   ShaderVisibleSamplerDescriptorHeap;

        std::unique_ptr<DX12IndirectSignatureCache> IndirectSignatureCache;

        DenOfIz_PhysicalDevice SelectedDeviceInfo;
        DX12Capabilities       DX12Capabilities;

        InternalAutoSync *AutoSync;
    };
} // namespace DenOfIz

#endif
