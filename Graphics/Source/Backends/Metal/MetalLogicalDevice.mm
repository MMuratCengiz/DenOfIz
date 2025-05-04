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
#import <DenOfIzGraphics/Backends/Metal/MetalLogicalDevice.h>
#import <DenOfIzGraphics/Backends/Metal/MetalCommandQueue.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalBottomLevelAS.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderBindingTable.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderLocalData.h>
#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalTopLevelAS.h>
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

void MetalLogicalDevice::CreateDevice( )
{
}

InteropArray<PhysicalDevice> MetalLogicalDevice::ListPhysicalDevices( )
{
    NSArray<id<MTLDevice>> *devices     = MTLCopyAllDevices( );
    auto                    deviceCount = (uint32_t)[devices count];

    InteropArray<PhysicalDevice> physicalDevices( deviceCount );
    for ( uint32_t i = 0; i < deviceCount; i++ )
    {
        auto          *device = [devices objectAtIndex:i];
        PhysicalDevice physicalDevice;
        physicalDevice.Name                                   = device.name.cString;
        physicalDevice.Id                                     = device.registryID;
        physicalDevice.Capabilities.ComputeShaders            = true;
        physicalDevice.Capabilities.GeometryShaders           = false;
        physicalDevice.Capabilities.DedicatedCopyQueue        = false;
        physicalDevice.Capabilities.HDR                       = true;
        physicalDevice.Capabilities.Tearing                   = true;
        physicalDevice.Capabilities.Tessellation              = true;
        physicalDevice.Capabilities.RayTracing                = [device supportsRaytracing];
        physicalDevice.Capabilities.MeshShaders               = [device supportsFamilyMacOS1] || [device supportsFamilyApple8];
        physicalDevice.Capabilities.VariableRateShading       = [device supportsFamilyMacOS1] || [device supportsFamilyApple7];
        physicalDevice.Capabilities.ShaderInt16               = true;
        physicalDevice.Capabilities.ShaderFloat16             = true;
        physicalDevice.Capabilities.TiledResources            = true;
        physicalDevice.Capabilities.SamplerFeedback           = false;
        physicalDevice.Capabilities.ConservativeRasterization = false;
        physicalDevice.Properties.IsDedicated                 = ![device isLowPower];
        physicalDevice.Properties.MemoryAvailableInMb         = 0;
        physicalDevices.SetElement( i, physicalDevice );
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
}

ICommandQueue *MetalLogicalDevice::CreateCommandQueue( const CommandQueueDesc &desc )
{
    return new MetalCommandQueue( m_context.get( ), desc );
}

ICommandListPool *MetalLogicalDevice::CreateCommandListPool( const CommandListPoolDesc &poolDesc )
{
    return new MetalCommandListPool( m_context.get( ), poolDesc );
}

IPipeline *MetalLogicalDevice::CreatePipeline( const PipelineDesc &pipelineDesc )
{
    return new MetalPipeline( m_context.get( ), pipelineDesc );
}

ISwapChain *MetalLogicalDevice::CreateSwapChain( const SwapChainDesc &swapChainDesc )
{
    return new MetalSwapChain( m_context.get( ), swapChainDesc );
}

IRootSignature *MetalLogicalDevice::CreateRootSignature( const RootSignatureDesc &rootSignatureDesc )
{
    return new MetalRootSignature( m_context.get( ), rootSignatureDesc );
}

IInputLayout *MetalLogicalDevice::CreateInputLayout( const InputLayoutDesc &inputLayoutDesc )
{
    return new MetalInputLayout( m_context.get( ), inputLayoutDesc );
}

IResourceBindGroup *MetalLogicalDevice::CreateResourceBindGroup( const ResourceBindGroupDesc &descriptorTableDesc )
{
    return new MetalResourceBindGroup( m_context.get( ), descriptorTableDesc );
}

IFence *MetalLogicalDevice::CreateFence( )
{
    return new MetalFence( m_context.get( ) );
}

ISemaphore *MetalLogicalDevice::CreateSemaphore( )
{
    return new MetalSemaphore( m_context.get( ) );
}

IBufferResource *MetalLogicalDevice::CreateBufferResource( const BufferDesc &bufferDesc )
{
    return new MetalBufferResource( m_context.get( ), bufferDesc );
}

ITextureResource *MetalLogicalDevice::CreateTextureResource( const TextureDesc &textureDesc )
{
    return new MetalTextureResource( m_context.get( ), textureDesc );
}

ISampler *MetalLogicalDevice::CreateSampler( const SamplerDesc &samplerDesc )
{
    return new MetalSampler( m_context.get( ), samplerDesc );
}

IBottomLevelAS *MetalLogicalDevice::CreateBottomLevelAS( const BottomLevelASDesc &desc )
{
    return new MetalBottomLevelAS( m_context.get( ), desc );
}

ITopLevelAS *MetalLogicalDevice::CreateTopLevelAS( const TopLevelASDesc &desc )
{
    return new MetalTopLevelAS( m_context.get( ), desc );
}

IShaderBindingTable *MetalLogicalDevice::CreateShaderBindingTable( const ShaderBindingTableDesc &desc )
{
    return new MetalShaderBindingTable( m_context.get( ), desc );
}

ILocalRootSignature *MetalLogicalDevice::CreateLocalRootSignature( const LocalRootSignatureDesc &desc )
{
    return new MetalLocalRootSignature( m_context.get( ), desc );
}

IShaderLocalData *MetalLogicalDevice::CreateShaderLocalData( const ShaderLocalDataDesc &createDesc )
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
