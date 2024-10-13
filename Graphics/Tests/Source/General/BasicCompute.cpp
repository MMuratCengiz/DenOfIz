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

#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "gtest/gtest.h"

using namespace DenOfIz;

void BasicCompute( const GraphicsApi &gApi )
{
    std::unique_ptr<ILogicalDevice> logicalDevice = gApi.CreateAndLoadOptimalLogicalDevice( );
    std::unique_ptr<ShaderProgram>  program       = gApi.CreateShaderProgram( {
        ShaderDesc{ .Stage = ShaderStage::Compute, .Path = "Assets/Shaders/Tests/GeneralTests/BasicCompute.hlsl" },
    } );
    std::unique_ptr<IRootSignature> rootSignature = std::unique_ptr<IRootSignature>( logicalDevice->CreateRootSignature(RootSignatureDesc{
        .ResourceBindings = {
            ResourceBindingDesc{ .Name = "computeReadBack", .Binding = 0, .Descriptor = ResourceDescriptor::RWBuffer, .Stages = { ShaderStage::Compute } },
        },
    }) );

    BufferDesc bufferDesc{ };
    bufferDesc.Descriptor                   = ResourceDescriptor::RWBuffer;
    bufferDesc.NumBytes                     = 1024 * sizeof( float );
    bufferDesc.BufferView.Stride            = sizeof( float );
    bufferDesc.HeapType                     = HeapType::GPU;
    bufferDesc.InitialState                 = ResourceState::UnorderedAccess;
    std::unique_ptr<IBufferResource> buffer = std::unique_ptr<IBufferResource>( logicalDevice->CreateBufferResource( bufferDesc ) );

    std::unique_ptr<IResourceBindGroup> resourceBindGroup =
        std::unique_ptr<IResourceBindGroup>( logicalDevice->CreateResourceBindGroup( ResourceBindGroupDesc{ .RootSignature = rootSignature.get( ) } ) );
    resourceBindGroup->Update( UpdateDesc{ 0 }.Uav( 0, buffer.get( ) ) );

    std::unique_ptr<IInputLayout> inputLayout = std::unique_ptr<IInputLayout>( logicalDevice->CreateInputLayout( { } ) );

    PipelineDesc pipelineDesc{ .ShaderProgram = program.get( ) };
    pipelineDesc.RootSignature = rootSignature.get( );
    pipelineDesc.InputLayout   = inputLayout.get( );
    pipelineDesc.BindPoint     = BindPoint::Compute;

    std::unique_ptr<IPipeline> pipeline = std::unique_ptr<IPipeline>( logicalDevice->CreatePipeline( pipelineDesc ) );
    std::unique_ptr<IFence>    fence    = std::unique_ptr<IFence>( logicalDevice->CreateFence( ) );

    std::unique_ptr<ICommandListPool> commandListPool =
        std::unique_ptr<ICommandListPool>( logicalDevice->CreateCommandListPool( CommandListPoolDesc{ .QueueType = QueueType::Compute } ) );
    auto commandList = commandListPool->GetCommandLists( )[ 0 ];

    bufferDesc.Descriptor   = BitSet<ResourceDescriptor>( );
    bufferDesc.HeapType     = HeapType::GPU_CPU;
    bufferDesc.InitialState = ResourceState::CopyDst;
    auto readBack           = std::unique_ptr<IBufferResource>( logicalDevice->CreateBufferResource( bufferDesc ) );

    commandList->Begin( );
    commandList->BindPipeline( pipeline.get( ) );
    commandList->BindResourceGroup( resourceBindGroup.get( ) );
    commandList->Dispatch( 1024, 1, 1 );

    PipelineBarrierDesc barrier{ };
    barrier.BufferBarrier( BufferBarrierDesc{ .Resource = buffer.get( ), .OldState = ResourceState::UnorderedAccess, .NewState = ResourceState::CopySrc } );
    commandList->PipelineBarrier( barrier );

    CopyBufferRegionDesc copyDesc{ };
    copyDesc.DstBuffer = readBack.get( );
    copyDesc.SrcBuffer = buffer.get( );
    copyDesc.NumBytes  = 1024 * sizeof( float );
    commandList->CopyBufferRegion( copyDesc );

    barrier = { };
    barrier.BufferBarrier( BufferBarrierDesc{ .Resource = buffer.get( ), .OldState = ResourceState::CopySrc, .NewState = ResourceState::UnorderedAccess } );
    commandList->PipelineBarrier( barrier );

    ExecuteDesc executeDesc{ };
    executeDesc.Notify = fence.get( );
    commandList->Execute( executeDesc );

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
