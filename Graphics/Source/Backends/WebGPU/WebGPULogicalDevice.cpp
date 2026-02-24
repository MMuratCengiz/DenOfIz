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

#ifdef BUILD_WEBGPU_NATIVE
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPULogicalDevice.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandListPool.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUCommandQueue.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUFence.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUInputLayout.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUPipelineCache.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUQueryPool.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPURootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUSemaphore.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUSwapChain.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUTexture.h"

using namespace DenOfIz;

WebGPULogicalDevice::WebGPULogicalDevice( ) : m_context( std::make_unique<WebGPUContext>( ) )
{
}

WebGPULogicalDevice::~WebGPULogicalDevice( )
{
    if ( m_context->Queue )
    {
        wgpuQueueRelease( m_context->Queue );
        m_context->Queue = nullptr;
    }

    if ( m_context->Device )
    {
        wgpuDeviceRelease( m_context->Device );
        m_context->Device = nullptr;
    }

    if ( m_context->Adapter )
    {
        wgpuAdapterRelease( m_context->Adapter );
        m_context->Adapter = nullptr;
    }

    if ( m_context->Instance )
    {
        wgpuInstanceRelease( m_context->Instance );
        m_context->Instance = nullptr;
    }
}

void WebGPULogicalDevice::CreateDevice( const DenOfIz_LogicalDeviceDesc &desc )
{
    m_deviceDesc = desc;
    m_context    = std::make_unique<WebGPUContext>( );
#if DZ_WEBGPU_USE_DAWN_API
    constexpr WGPUInstanceDescriptor instanceDesc = { };
    m_context->Instance                           = wgpuCreateInstance( &instanceDesc );
#else
    m_context->Instance = wgpuCreateInstance( nullptr );
#endif

    if ( !m_context->Instance )
    {
        spdlog::critical( "WebGPU: Failed to create instance" );
        return;
    }

    WGPURequestAdapterOptions adapterOpts = { };
    adapterOpts.powerPreference           = WGPUPowerPreference_HighPerformance;

    struct AdapterData
    {
        WGPUAdapter adapter = nullptr;
        bool        done    = false;
    } adapterData;

#if DZ_WEBGPU_USE_DAWN_API
    auto onAdapterRequestEnded = []( WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userdata1, void *userdata2 )
    {
        const auto data = static_cast<AdapterData *>( userdata1 );
        if ( status == WGPURequestAdapterStatus_Success )
        {
            data->adapter = adapter;
        }
        else
        {
            spdlog::error( "WebGPU: Failed to request adapter: {}", message.data ? message.data : "Unknown error" );
        }
        data->done = true;
    };

    WGPURequestAdapterCallbackInfo callbackInfo = { };
#if defined( __EMSCRIPTEN__ )
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
#else
    callbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
#endif
    callbackInfo.callback  = onAdapterRequestEnded;
    callbackInfo.userdata1 = &adapterData;
    callbackInfo.userdata2 = nullptr;

    wgpuInstanceRequestAdapter( m_context->Instance, &adapterOpts, callbackInfo );
#else
    auto onAdapterRequestEnded = []( WGPURequestAdapterStatus status, WGPUAdapter adapter, const char *message, void *userdata )
    {
        const auto data = static_cast<AdapterData *>( userdata );
        if ( status == WGPURequestAdapterStatus_Success )
        {
            data->adapter = adapter;
        }
        else
        {
            spdlog::error( "WebGPU: Failed to request adapter: {}", message ? message : "Unknown error" );
        }
        data->done = true;
    };

    wgpuInstanceRequestAdapter( m_context->Instance, &adapterOpts, onAdapterRequestEnded, &adapterData );
#endif

#if defined( __EMSCRIPTEN__ )
    while ( !adapterData.done )
    {
        emscripten_sleep( 10 );
    }
#else
    while ( !adapterData.done )
    {
#if defined( WEBGPU_BACKEND_WGPU )
        SDL_Delay( 10 );
#else
        wgpuInstanceProcessEvents( m_context->Instance );
        SDL_Delay( 100 );
#endif
    }
#endif

    m_context->Adapter = adapterData.adapter;
    if ( !m_context->Adapter )
    {
        spdlog::critical( "WebGPU: Failed to get adapter" );
        return;
    }
    WGPUDeviceDescriptor deviceDesc = { };
    deviceDesc.label                = DZ_WEBGPU_STRING( "WebGPU Device" );
#if DZ_WEBGPU_USE_DAWN_API
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredFeatures     = nullptr;
    auto onUncapturedError          = []( WGPUDevice const *device, WGPUErrorType type, WGPUStringView message, void *userdata1, void *userdata2 )
    {
        auto errorType = "Unknown";
        switch ( type )
        {
        case WGPUErrorType_NoError:
            errorType = "NoError";
            break;
        case WGPUErrorType_Validation:
            errorType = "Validation";
            break;
        case WGPUErrorType_OutOfMemory:
            errorType = "OutOfMemory";
            break;
        case WGPUErrorType_Internal:
            errorType = "Internal";
            break;
        case WGPUErrorType_Unknown:
            errorType = "Unknown";
            break;
        default:
            break;
        }
        spdlog::error( "WebGPU Uncaptured Error [{}]: {}", errorType, message.data ? message.data : "No message" );
    };

    deviceDesc.uncapturedErrorCallbackInfo.callback  = onUncapturedError;
    deviceDesc.uncapturedErrorCallbackInfo.userdata1 = nullptr;
    deviceDesc.uncapturedErrorCallbackInfo.userdata2 = nullptr;

    auto onDeviceLost = []( WGPUDevice const *device, WGPUDeviceLostReason reason, WGPUStringView message, void *userdata1, void *userdata2 )
    {
        auto reasonStr = "Unknown";
        switch ( reason )
        {
        case WGPUDeviceLostReason_Unknown:
            reasonStr = "Unknown";
            break;
        case WGPUDeviceLostReason_Destroyed:
            reasonStr = "Destroyed";
            break;
        case WGPUDeviceLostReason_Force32:
            reasonStr = "Force32";
            break;
        default:
            reasonStr = "Unknown";
            break;
        }
        spdlog::error( "WebGPU Device Lost [{}]: {}", reasonStr, message.data ? message.data : "No message" );
    };

    deviceDesc.deviceLostCallbackInfo.mode      = WGPUCallbackMode_AllowProcessEvents;
    deviceDesc.deviceLostCallbackInfo.callback  = onDeviceLost;
    deviceDesc.deviceLostCallbackInfo.userdata1 = nullptr;
    deviceDesc.deviceLostCallbackInfo.userdata2 = nullptr;
#endif

    struct DeviceData
    {
        WGPUDevice device = nullptr;
        bool       done   = false;
    } deviceData;

#if DZ_WEBGPU_USE_DAWN_API
    auto onDeviceRequestEnded = []( WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userdata1, void *userdata2 )
    {
        auto *data = static_cast<DeviceData *>( userdata1 );
        if ( status == WGPURequestDeviceStatus_Success )
        {
            data->device = device;
        }
        else
        {
            spdlog::error( "WebGPU: Failed to request device: {}", message.data ? message.data : "Unknown error" );
        }
        data->done = true;
    };

    WGPURequestDeviceCallbackInfo deviceCallbackInfo = { };
#if defined( __EMSCRIPTEN__ )
    deviceCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
#else
    deviceCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
#endif
    deviceCallbackInfo.callback  = onDeviceRequestEnded;
    deviceCallbackInfo.userdata1 = &deviceData;
    deviceCallbackInfo.userdata2 = nullptr;

    wgpuAdapterRequestDevice( m_context->Adapter, &deviceDesc, deviceCallbackInfo );
#else
    auto onDeviceRequestEnded = []( WGPURequestDeviceStatus status, WGPUDevice device, const char *message, void *userdata )
    {
        auto *data = static_cast<DeviceData *>( userdata );
        if ( status == WGPURequestDeviceStatus_Success )
        {
            data->device = device;
        }
        else
        {
            spdlog::error( "WebGPU: Failed to request device: {}", message ? message : "Unknown error" );
        }
        data->done = true;
    };

    wgpuAdapterRequestDevice( m_context->Adapter, &deviceDesc, onDeviceRequestEnded, &deviceData );
#endif

#if defined( __EMSCRIPTEN__ )
    while ( !deviceData.done )
    {
        emscripten_sleep( 10 );
    }
#else
    while ( !deviceData.done )
    {
#if defined( WEBGPU_BACKEND_WGPU )
        SDL_Delay( 10 );
#else
        wgpuInstanceProcessEvents( m_context->Instance );
        SDL_Delay( 100 );
#endif
    }
#endif

    m_context->Device = deviceData.device;
    if ( !m_context->Device )
    {
        spdlog::critical( "WebGPU: Failed to create device" );
        return;
    }

    m_context->Queue = wgpuDeviceGetQueue( m_context->Device );
    if ( !m_context->Queue )
    {
        spdlog::critical( "WebGPU: Failed to get device queue" );
        return;
    }

#ifdef BUILD_WEBGPU_NATIVE
    spdlog::info( "WebGPU: Device initialized successfully" );
#else
    spdlog::info( "WebGPU: Device initialized from browser context" );
#endif

    QueryAdapterCapabilities( );

    m_autoSync          = std::make_unique<InternalAutoSync>( m_deviceDesc.AutoSync );
    m_context->AutoSync = m_autoSync.get( );
}

