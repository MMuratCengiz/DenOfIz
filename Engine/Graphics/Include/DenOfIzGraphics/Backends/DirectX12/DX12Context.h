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

#include <DenOfIzCore/Common.h>
#include <d3d12.h>

#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "directx/d3dx12.h"

struct DX12Context
{
    ID3D12Device5 *D3DDevice;

    IDXGISwapChain3 *SwapChain;
    IDXGIFactory4 *DXGIFactory;

    ID3D12CommandQueue *CommandQueue;
    ID3D12CommandAllocator *CommandAllocators;

    ID3D12GraphicsCommandList4 *GraphicsCommandList;

    DXGI_FORMAT BackBufferFormat;
    DXGI_FORMAT DepthBufferFormat;

    D3D12_VIEWPORT ScreenViewport;
    D3D12_RECT ScissorRect;
};

#endif
