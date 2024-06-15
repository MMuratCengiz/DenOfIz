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

#include <TF/TFCommon.h>

using namespace DenOfIz;

void TFCommon::Resize(uint32_t width, uint32_t height) { InitSwapChain(); }

uint32_t TFCommon::AcquireNextImage()
{
    if ( p_SwapChain->mEnableVsync != this->mSettings.mVSyncEnabled )
    {
        waitQueueIdle(p_GraphicsQueue);
        ::toggleVSync(p_Renderer, &p_SwapChain);
    }

    uint32_t swapchainImageIndex;
    acquireNextImage(p_Renderer, p_SwapChain, p_ImageAcquiredSemaphore, nullptr, &swapchainImageIndex);
    return swapchainImageIndex;
}

void TFCommon::Present(std::vector<Semaphore *> waitSemaphores, uint32_t swapchainImageIndex)
{
    waitSemaphores.push_back(p_ImageAcquiredSemaphore);

    QueuePresentDesc presentDesc = {
        .pSwapChain = p_SwapChain,
        .ppWaitSemaphores = waitSemaphores.data(),
        .mWaitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
        .mIndex = static_cast<uint8_t>(swapchainImageIndex),
        .mSubmitDone = true,
    };

    queuePresent(p_GraphicsQueue, &presentDesc);
}

void TFCommon::InitQueue()
{
    QueueDesc queueDesc = {};
    queueDesc.mType = QUEUE_TYPE_GRAPHICS;
    queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
    addQueue(p_Renderer, &queueDesc, &p_GraphicsQueue);

    GpuCmdRingDesc cmdRingDesc = {};
    cmdRingDesc.pQueue = p_GraphicsQueue;
    cmdRingDesc.mPoolCount = g_DataBufferCount;
    cmdRingDesc.mCmdPerPoolCount = 1;
    cmdRingDesc.mAddSyncPrimitives = true;
    addGpuCmdRing(p_Renderer, &cmdRingDesc, &GraphicsCmdRing);
}

GpuCmdRingElement TFCommon::NextCmdRingElement()
{
    GpuCmdRingElement elem = getNextGpuCmdRingElement(&GraphicsCmdRing, true, 1);

    // Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
    FenceStatus fenceStatus;
    getFenceStatus(p_Renderer, elem.pFence, &fenceStatus);
    if ( fenceStatus == FENCE_STATUS_INCOMPLETE )
    {
        waitForFences(p_Renderer, 1, &elem.pFence);
    }

    // Reset cmd pool for this frame
    resetCmdPool(p_Renderer, elem.pCmdPool);
    return elem;
}

void TFCommon::InitProfiler()
{
    const char *ppGpuProfilerName[ 1 ] = { "Graphics" };

    // Initialize micro profiler and its UI.
    ProfilerDesc profiler = {};
    profiler.pRenderer = p_Renderer;
    profiler.ppQueues = &p_GraphicsQueue;
    profiler.ppProfilerNames = ppGpuProfilerName;
    profiler.pProfileTokens = &m_gpuProfileToken;
    profiler.mGpuProfilerCount = 1;
    profiler.mWidthUI = mSettings.mWidth;
    profiler.mHeightUI = mSettings.mHeight;
    initProfiler(&profiler);

    m_GPUProfileToken = addGpuProfiler(p_Renderer, p_GraphicsQueue, "Graphics");
}

bool TFCommon::InitSwapChain()
{
    SwapChainDesc swapChainDesc = {
        .mWindowHandle = pWindow->handle,
        .ppPresentQueues = &p_GraphicsQueue,
        .mPresentQueueCount = 1,
        .mImageCount = getRecommendedSwapchainImageCount(p_Renderer, &pWindow->handle),
        .mWidth = static_cast<uint32_t>(mSettings.mWidth),
        .mHeight = static_cast<uint32_t>(mSettings.mHeight),
        .mColorFormat = getSupportedSwapchainFormat(p_Renderer, &swapChainDesc, COLOR_SPACE_SDR_SRGB),
        .mFlags = SWAP_CHAIN_CREATION_FLAG_NONE,
        .mEnableVsync = mSettings.mVSyncEnabled,
        .mColorSpace = COLOR_SPACE_SDR_SRGB,
    };

    addSwapChain(p_Renderer, &swapChainDesc, &p_SwapChain);

    if ( p_SwapChain == nullptr )
    {
        m_initializationState.SwapChain.Initialized = false;
        m_initializationState.SwapChain.InitializationMessage = "SwapChain initialization failed.";
        return false;
    }
    else
    {
        m_initializationState.SwapChain.Initialized = true;
        return true;
    }
}

bool TFCommon::Init()
{ // FILE PATHS
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

    initRenderer(GetName(), &settings, &p_Renderer);
    initRaytracing(p_Renderer, &p_Raytracing);
    initResourceLoaderInterface(p_Renderer);

    InitQueue();
    //	InitProfiler();

    addSemaphore(p_Renderer, &p_ImageAcquiredSemaphore);

    //	// Initialize Forge User Interface Rendering
    //	UserInterfaceDesc uiRenderDesc = {};
    //	uiRenderDesc.pRenderer = p_Renderer;
    //	initUserInterface(&uiRenderDesc);
    //
    //	/************************************************************************/
    //	// GUI
    //	/************************************************************************/
    //	UIComponentDesc guiDesc = {};
    //	guiDesc.mStartPosition = vec2(mSettings.mWidth * 0.01f, mSettings.mHeight * 0.15f);
    //	uiCreateComponent(GetName(), &guiDesc, &p_UIComponent);

    return true;
}

void TFCommon::Exit()
{
    removeSemaphore(p_Renderer, p_ImageAcquiredSemaphore);
    removeSwapChain(p_Renderer, p_SwapChain);
    removeGpuCmdRing(p_Renderer, &GraphicsCmdRing);
    removeQueue(p_Renderer, p_GraphicsQueue);
    removeGpuProfiler(m_GPUProfileToken);
    exitResourceLoaderInterface(p_Renderer);
    exitRenderer(p_Renderer);
    p_Renderer = NULL;
}

bool TFCommon::Load(ReloadDesc *pReloadDesc)
{
    if ( pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET) )
    {
        if ( !InitSwapChain() )
        {
            return false;
        }
    }

    //	UserInterfaceLoadDesc uiLoad = {};
    //	uiLoad.mColorFormat = p_SwapChain->ppRenderTargets[0]->mFormat;
    //	uiLoad.mHeight = mSettings.mHeight;
    //	uiLoad.mWidth = mSettings.mWidth;
    //	uiLoad.mLoadType = pReloadDesc->mType;
    //	loadUserInterface(&uiLoad);
    //
    //	FontSystemLoadDesc fontLoad = {};
    //	fontLoad.mColorFormat = p_SwapChain->ppRenderTargets[0]->mFormat;
    //	fontLoad.mHeight = mSettings.mHeight;
    //	fontLoad.mWidth = mSettings.mWidth;
    //	fontLoad.mLoadType = pReloadDesc->mType;
    //	loadFontSystem(&fontLoad);
    //
    //	initScreenshotInterface(p_Renderer, p_GraphicsQueue);
    return true;
}

void TFCommon::Unload(ReloadDesc *pReloadDesc)
{
    waitQueueIdle(p_GraphicsQueue);

    unloadFontSystem(pReloadDesc->mType);
    //	unloadUserInterface(pReloadDesc->mType);

    exitScreenshotInterface();
}

const char *TFCommon::GetName() { return "DenOfIz"; }
