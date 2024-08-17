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

#include <DenOfIzGraphics/Backends/Metal/MetalLogicalDevice.h>

#define IR_PRIVATE_IMPLEMENTATION
#include <metal_irconverter_runtime/metal_irconverter_runtime.h>

using namespace DenOfIz;

MetalLogicalDevice::MetalLogicalDevice( )
{
    m_context = std::make_unique<MetalContext>( );
}

MetalLogicalDevice::~MetalLogicalDevice( )
{
}

void MetalLogicalDevice::CreateDevice( )
{
}

std::vector<PhysicalDevice> MetalLogicalDevice::ListPhysicalDevices( )
{
    NSArray<id<MTLDevice>> *devices     = MTLCopyAllDevices( );
    auto                    deviceCount = (uint32_t)[devices count];

    std::vector<PhysicalDevice> physicalDevices;
    physicalDevices.reserve( deviceCount );

    for ( uint32_t i = 0; i < deviceCount; i++ )
    {
        auto          *device = [devices objectAtIndex:i];
        PhysicalDevice physicalDevice;
        physicalDevice.Name                            = device.name.cString;
        physicalDevice.Id                              = device.registryID;
        physicalDevice.Capabilities.ComputeShaders     = true;
        physicalDevice.Capabilities.GeometryShaders    = false;
        physicalDevice.Capabilities.DedicatedCopyQueue = false;
        physicalDevice.Capabilities.HDR                = true;
        physicalDevice.Capabilities.Tearing            = true;
        physicalDevice.Capabilities.Tessellation       = true;
        physicalDevice.Capabilities.RayTracing         = [device supportsRaytracing];
        physicalDevice.Properties.IsDedicated          = ![device isLowPower];
        physicalDevice.Properties.MemoryAvailableInMb  = 0;
        physicalDevices.push_back( physicalDevice );
    }

    return physicalDevices;
}

void MetalLogicalDevice::LoadPhysicalDevice( const PhysicalDevice &device )
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

    m_context->CommandQueue = [m_context->Device newCommandQueue];
    m_selectedDeviceInfo    = device;

    m_selectedDeviceInfo.Constants.ConstantBufferAlignment   = 256;
    m_selectedDeviceInfo.Constants.BufferTextureAlignment    = 16;
    m_selectedDeviceInfo.Constants.BufferTextureRowAlignment = 1;

    m_context->SelectedDeviceInfo = m_selectedDeviceInfo;

    MTLHeapDescriptor *heapDesc   = [[MTLHeapDescriptor alloc] init];
    heapDesc.size                 = 4 * 1024;
    heapDesc.storageMode          = MTLStorageModeShared;
    heapDesc.hazardTrackingMode   = MTLHazardTrackingModeTracked;
    heapDesc.type                 = MTLHeapTypeAutomatic;
    m_context->ReadOnlyHeap       = [m_context->Device newHeapWithDescriptor:heapDesc];
}

std::unique_ptr<ICommandListPool> MetalLogicalDevice::CreateCommandListPool( const CommandListPoolDesc &poolDesc )
{
    auto *pool = new MetalCommandListPool( m_context.get( ), poolDesc );
    return std::unique_ptr<ICommandListPool>( pool );
}

std::unique_ptr<IPipeline> MetalLogicalDevice::CreatePipeline( const PipelineDesc &pipelineDesc )
{
    auto *pipeline = new MetalPipeline( m_context.get( ), pipelineDesc );
    return std::unique_ptr<IPipeline>( pipeline );
}

std::unique_ptr<ISwapChain> MetalLogicalDevice::CreateSwapChain( const SwapChainDesc &swapChainDesc )
{
    auto *swapChain = new MetalSwapChain( m_context.get( ), swapChainDesc );
    return std::unique_ptr<ISwapChain>( swapChain );
}

std::unique_ptr<IRootSignature> MetalLogicalDevice::CreateRootSignature( const RootSignatureDesc &rootSignatureDesc )
{
    auto *rootSignature = new MetalRootSignature( m_context.get( ), rootSignatureDesc );
    return std::unique_ptr<IRootSignature>( rootSignature );
}

std::unique_ptr<IInputLayout> MetalLogicalDevice::CreateInputLayout( const InputLayoutDesc &inputLayoutDesc )
{
    auto *inputLayout = new MetalInputLayout( m_context.get( ), inputLayoutDesc );
    return std::unique_ptr<IInputLayout>( inputLayout );
}

std::unique_ptr<IResourceBindGroup> MetalLogicalDevice::CreateResourceBindGroup( const ResourceBindGroupDesc &descriptorTableDesc )
{
    auto *resourceBindGroup = new MetalResourceBindGroup( m_context.get( ), descriptorTableDesc );
    return std::unique_ptr<IResourceBindGroup>( resourceBindGroup );
}

std::unique_ptr<IFence> MetalLogicalDevice::CreateFence( )
{
    auto *fence = new MetalFence( m_context.get( ) );
    return std::unique_ptr<IFence>( fence );
}

std::unique_ptr<ISemaphore> MetalLogicalDevice::CreateSemaphore( )
{
    auto *semaphore = new MetalSemaphore( m_context.get( ) );
    return std::unique_ptr<ISemaphore>( semaphore );
}

std::unique_ptr<IBufferResource> MetalLogicalDevice::CreateBufferResource( std::string name, const BufferDesc &bufferDesc )
{
    auto *buffer = new MetalBufferResource( m_context.get( ), bufferDesc, name );
    return std::unique_ptr<IBufferResource>( buffer );
}

std::unique_ptr<ITextureResource> MetalLogicalDevice::CreateTextureResource( std::string name, const TextureDesc &textureDesc )
{
    auto *texture = new MetalTextureResource( m_context.get( ), textureDesc, name );
    return std::unique_ptr<ITextureResource>( texture );
}

std::unique_ptr<ISampler> MetalLogicalDevice::CreateSampler( std::string name, const SamplerDesc &samplerDesc )
{
    auto *sampler = new MetalSampler( m_context.get( ), samplerDesc, name );
    return std::unique_ptr<ISampler>( sampler );
}

void MetalLogicalDevice::WaitIdle( )
{
}