DenOfIz_PhysicalDeviceArray WebGPULogicalDevice::ListPhysicalDevices( )
{
    if ( m_physicalDevices.empty( ) && m_context->Adapter )
    {
        DenOfIz_PhysicalDevice device;
        device.Id = 0;

#if DZ_WEBGPU_USE_DAWN_API
        WGPUAdapterInfo info = { };
        if ( wgpuAdapterGetInfo( m_context->Adapter, &info ) == WGPUStatus_Success )
        {
            m_deviceNameStorage.emplace_back( info.device.data ? info.device.data : "WebGPU Adapter" );
            device.Name                   = DENOFIZ_STRING( m_deviceNameStorage.back( ).c_str( ) );
            device.Properties.IsDedicated = info.backendType == WGPUBackendType_D3D12 || info.backendType == WGPUBackendType_Vulkan || info.backendType == WGPUBackendType_Metal;
            wgpuAdapterInfoFreeMembers( info );
        }
        else
        {
            m_deviceNameStorage.emplace_back( "WebGPU Adapter" );
            device.Name = DENOFIZ_STRING( m_deviceNameStorage.back( ).c_str( ) );
        }

        WGPULimits limits = { };
        if ( wgpuAdapterGetLimits( m_context->Adapter, &limits ) == WGPUStatus_Success )
        {
            device.Properties.MemoryAvailableInMb = static_cast<uint32_t>( limits.maxBufferSize / ( 1024 * 1024 ) );
        }

        device.Constants.ConstantBufferAlignment = limits.minUniformBufferOffsetAlignment;
        device.Constants.StorageBufferAlignment  = limits.minStorageBufferOffsetAlignment;
#else
        m_deviceNameStorage.emplace_back( "WebGPU Adapter" );
        device.Name                   = DENOFIZ_STRING( m_deviceNameStorage.back( ).c_str( ) );
        device.Properties.IsDedicated = false;

        device.Constants.ConstantBufferAlignment = 256;
        device.Constants.StorageBufferAlignment  = 256;
#endif

        device.Capabilities.ComputeShaders     = true;
        device.Capabilities.Tearing            = false;
        device.Capabilities.RayTracing         = false;
        device.Capabilities.MeshShaders        = false;
        device.Capabilities.DedicatedCopyQueue = false;
        device.Capabilities.MultiDrawIndirect  = true;
        device.Capabilities.QueryStatistics    = false;
        device.Capabilities.SrvArray           = false;
        device.Capabilities.Bindless           = false;

        device.Constants.BufferTextureAlignment    = 256;
        device.Constants.BufferTextureRowAlignment = 256;

        m_physicalDevices.push_back( device );
    }

    DenOfIz_PhysicalDeviceArray result{ };
    result.Elements    = m_physicalDevices.data( );
    result.NumElements = static_cast<uint32_t>( m_physicalDevices.size( ) );
    return result;
}

