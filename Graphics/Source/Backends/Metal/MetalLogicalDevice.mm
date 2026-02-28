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

#import "Metal/Metal.h"
#define IR_PRIVATE_IMPLEMENTATION
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalLogicalDevice.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandQueue.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalBottomLevelAS.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderBindingTable.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderLocalData.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalTopLevelAS.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalQueryPool.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalPipelineCache.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalSwapChain.h"
#import <metal_irconverter/metal_irconverter.h>
#import <metal_irconverter_runtime/metal_irconverter_runtime.h>

using namespace DenOfIz;

MetalLogicalDevice::MetalLogicalDevice( )
{
    m_context = std::make_unique<MetalContext>( );
}

MetalLogicalDevice::~MetalLogicalDevice( )
{
}

void MetalLogicalDevice::CreateDevice( const DenOfIz_LogicalDeviceDesc &desc )
{
    m_deviceDesc = desc;
}

DenOfIz_PhysicalDeviceArray MetalLogicalDevice::ListPhysicalDevices( )
{
    NSArray<id<MTLDevice>> *devices     = MTLCopyAllDevices( );
    auto                    deviceCount = (uint32_t)[devices count];

    m_physicalDevices.clear( );
    m_physicalDevices.reserve( deviceCount );
    m_deviceNameStorage.clear( );

    for ( uint32_t i = 0; i < deviceCount; i++ )
    {
        auto          *device = [devices objectAtIndex:i];
        DenOfIz_PhysicalDevice physicalDevice;
        m_deviceNameStorage.emplace_back( device.name.cString );
        physicalDevice.Name                                   = DENOFIZ_STRING( m_deviceNameStorage.back( ).c_str( ) );
        physicalDevice.Id                                     = device.registryID;
        physicalDevice.Capabilities.ComputeShaders            = true;
        physicalDevice.Capabilities.GeometryShaders           = false;
        physicalDevice.Capabilities.DedicatedCopyQueue        = false;
        physicalDevice.Capabilities.HDR                       = true;
        physicalDevice.Capabilities.Tearing                   = true;
        physicalDevice.Capabilities.Tessellation              = true;
        physicalDevice.Capabilities.RayTracing                = [device supportsRaytracing];
        physicalDevice.Capabilities.MeshShaders               = true; //Todo
        physicalDevice.Capabilities.VariableRateShading       = true; //Todo
        physicalDevice.Capabilities.ShaderInt16               = true;
        physicalDevice.Capabilities.ShaderFloat16             = true;
        physicalDevice.Capabilities.TiledResources            = true;
        physicalDevice.Capabilities.SamplerFeedback           = false;
        physicalDevice.Capabilities.ConservativeRasterization = false;
        physicalDevice.Capabilities.MultiDrawIndirect         = true;
        physicalDevice.Capabilities.QueryStatistics           = true;
        physicalDevice.Capabilities.SrvArray                  = true;
        physicalDevice.Capabilities.Bindless                  = true;
        physicalDevice.Capabilities.ShaderOutputLayer         = true;
        physicalDevice.Properties.IsDedicated                 = ![device isLowPower];
        physicalDevice.Properties.MemoryAvailableInMb         = 0;
        m_physicalDevices.push_back( physicalDevice );
    }

    DenOfIz_PhysicalDeviceArray result;
    result.Elements = m_physicalDevices.data( );
    result.NumElements = static_cast<uint32_t>( m_physicalDevices.size( ) );
    return result;
}

void MetalLogicalDevice::LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device )
{
    NSArray<id<MTLDevice>> *devices     = MTLCopyAllDevices( );
    auto                    deviceCount = (uint32_t)[devices count];

    for ( uint32_t i = 0; i < deviceCount; i++ )
    {
        if ( [devices objectAtIndex:i].registryID == device.Id )
        {
            m_context->Device = [devices objectAtIndex:i];
            break;
        }
    }

    m_context->CommandQueue = [m_context->Device newCommandQueueWithMaxCommandBufferCount:512];
    m_selectedDeviceInfo    = device;

    m_selectedDeviceInfo.Constants.StorageBufferAlignment    = 16;
    m_selectedDeviceInfo.Constants.ConstantBufferAlignment   = 256;
    m_selectedDeviceInfo.Constants.BufferTextureAlignment    = 512;
    m_selectedDeviceInfo.Constants.BufferTextureRowAlignment = 256;

    m_context->SelectedDeviceInfo = m_selectedDeviceInfo;

    MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
    heapDesc.size               = 4 * 1024;
    heapDesc.storageMode        = MTLStorageModeShared;
    heapDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
    heapDesc.type               = MTLHeapTypeAutomatic;
    m_context->ReadOnlyHeap     = [m_context->Device newHeapWithDescriptor:heapDesc];
    
    m_autoSync          = std::make_unique<InternalAutoSync>( m_deviceDesc.AutoSync );
    m_context->AutoSync = m_autoSync.get( );

    InitializeCaptureManager( );
}

