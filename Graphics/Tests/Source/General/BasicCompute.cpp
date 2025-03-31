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

#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>

#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "gtest/gtest.h"

using namespace DenOfIz;

void BasicCompute( const GraphicsApi &gApi )
{
    auto            logicalDevice = std::unique_ptr<ILogicalDevice>( gApi.CreateAndLoadOptimalLogicalDevice( ) );
    ShaderStageDesc shaderDesc{ };
    shaderDesc.Stage = ShaderStage::Compute;
    shaderDesc.Path  = "Assets/Shaders/Tests/GeneralTests/BasicCompute.hlsl";
    InteropArray<ShaderStageDesc> shaderStages{ };
    shaderStages.AddElement( shaderDesc );

    ShaderProgramDesc programDesc{ .ShaderStages = shaderStages };
    auto              program = std::make_unique<ShaderProgram>( programDesc );

    InteropArray<ResourceBindingDesc> resourceBindings{ };
    ResourceBindingDesc              &resourceBindingDesc = resourceBindings.EmplaceElement( );
    resourceBindingDesc.Name                              = "computeReadBack";
    resourceBindingDesc.Binding                           = 0;
    resourceBindingDesc.Descriptor                        = ResourceDescriptor::RWBuffer;
    resourceBindingDesc.Stages.AddElement( { ShaderStage::Compute } );

    RootSignatureDesc rootSignatureDesc{ };
    rootSignatureDesc.ResourceBindings = resourceBindings;

    auto rootSignature = std::unique_ptr<IRootSignature>( logicalDevice->CreateRootSignature( rootSignatureDesc ) );

    BufferDesc bufferDesc{ };
    bufferDesc.Descriptor           = ResourceDescriptor::RWBuffer;
    bufferDesc.NumBytes             = 1024 * sizeof( float );
    bufferDesc.StructureDesc.Stride = sizeof( float );
    bufferDesc.HeapType             = HeapType::GPU;
    bufferDesc.InitialUsage         = ResourceUsage::UnorderedAccess;
    auto buffer                     = std::unique_ptr<IBufferResource>( logicalDevice->CreateBufferResource( bufferDesc ) );

    auto resourceBindGroup = std::unique_ptr<IResourceBindGroup>( logicalDevice->CreateResourceBindGroup( ResourceBindGroupDesc{ .RootSignature = rootSignature.get( ) } ) );
    resourceBindGroup->BeginUpdate( )->Uav( 0, buffer.get( ) )->EndUpdate( );

    auto inputLayout = std::unique_ptr<IInputLayout>( logicalDevice->CreateInputLayout( { } ) );

    PipelineDesc pipelineDesc{ .ShaderProgram = program.get( ) };
    pipelineDesc.RootSignature = rootSignature.get( );
    pipelineDesc.InputLayout   = inputLayout.get( );
    pipelineDesc.BindPoint     = BindPoint::Compute;

    auto pipeline = std::unique_ptr<IPipeline>( logicalDevice->CreatePipeline( pipelineDesc ) );
    auto fence    = std::unique_ptr<IFence>( logicalDevice->CreateFence( ) );

    auto commandQueue    = std::unique_ptr<ICommandQueue>( logicalDevice->CreateCommandQueue( CommandQueueDesc{ .QueueType = QueueType::Compute } ) );
    auto commandListPool = std::unique_ptr<ICommandListPool>( logicalDevice->CreateCommandListPool( CommandListPoolDesc{ commandQueue.get( ) } ) );
    auto commandList     = commandListPool->GetCommandLists( ).GetElement( 0 );

    bufferDesc.Descriptor   = BitSet<ResourceDescriptor>( );
    bufferDesc.HeapType     = HeapType::GPU_CPU;
    bufferDesc.InitialUsage = ResourceUsage::CopyDst;
    auto readBack           = std::unique_ptr<IBufferResource>( logicalDevice->CreateBufferResource( bufferDesc ) );

    commandList->Begin( );
    commandList->BindPipeline( pipeline.get( ) );
    commandList->BindResourceGroup( resourceBindGroup.get( ) );
    commandList->Dispatch( 1024, 1, 1 );

    PipelineBarrierDesc barrier{ };
    barrier.BufferBarrier( BufferBarrierDesc{ .Resource = buffer.get( ), .OldState = ResourceUsage::UnorderedAccess, .NewState = ResourceUsage::CopySrc } );
    commandList->PipelineBarrier( barrier );

    CopyBufferRegionDesc copyDesc{ };
    copyDesc.DstBuffer = readBack.get( );
    copyDesc.SrcBuffer = buffer.get( );
    copyDesc.NumBytes  = 1024 * sizeof( float );
    commandList->CopyBufferRegion( copyDesc );

    barrier = { };
    barrier.BufferBarrier( BufferBarrierDesc{ .Resource = buffer.get( ), .OldState = ResourceUsage::CopySrc, .NewState = ResourceUsage::UnorderedAccess } );
    commandList->PipelineBarrier( barrier );

    ExecuteCommandListsDesc executeCommandListsDesc{ };
    executeCommandListsDesc.Signal = fence.get( );
    executeCommandListsDesc.CommandLists.AddElement( commandList );
    commandQueue->ExecuteCommandLists( executeCommandListsDesc );

    fence->Wait( );

    auto *mappedData = static_cast<float *>( readBack->MapMemory( ) );
    for ( UINT i = 0; i < 1024; i++ )
    {
        ASSERT_EQ( mappedData[ i ], i * 10.0f );
    }
    readBack->UnmapMemory( );
}

TEST( General, BasicCompute_Win32_DX12 )
{
    const GraphicsApi gApi( { .Windows = APIPreferenceWindows::DirectX12 } );
    BasicCompute( gApi );
}

TEST( General, BasicCompute_Win32_Vulkan )
{
    const GraphicsApi gApi( { .Windows = APIPreferenceWindows::Vulkan } );
    BasicCompute( gApi );
}
