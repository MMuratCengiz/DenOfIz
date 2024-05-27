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

#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>

#include "DenOfIzCore/Logger.h"
#include "SDL2/SDL_syswm.h"

using namespace DenOfIz;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

DX12LogicalDevice::DX12LogicalDevice()
{
}

DX12LogicalDevice::~DX12LogicalDevice()
{
	WaitIdle();
	Dispose();
}

void DX12LogicalDevice::CreateDevice(SDL_Window* window)
{
	m_context->Window = window;
#if defined(DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()))))
		{
			debugController->EnableDebugLayer();
		}
		else
		{
			LOG(Verbosity::Warning, "DX12Device", "WARNING: Direct3D Debug Device is not available");
		}

		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			m_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
					80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */, };
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}
#endif

	DX_CHECK_RESULT(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_context->DXGIFactory.ReleaseAndGetAddressOf())));
}

std::vector<PhysicalDeviceInfo> DX12LogicalDevice::ListPhysicalDevices()
{
	std::vector<PhysicalDeviceInfo> result;
	ComPtr<IDXGIAdapter1> adapter;
	for (UINT adapterIndex = 0; SUCCEEDED(
			m_context->DXGIFactory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf())); adapterIndex++)
	{
		PhysicalDeviceInfo deviceInfo{};
		CreateDeviceInfo(*adapter.Get(), deviceInfo);
		result.push_back(deviceInfo);
	}

	return result;
}

void DX12LogicalDevice::CreateDeviceInfo(IDXGIAdapter1& adapter, PhysicalDeviceInfo& deviceInfo)
{
	DXGI_ADAPTER_DESC adapterDesc;
	adapter.GetDesc(&adapterDesc);
	deviceInfo.Id = adapterDesc.DeviceId;
	deviceInfo.Name = LPCSTR(adapterDesc.Description);

	// Todo actually read these from somewhere:
	deviceInfo.Capabilities.DedicatedTransferQueue = true;
	deviceInfo.Capabilities.ComputeShaders = true;

	DXGI_ADAPTER_DESC1 desc;
	DX_CHECK_RESULT(adapter.GetDesc1(&desc));

	deviceInfo.Properties.IsDedicated = !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);

	ComPtr<ID3D12Device> device;
	DX_CHECK_RESULT(D3D12CreateDevice(&adapter, m_minFeatureLevel, IID_PPV_ARGS(device.GetAddressOf())));
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts = {};
	if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts, sizeof(opts))))
	{
		deviceInfo.Capabilities.RayTracing = opts.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
	}

	bool allowTearing = false;
	ComPtr<IDXGIFactory5> factory5;
	HRESULT hr = m_context->DXGIFactory.As(&factory5);
	if (SUCCEEDED(hr))
	{
		hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
	}

	if (FAILED(hr) || !allowTearing)
	{
		deviceInfo.Capabilities.Tearing = false;
		LOG(Verbosity::Warning, "DX12Device", "WARNING: Variable refresh rate displays not supported");
	}
}

void DX12LogicalDevice::LoadPhysicalDevice(const PhysicalDeviceInfo& device)
{
	m_selectedDeviceInfo = device;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory6;
	HRESULT hr = m_context->DXGIFactory.As(&factory6);

	DX_CHECK_RESULT(hr);

	for (UINT adapterIndex = 0; SUCCEEDED(
			factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))); adapterIndex++)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		adapter->GetDesc(&adapterDesc);
		if (device.Id == adapterDesc.DeviceId)
		{
			break;
		}
	}

	m_context->Adapter = adapter.Detach();

	// Create the DX12 API device object.
	DX_CHECK_RESULT(D3D12CreateDevice(m_context->Adapter.Get(), m_minFeatureLevel, IID_PPV_ARGS(m_context->D3DDevice.ReleaseAndGetAddressOf())));

	m_context->D3DDevice->SetName(L"DeviceResources");

	// Confirm the device supports DXR.
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts = {};
	if (FAILED(m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts, sizeof(opts))) || opts.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
	{
		OutputDebugStringA("DirectX Raytracing support not found.\n");

	}

	// Confirm the device supports Shader Model 6.3 or better.
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_3 };
	if (FAILED(m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) || shaderModel.HighestShaderModel <
			D3D_SHADER_MODEL_6_3)
	{
		OutputDebugStringA("ERROR: Requires Shader Model 6.3 or better support.\n");
		throw std::exception("Requires Shader Model 6.3 or better support");
	}