void WebGPULogicalDevice::LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device )
{
    m_selectedDeviceInfo      = device;
    m_context->SelectedDevice = device;
    spdlog::info( "WebGPU: Selected device: {}", device.Name.Chars );
}

bool WebGPULogicalDevice::IsDeviceLost( )
{
    return false;
}

void WebGPULogicalDevice::WaitIdle( )
{
    if ( !m_context->Queue )
    {
        return;
    }

    WebGPUFence fence{ m_context.get( ) };
    fence.PrepareForSignal( );

    wgpuQueueSubmit( m_context->Queue, 0, nullptr );

#if DZ_WEBGPU_USE_DAWN_API
    WGPUQueueWorkDoneCallbackInfo workDoneInfo = { };
    workDoneInfo.callback                      = WebGPUFence::OnWorkDone;
    workDoneInfo.userdata1                     = &fence;
#if defined( __EMSCRIPTEN__ )
    workDoneInfo.mode = WGPUCallbackMode_AllowSpontaneous;
#else
    workDoneInfo.mode = WGPUCallbackMode_AllowProcessEvents;
#endif
    wgpuQueueOnSubmittedWorkDone( m_context->Queue, workDoneInfo );
#else
    wgpuQueueOnSubmittedWorkDone( m_context->Queue, WebGPUFence::OnWorkDone, &fence );
#endif
    fence.Wait( );
}

