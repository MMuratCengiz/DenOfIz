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

#include <TheForgeContext.h>

using namespace DenOfIz;

TheForgeContext::TheForgeContext(const InitialAppInfo& appInfo)
		:m_appInfo(appInfo)
{
	// FILE PATHS
	fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SHADER_BINARIES, "Assets/CompiledShaders");
	fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_TEXTURES, "Assets/Textures");
	fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_FONTS, "Assets/Fonts");
	fsSetPathForResourceDir(pSystemFileIO, RM_CONTENT, RD_SCRIPTS, "Assets/Scripts");
	fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_SCREENSHOTS, "Screenshots");
	fsSetPathForResourceDir(pSystemFileIO, RM_DEBUG, RD_DEBUG, "Debug");

	RendererDesc settings = {};
	settings.mShaderTarget = SHADER_TARGET_6_3;
#if defined(SHADER_STATS_AVAILABLE)
	settings.mEnableShaderStats = true;
#endif

	initRenderer(m_appInfo.Name.c_str(), &settings, &pRenderer);
	initResourceLoaderInterface(pRenderer);

	InitQueue();

	addSemaphore(pRenderer, &pImageAcquiredSemaphore);


	// Initialize Forge User Interface Rendering
	UserInterfaceDesc uiRenderDesc = {};
	uiRenderDesc.pRenderer = pRenderer;
	initUserInterface(&uiRenderDesc);

	/************************************************************************/
	// GUI
	/************************************************************************/
	UIComponentDesc guiDesc = {};
	guiDesc.mStartPosition = vec2(m_appInfo.Width * 0.01f, m_appInfo.Height * 0.15f);
	uiCreateComponent(m_appInfo.Name.c_str(), &guiDesc, &pUIComponent);
}

void TheForgeContext::InitQueue()
{
	QueueDesc queueDesc = {};
	queueDesc.mType = QUEUE_TYPE_GRAPHICS;
	queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
	addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

	GpuCmdRingDesc cmdRingDesc = {};
	cmdRingDesc.pQueue = pGraphicsQueue;
	cmdRingDesc.mPoolCount = gDataBufferCount;
	cmdRingDesc.mCmdPerPoolCount = 1;
	cmdRingDesc.mAddSyncPrimitives = true;
	addGpuCmdRing(pRenderer, &cmdRingDesc, &mCmdRing);
}

void TheForgeContext::InitProfiler()
{
	const char* ppGpuProfilerName[1] = { "Graphics" };

	// Initialize micro profiler and its UI.
	ProfilerDesc profiler = {};
	profiler.pRenderer = pRenderer;
	profiler.ppQueues = &pGraphicsQueue;
	profiler.ppProfilerNames = ppGpuProfilerName;
	profiler.pProfileTokens = &m_GpuProfileToken;
	profiler.mGpuProfilerCount = 1;
	profiler.mWidthUI = m_appInfo.Width;
	profiler.mHeightUI = m_appInfo.Height;
	initProfiler(&profiler);
}

void TheForgeContext::InitImgui()
{

}

TheForgeContext::~TheForgeContext()
{
	removeSemaphore(pRenderer, pImageAcquiredSemaphore);
	removeGpuCmdRing(pRenderer, &mCmdRing);
	removeQueue(pRenderer, pGraphicsQueue);
	exitResourceLoaderInterface(pRenderer);
	exitRenderer(pRenderer);
	pRenderer = NULL;
}
