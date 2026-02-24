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
    auto                    logicalDevice = std::unique_ptr<ILogicalDevice>( gApi.CreateAndLoadOptimalLogicalDevice( ) );
    DenOfIz_ShaderStageDesc shaderDesc{ };
    shaderDesc.Stage = DENOFIZ_SHADER_STAGE_COMPUTE_BIT;
    shaderDesc.Path  = DENOFIZ_STRING( "Assets/Shaders/Tests/GeneralTests/BasicCompute.hlsl" );
    ShaderProgramDesc programDesc{ .ShaderStages = { .Elements = &shaderDesc, .NumElements = 1 } };
    auto              program = std::make_unique<ShaderProgram>( programDesc );

    DenOfIz_ResourceBindingDesc resourceBindingDesc{ };
    resourceBindingDesc.Name    = DENOFIZ_STRING( "computeReadBack" );
    resourceBindingDesc.Binding = 0;
    resourceBindingDesc.Type    = DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS;
    resourceBindingDesc.Stages  = DENOFIZ_SHADER_STAGE_COMPUTE_BIT;
    DenOfIz_RootSignatureDesc rootSignatureDesc{ };
    rootSignatureDesc.ResourceBindings.Elements    = &resourceBindingDesc;
    rootSignatureDesc.ResourceBindings.NumElements = 1;

    auto rootSignature = std::unique_ptr<IRootSignature>( logicalDevice->CreateRootSignature( rootSignatureDesc ) );

    DenOfIz_BufferDesc bufferDesc{ };
    bufferDesc.Usage                = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    bufferDesc.NumBytes             = 1024 * sizeof( float );
    bufferDesc.StructureDesc.Stride = sizeof( float );
    bufferDesc.HeapType             = DENOFIZ_HEAP_TYPE_GPU;
    auto buffer                     = std::unique_ptr<IBuffer>( logicalDevice->CreateBuffer( bufferDesc ) );

    auto resourceBindGroup =
        std::unique_ptr<IResourceBindGroup>( logicalDevice->CreateResourceBindGroup( DenOfIz_ResourceBindGroupDesc{ .RootSignature = rootSignature.get( ) } ) );
    resourceBindGroup->BeginUpdate( )->Uav( 0, buffer.get( ) )->EndUpdate( );

    auto inputLayout = std::unique_ptr<IInputLayout>( logicalDevice->CreateInputLayout( { } ) );

    DenOfIz_PipelineDesc pipelineDesc{ .ShaderProgram = program.get( ) };
    pipelineDesc.RootSignature = rootSignature.get( );
    pipelineDesc.InputLayout   = inputLayout.get( );
    pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_COMPUTE;

    auto pipeline = std::unique_ptr<IPipeline>( logicalDevice->CreatePipeline( pipelineDesc ) );
    auto fence    = std::unique_ptr<IFence>( logicalDevice->CreateFence( ) );

    auto commandQueue    = std::unique_ptr<ICommandQueue>( logicalDevice->CreateCommandQueue( DenOfIz_CommandQueueDesc{ .QueueType = DENOFIZ_QUEUE_TYPE_COMPUTE } ) );
    auto commandListPool = std::unique_ptr<ICommandListPool>( logicalDevice->CreateCommandListPool( DenOfIz_CommandListPoolDesc{ commandQueue.get( ) } ) );
    auto commandList     = commandListPool->GetCommandLists( ).Elements[ 0 ];

    bufferDesc.Usage    = DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    bufferDesc.HeapType = DENOFIZ_HEAP_TYPE_GPU_CPU;
    auto readBack       = std::unique_ptr<IBuffer>( logicalDevice->CreateBuffer( bufferDesc ) );

    commandList->Begin( );
    commandList->BindPipeline( pipeline.get( ) );
    commandList->BindResourceGroup( resourceBindGroup.get( ) );
    commandList->Dispatch( 1024, 1, 1 );

    std::array<DenOfIz_BufferBarrierDesc, 1> barriers = { DenOfIz_BufferBarrierDesc{
        .Resource = buffer.get( ), .OldState = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT, .NewState = DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT } };
    DenOfIz_PipelineBarrierDesc              barrier{ };
    barrier.BufferBarriers.Elements    = barriers.data( );
    barrier.BufferBarriers.NumElements = barriers.size( );
    commandList->PipelineBarrier( &barrier );

    DenOfIz_CopyBufferRegionDesc copyDesc{ };
    copyDesc.DstBuffer = readBack.get( );
    copyDesc.SrcBuffer = buffer.get( );
    copyDesc.NumBytes  = 1024 * sizeof( float );
    commandList->CopyBufferRegion( copyDesc );

    std::array<DenOfIz_BufferBarrierDesc, 1> barriers2 = { DenOfIz_BufferBarrierDesc{
        .Resource = buffer.get( ), .OldState = DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT, .NewState = DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT } };
    barrier                                            = { };
    barrier.BufferBarriers.Elements                    = barriers2.data( );
    barrier.BufferBarriers.NumElements                 = barriers2.size( );
    commandList->PipelineBarrier( &barrier );

    DenOfIz_ExecuteCommandListsDesc executeCommandListsDesc{ };
    executeCommandListsDesc.Signal                   = fence.get( );
    executeCommandListsDesc.CommandLists.Elements    = &commandList;
    executeCommandListsDesc.CommandLists.NumElements = 1;
    commandQueue->ExecuteCommandLists( executeCommandListsDesc );

    fence->Wait( );

    auto *mappedData = static_cast<float *>( readBack->MapMemory( ) );
    for ( int i = 0; i < 1024; i++ )
    {
        ASSERT_EQ( mappedData[ i ], i * 10.0f );
    }
    readBack->UnmapMemory( );
}

TEST( General, BasicCompute_Win32_DX12 )
{
    const GraphicsApi gApi( { .Windows = DENOFIZ_API_PREFERENCE_WINDOWS_DIRECTX12 } );
    BasicCompute( gApi );
}

TEST( General, BasicCompute_Win32_Vulkan )
{
    const GraphicsApi gApi( { .Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN } );
    BasicCompute( gApi );
}
