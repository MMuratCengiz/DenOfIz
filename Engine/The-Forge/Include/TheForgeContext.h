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

namespace DenOfIz
{

struct InitialAppInfo
{
	std::string Name = "DenOfIz";
	int Width = 1920;
	int Height = 1080;
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
};

class TheForgeContext
{
private:
	InitialAppInfo m_appInfo{};
	SystemInitializationState m_InitializationState{};
	ProfileToken m_GpuProfileToken;
public:
	static const uint32_t gDataBufferCount = 2;

	UIComponent* pUIComponent;
	Renderer* pRenderer;
	Raytracing* pRaytracing = NULL;

	Queue* pGraphicsQueue;
	GpuCmdRing mCmdRing = {};

	Semaphore* pImageAcquiredSemaphore;
	TheForgeContext(const InitialAppInfo& appInfo);
	~TheForgeContext();
private:
	void InitQueue();
	void InitProfiler();
	void InitImgui();
};

}