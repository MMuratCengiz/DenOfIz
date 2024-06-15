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

#include <string>

#include "Common_3/Application/Interfaces/IApp.h"
#include "Common_3/Application/Interfaces/ICameraController.h"
#include "Common_3/Application/Interfaces/IFont.h"
#include "Common_3/Application/Interfaces/IInput.h"
#include "Common_3/Application/Interfaces/IProfiler.h"
#include "Common_3/Application/Interfaces/IScreenshot.h"
#include "Common_3/Application/Interfaces/IUI.h"
#include "Common_3/Game/Interfaces/IScripting.h"
#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "Common_3/Resources/ResourceLoader/Interfaces/IResourceLoader.h"
#include "Common_3/Utilities/Interfaces/IFileSystem.h"
#include "Common_3/Utilities/Interfaces/ILog.h"
#include "Common_3/Utilities/Interfaces/ITime.h"
#include "Common_3/Utilities/RingBuffer.h"
#include "Common_3/Utilities/Threading/ThreadSystem.h"
#include "Common_3/Graphics/Interfaces/IRay.h"
#include <vector>

namespace DenOfIz
{

struct InitialAppInfo
{
	std::string Name = "DenOfIz";
	int Width = 1920;
	int Height = 1080;

	bool VSyncEnabled = false;
};

struct InitializationState
{
	bool Initialized = false;
	std::string InitializationMessage = "Undefined.";
};

struct SystemInitializationState
{
	InitializationState Renderer{};
	InitializationState UI{};
	InitializationState Profiler{};
	InitializationState Fonts{};
	InitializationState SwapChain{};
};

class TFCommon : public IApp
{
private:
	SystemInitializationState m_initializationState{};
	ProfileToken m_gpuProfileToken;
	ThreadSystem m_threadSystem;

	std::vector<float> m_triangle{ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
								   -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
								   0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
public:
	static const uint32_t g_DataBufferCount = 2;

	UIComponent* p_UIComponent;
	Renderer* p_Renderer;
	Raytracing* p_Raytracing;
	Queue* p_GraphicsQueue;

	GpuCmdRing GraphicsCmdRing = {};
	SwapChain * p_SwapChain;

	Semaphore* p_ImageAcquiredSemaphore;
	uint32_t m_FrameIndex = 0;
	ProfileToken m_GPUProfileToken;

	TFCommon() = default;
	uint32_t AcquireNextImage();
	GpuCmdRingElement NextCmdRingElement();
	void Resize(uint32_t width, uint32_t height);
	void Present(std::vector<Semaphore*> waitSemaphores, uint32_t swapchainImageIndex);

	virtual bool Init() override;
	virtual void Exit() override;
	virtual bool Load(ReloadDesc* pReloadDesc) override;
	virtual void Unload(ReloadDesc* pReloadDesc) override;
	const char* GetName() override;
private:
	bool InitSwapChain();
	void InitQueue();
	void InitProfiler();
};

}