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

#include <DenOfIzGraphics/Backends/DirectX12/DX12SwapChain.h>
#include "SDL_syswm.h"

using namespace DenOfIz;

DX12SwapChain::DX12SwapChain(DX12Context* context)
	:m_context(context)
{
}

uint32_t DX12SwapChain::AcquireNextImage()
{
	return m_context->SwapChain->GetCurrentBackBufferIndex();
}

void DX12SwapChain::Resize(uint32_t width, uint32_t height)
{
}

void DX12SwapChain::Present()
{
	SDL_Surface* surface = SDL_GetWindowSurface(m_context->Window);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_context->Window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = surface->w;
	swapChainDesc.Height = surface->h;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_context->BackBufferCount;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

//	swapChainDesc.Flags = (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u; TODO

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;

	// Create a swap chain for the window.
	ComPtr<IDXGISwapChain1> swapChain;

	DX_CHECK_RESULT(m_context->DXGIFactory->CreateSwapChainForHwnd(m_context->CommandQueue.Get(), hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain.GetAddressOf()));
	DX_CHECK_RESULT(swapChain.As(&m_context->SwapChain));
	DX_CHECK_RESULT(m_context->DXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
}

DX12SwapChain::~DX12SwapChain()
{
	m_context->SwapChain->Release();
}