ICommandQueue *MetalLogicalDevice::CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc )
{
    return new MetalCommandQueue( m_context.get( ), desc );
}

ICommandListPool *MetalLogicalDevice::CreateCommandListPool( const DenOfIz_CommandListPoolDesc &poolDesc )
{
    return new MetalCommandListPool( m_context.get( ), poolDesc );
}

IPipeline *MetalLogicalDevice::CreatePipeline( const DenOfIz_PipelineDesc &pipelineDesc )
{
    return new MetalPipeline( m_context.get( ), pipelineDesc );
}

ISwapChain *MetalLogicalDevice::CreateSwapChain( const DenOfIz_SwapChainDesc &swapChainDesc )
{
    return new MetalSwapChain( m_context.get( ), swapChainDesc );
}

IRootSignature *MetalLogicalDevice::CreateRootSignature( const DenOfIz_RootSignatureDesc &rootSignatureDesc )
{
    return new MetalRootSignature( m_context.get( ), rootSignatureDesc );
}

IInputLayout *MetalLogicalDevice::CreateInputLayout( const DenOfIz_InputLayoutDesc &inputLayoutDesc )
{
    return new MetalInputLayout( m_context.get( ), inputLayoutDesc );
}

IBindGroupLayout *MetalLogicalDevice::CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc )
{
    return new MetalBindGroupLayout( m_context.get( ), desc );
}

IBindGroup *MetalLogicalDevice::CreateBindGroup( const DenOfIz_BindGroupDesc &descriptorTableDesc )
{
    return new MetalBindGroup( m_context.get( ), descriptorTableDesc );
}

IFence *MetalLogicalDevice::CreateFence( )
{
    return new MetalFence( m_context.get( ) );
}

ISemaphore *MetalLogicalDevice::CreateSemaphore( )
{
    return new MetalSemaphore( m_context.get( ) );
}

IBuffer *MetalLogicalDevice::CreateBuffer( const DenOfIz_BufferDesc &bufferDesc )
{
    return new MetalBuffer( m_context.get( ), bufferDesc );
}

ITexture *MetalLogicalDevice::CreateTexture( const DenOfIz_TextureDesc &textureDesc )
{
    return new MetalTexture( m_context.get( ), textureDesc );
}

ISampler *MetalLogicalDevice::CreateSampler( const DenOfIz_SamplerDesc &samplerDesc )
{
    return new MetalSampler( m_context.get( ), samplerDesc );
}

IQueryPool *MetalLogicalDevice::CreateQueryPool( const DenOfIz_QueryPoolDesc &desc )
{
    return new MetalQueryPool( m_context.get( ), desc );
}

IPipelineCache *MetalLogicalDevice::CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc )
{
    return new MetalPipelineCache( m_context.get( ), desc );
}

IBottomLevelAS *MetalLogicalDevice::CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &desc )
{
    return new MetalBottomLevelAS( m_context.get( ), desc );
}

ITopLevelAS *MetalLogicalDevice::CreateTopLevelAS( const DenOfIz_TopLevelASDesc &desc )
{
    return new MetalTopLevelAS( m_context.get( ), desc );
}

IShaderBindingTable *MetalLogicalDevice::CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &desc )
{
    return new MetalShaderBindingTable( m_context.get( ), desc );
}

ILocalRootSignature *MetalLogicalDevice::CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &desc )
{
    return new MetalLocalRootSignature( m_context.get( ), desc );
}

IShaderLocalData *MetalLogicalDevice::CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &createDesc )
{
    return new MetalShaderLocalData( m_context.get( ), createDesc );
}

void MetalLogicalDevice::WaitIdle( )
{
}

bool MetalLogicalDevice::IsDeviceLost( )
{
    return false;
}

void MetalLogicalDevice::InitializeCaptureManager( )
{
    if ( !m_deviceDesc.EnableValidationLayers )
    {
        m_context->CaptureEnabled = false;
        return;
    }

    MTLCaptureManager *captureManager = [MTLCaptureManager sharedCaptureManager];
    if ( ![captureManager supportsDestination:MTLCaptureDestinationGPUTraceDocument] )
    {
        spdlog::warn( "Metal GPU capture is not available. To enable GPU capture, set these environment variables BEFORE launching:" );
        spdlog::warn( "  export MTL_DEBUG_LAYER=1" );
        spdlog::warn( "  export METAL_CAPTURE_ENABLED=1" );
        spdlog::warn( "Or run from Xcode with Metal GPU Capture enabled in the scheme diagnostics." );
        m_context->CaptureEnabled = false;
        return;
    }

    m_context->CaptureScope       = [captureManager newCaptureScopeWithDevice:m_context->Device];
    m_context->CaptureScope.label = @"DenOfIz Frame Capture";
    [captureManager setDefaultCaptureScope:m_context->CaptureScope];

    m_context->CaptureEnabled = true;
    m_context->IsCapturing    = false;

    spdlog::info( "Metal GPU capture initialized. Use DenOfIz_Metal_BeginGpuCapture/EndGpuCapture to capture frames." );
}
