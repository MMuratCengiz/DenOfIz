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

#include <DenOfIzGraphics/Renderer/ComputeTest.h>

using namespace DenOfIz;

ComputeTest::ComputeTest( GraphicsApi *gApi, ILogicalDevice *logicalDevice ) : m_gApi( gApi ), m_logicalDevice( logicalDevice )
{
}

ComputeTest::~ComputeTest( )
{
    m_fence->Wait( );
}

int ComputeTest::Run( )
{
    m_program              = m_gApi->CreateShaderProgram( {
        ShaderDesc{ .Stage = ShaderStage::Compute, .Path = "Assets/Shaders/compute.hlsl" },
    } );
    auto programReflection = m_program->Reflect( );
    m_rootSignature        = m_logicalDevice->CreateRootSignature( programReflection.RootSignature );

    BufferDesc bufferDesc{ };
    bufferDesc.Descriptor        = ResourceDescriptor::RWBuffer;
    bufferDesc.NumBytes          = 1024 * sizeof( float );
    bufferDesc.BufferView.Stride = sizeof( float );
    bufferDesc.HeapType          = HeapType::GPU;
    bufferDesc.InitialState      = ResourceState::UnorderedAccess;
    bufferDesc.DebugName         = "ComputeTestBuffer";
    buffer                       = m_logicalDevice->CreateBufferResource( bufferDesc );

    m_resourceBindGroup = m_logicalDevice->CreateResourceBindGroup( ResourceBindGroupDesc{ .RootSignature = m_rootSignature.get( ) } );
    m_resourceBindGroup->Update( UpdateDesc( 0 ).Uav( 0, buffer.get( ) ) );

    m_inputLayout = m_logicalDevice->CreateInputLayout( { } );

    PipelineDesc pipelineDesc{ .ShaderProgram = m_program.get( ) };
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.BindPoint     = BindPoint::Compute;

    m_pipeline = m_logicalDevice->CreatePipeline( pipelineDesc );
    m_fence    = m_logicalDevice->CreateFence( );

    m_commandListPool = m_logicalDevice->CreateCommandListPool( CommandListPoolDesc{ .QueueType = QueueType::Compute } );
    auto commandList  = m_commandListPool->GetCommandLists( )[ 0 ];

    bufferDesc.Descriptor   = BitSet<ResourceDescriptor>( );
    bufferDesc.HeapType     = HeapType::GPU_CPU;
    bufferDesc.InitialState = ResourceState::CopyDst;
    bufferDesc.DebugName    = "ComputeTestReadBackBuffer";
    auto readBack           = m_logicalDevice->CreateBufferResource( bufferDesc );

    commandList->Begin( );
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
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
    executeDesc.Notify = m_fence.get( );
    commandList->Execute( executeDesc );

    m_fence->Wait( );

    float *mappedData = static_cast<float *>( readBack->MapMemory( ) );
    for ( UINT i = 0; i < 1024; i++ )
    {
        std::cout << "Index " << i << ": " << mappedData[ i ] << std::endl;
    }
    readBack->UnmapMemory( );
    return 0;
}
