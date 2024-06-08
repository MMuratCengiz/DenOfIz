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

#include <DenOfIzCore/Common_Windows.h>
#include <DenOfIzCore/Logger.h>
#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#include <directx/dxgicommon.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <wrl/event.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "D3D12MemAlloc.h"

#ifdef DEBUG
#include <dxgidebug.h>
#endif

#include <DenOfIzCore/Common.h>

using namespace Microsoft::WRL;

namespace DenOfIz
{

struct DX12Context
{
	static const int BackBufferCount = 3;
	bool IsDeviceLost = false;

	ComPtr<IDXGIAdapter1> Adapter;

	ComPtr<ID3D12Device14> D3DDevice;
	ComPtr<IDXGIFactory7> DXGIFactory;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12CommandAllocator> DirectCommandAllocator[BackBufferCount];
	ComPtr<ID3D12GraphicsCommandList4> DirectCommandList;

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	DXGI_FORMAT DepthBufferFormat = DXGI_FORMAT_D32_FLOAT;
	// RenderTargetView and DepthStencilView Descriptor Heap
	ComPtr<ID3D12DescriptorHeap> RTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> DSVDescriptorHeap;
	ComPtr<IDXGISwapChain3> SwapChain;
	std::vector<ComPtr<ID3D12Resource>> SwapChainImages;

	SDL_Window* Window;
	PhysicalDeviceInfo SelectedDeviceInfo;
	D3D12MA::Allocator* DX12MemoryAllocator;
};

}

#define DX_CHECK_RESULT(exp) 																				\
do                                                                                    						\
{                                                                                    						\
	HRESULT hrNoConflictName = exp;                                                         				\
	if (!SUCCEEDED(hrNoConflictName))                                                                     	\
	{                                                                                       				\
		std::string message = (boost::format("FAILED with HRESULT:") % (uint32_t)hrNoConflictName).str(); 	\
		LOG(Verbosity::Critical, "DirectX", message);                                       				\
		assertm(false, message);                                                            				\
	}                                                                                    					\
} while (false)

#endif