#ifndef NDEBUG
	// Configure debug device (if active).
	ComPtr<ID3D12InfoQueue> d3dInfoQueue;
	if (SUCCEEDED(m_context->D3DDevice.As(&d3dInfoQueue)))
	{
#ifdef _DEBUG
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
		D3D12_MESSAGE_ID hide[] = { D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
				// Workarounds for debug layer issues on hybrid-graphics systems
									D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE, D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE, };
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(hide);
		filter.DenyList.pIDList = hide;
		d3dInfoQueue->AddStorageFilterEntries(&filter);
	}
#endif

	// Determine maximum supported feature level for this device
	static constexpr D3D_FEATURE_LEVEL s_featureLevels[] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

	D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = { _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0 };

	hr = m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
	if (SUCCEEDED(hr))
	{
		m_minFeatureLevel = featLevels.MaxSupportedFeatureLevel;
	}
	else
	{
		m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
	}

	// Create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX_CHECK_RESULT(m_context->D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_context->CommandQueue.ReleaseAndGetAddressOf())));

	m_context->CommandQueue->SetName(L"DeviceResources");

	// Create descriptor heaps for render target views and depth stencil views.
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.NumDescriptors = m_context->BackBufferCount;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	DX_CHECK_RESULT(m_context->D3DDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(m_context->RTVDescriptorHeap.ReleaseAndGetAddressOf())));

	m_context->RTVDescriptorHeap->SetName(L"DeviceResources");

//	m_rtvDescriptorSize = m_context->D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (m_context->DepthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
		dsvDescriptorHeapDesc.NumDescriptors = 1;
		dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		DX_CHECK_RESULT(m_context->D3DDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(m_context->DSVDescriptorHeap.ReleaseAndGetAddressOf())));

		m_context->DSVDescriptorHeap->SetName(L"DeviceResources");
	}

	// Create a command allocator for each back buffer that will be rendered to.
	for (UINT n = 0; n < m_context->BackBufferCount; n++)
	{
		DX_CHECK_RESULT(m_context->D3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_context->DirectCommandAllocator[n].ReleaseAndGetAddressOf())));

		std::string name = (boost::format("Render target %1") % n).str();
		m_context->DirectCommandAllocator[n]->SetName(LPCWSTR(name.c_str()));
	}

	// Create a command list for recording graphics commands.
	DX_CHECK_RESULT(m_context->D3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_context->DirectCommandAllocator[0].Get(), nullptr,
			IID_PPV_ARGS(m_context->DirectCommandList.ReleaseAndGetAddressOf())));
	DX_CHECK_RESULT(m_context->DirectCommandList->Close());

	m_context->DirectCommandList->SetName(L"DeviceResources");

	// Create a fence for tracking GPU execution progress.
//	DX_CHECK_RESULT(m_context->D3DDevice->CreateFence(m_fenceValues[m_backBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));

//	m_fenceValues[m_backBufferIndex]++;
//	m_fence->SetName(L"DeviceResources");
//
//	m_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
//	if (!m_fenceEvent.IsValid())
//	{
//		throw std::exception("CreateEvent");
//	}
}

void DX12LogicalDevice::CreateSwapChain()
{
	SDL_Surface* surface = SDL_GetWindowSurface(m_context->Window);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_context->BackBufferCount;
	swapChainDesc.Width = surface->w;
	swapChainDesc.Height = surface->h;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_context->Window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	DX_CHECK_RESULT(m_context->DXGIFactory->CreateSwapChainForHwnd(
			m_context->CommandQueue.Get(),
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
	));

	DX_CHECK_RESULT(swapChain.As(&m_context->SwapChain));
}

void DX12LogicalDevice::Dispose()
{
//	for (UINT n = 0; n < m_context->BackBufferCount; n++)
//	{
//		m_commandAllocators[n].Reset();
//		m_renderTargets[n].Reset();
//	}
//
//	m_context->DepthStencil.Reset();
//	m_context->CommandList.Reset();
//	m_context->fence.Reset();
	m_context->CommandQueue.Reset();
	m_context->RTVDescriptorHeap.Reset();
	m_context->DSVDescriptorHeap.Reset();
	m_context->SwapChain.Reset();
	m_context->D3DDevice.Reset();
	m_context->DXGIFactory.Reset();

#ifdef DEBUG
	{
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
	}
#endif
}

void DX12LogicalDevice::WaitIdle()
{

}