void WebGPULogicalDevice::QueryAdapterCapabilities( ) const
{
    if ( !m_context->Adapter )
        return;

#if DZ_WEBGPU_USE_DAWN_API
    WGPULimits limits = { };
    if ( wgpuAdapterGetLimits( m_context->Adapter, &limits ) == WGPUStatus_Success )
    {
        spdlog::info( "WebGPU Adapter Limits:" );
        spdlog::info( "  Max Texture Dimension 1D: {}", limits.maxTextureDimension1D );
        spdlog::info( "  Max Texture Dimension 2D: {}", limits.maxTextureDimension2D );
        spdlog::info( "  Max Texture Dimension 3D: {}", limits.maxTextureDimension3D );
        spdlog::info( "  Max Texture Array Layers: {}", limits.maxTextureArrayLayers );
        spdlog::info( "  Max Buffer Size: {} MB", limits.maxBufferSize / ( 1024 * 1024 ) );
        spdlog::info( "  Max Vertex Buffers: {}", limits.maxVertexBuffers );
        spdlog::info( "  Max Vertex Attributes: {}", limits.maxVertexAttributes );
        spdlog::info( "  Max Uniform Buffer Binding Size: {} KB", limits.maxUniformBufferBindingSize / 1024 );
        spdlog::info( "  Max Storage Buffer Binding Size: {} MB", limits.maxStorageBufferBindingSize / ( 1024 * 1024 ) );
    }
#else
    spdlog::info( "WebGPU Adapter Limits: Not available on this platform" );
#endif
}

ICommandQueue *WebGPULogicalDevice::CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc )
{
    return new WebGPUCommandQueue( m_context.get( ), desc );
}

ICommandListPool *WebGPULogicalDevice::CreateCommandListPool( const DenOfIz_CommandListPoolDesc &desc )
{
    return new WebGPUCommandListPool( m_context.get( ), desc );
}

IPipeline *WebGPULogicalDevice::CreatePipeline( const DenOfIz_PipelineDesc &desc )
{
    return new WebGPUPipeline( m_context.get( ), desc );
}

ISwapChain *WebGPULogicalDevice::CreateSwapChain( const DenOfIz_SwapChainDesc &desc )
{
    return new WebGPUSwapChain( m_context.get( ), desc );
}

IRootSignature *WebGPULogicalDevice::CreateRootSignature( const DenOfIz_RootSignatureDesc &desc )
{
    return new WebGPURootSignature( m_context.get( ), desc );
}

IInputLayout *WebGPULogicalDevice::CreateInputLayout( const DenOfIz_InputLayoutDesc &desc )
{
    return new WebGPUInputLayout( desc );
}

IBindGroupLayout *WebGPULogicalDevice::CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc )
{
    return new WebGPUBindGroupLayout( m_context.get( ), desc );
}

IBindGroup *WebGPULogicalDevice::CreateBindGroup( const DenOfIz_BindGroupDesc &desc )
{
    return new WebGPUBindGroup( m_context.get( ), desc );
}

IFence *WebGPULogicalDevice::CreateFence( )
{
    return new WebGPUFence( m_context.get( ) );
}

ISemaphore *WebGPULogicalDevice::CreateSemaphore( )
{
    return new WebGPUSemaphore( m_context.get( ) );
}

IBuffer *WebGPULogicalDevice::CreateBuffer( const DenOfIz_BufferDesc &desc )
{
    return new WebGPUBuffer( m_context.get( ), desc );
}

ITexture *WebGPULogicalDevice::CreateTexture( const DenOfIz_TextureDesc &desc )
{
    return new WebGPUTexture( m_context.get( ), desc );
}

ISampler *WebGPULogicalDevice::CreateSampler( const DenOfIz_SamplerDesc &desc )
{
    return new WebGPUSampler( m_context.get( ), desc );
}

IQueryPool *WebGPULogicalDevice::CreateQueryPool( const DenOfIz_QueryPoolDesc &desc )
{
    return new WebGPUQueryPool( m_context.get( ), desc );
}

IPipelineCache *WebGPULogicalDevice::CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc )
{
    return new WebGPUPipelineCache( m_context.get( ), desc );
}

ITopLevelAS *WebGPULogicalDevice::CreateTopLevelAS( const DenOfIz_TopLevelASDesc &desc )
{
    spdlog::critical( "WebGPU: Ray tracing not supported - CreateTopLevelAS" );
    return nullptr;
}

IBottomLevelAS *WebGPULogicalDevice::CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &desc )
{
    spdlog::critical( "WebGPU: Ray tracing not supported - CreateBottomLevelAS" );
    return nullptr;
}

IShaderBindingTable *WebGPULogicalDevice::CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &desc )
{
    spdlog::critical( "WebGPU: Ray tracing not supported - CreateShaderBindingTable" );
    return nullptr;
}

ILocalRootSignature *WebGPULogicalDevice::CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &desc )
{
    spdlog::critical( "WebGPU: Ray tracing not supported - CreateLocalRootSignature" );
    return nullptr;
}

IShaderLocalData *WebGPULogicalDevice::CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &desc )
{
    spdlog::critical( "WebGPU: Ray tracing not supported - CreateShaderLocalData" );
    return nullptr;
}

WebGPUContext *WebGPULogicalDevice::GetContext( ) const
{
    return m_context.get( );
}
