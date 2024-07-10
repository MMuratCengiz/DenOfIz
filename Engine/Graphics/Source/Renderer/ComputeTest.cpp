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

ComputeTest::ComputeTest()
{
}

ComputeTest::~ComputeTest()
{
    m_fence->Wait();

    m_commandListPool.reset();
    m_fence.reset();
    buffer.reset();
    m_rootSignature.reset();
    m_inputLayout.reset();
    m_pipeline.reset();
    m_logicalDevice.reset();
    GfxGlobal::Destroy();
    GraphicsAPI::ReportLiveObjects();
}

int ComputeTest::Run()
{
    GraphicsAPI::SetAPIPreference(APIPreference{
        //            .Windows = APIPreferenceWindows::Vulkan,
    });

    m_logicalDevice = GraphicsAPI::CreateLogicalDevice(nullptr);

    auto firstDevice = m_logicalDevice->ListPhysicalDevices()[ 0 ];
    m_logicalDevice->LoadPhysicalDevice(firstDevice);

    m_program.AddShader(ShaderDesc{ .Stage = ShaderStage::Compute, .Path = "Assets/Shaders/compute.hlsl" });
    m_program.Compile();

    m_rootSignature = m_logicalDevice->CreateRootSignature(RootSignatureDesc{});
    ResourceBinding bufferBinding{};
    bufferBinding.Name       = "computeReadBack";
    bufferBinding.Binding    = 0;
    bufferBinding.Descriptor = BitSet<ResourceDescriptor>(ResourceDescriptor::RWBuffer);
    bufferBinding.Stages     = { ShaderStage::Compute };
    m_rootSignature->AddResourceBinding(bufferBinding);
    m_rootSignature->Create();

    BufferDesc bufferDesc{};
    bufferDesc.Descriptor        = ResourceDescriptor::RWBuffer;
    bufferDesc.NumBytes          = 1024 * sizeof(float);
    bufferDesc.BufferView.Stride = sizeof(float);
    bufferDesc.HeapType          = HeapType::GPU;
    bufferDesc.InitialState      = ResourceState::UnorderedAccess;
    buffer                       = m_logicalDevice->CreateBufferResource("buffer", bufferDesc);

    m_resourceBindGroup = m_logicalDevice->CreateResourceBindGroup(ResourceBindGroupDesc{ .RootSignature = m_rootSignature.get() });
    m_resourceBindGroup->BindBuffer(buffer.get());

    m_inputLayout = m_logicalDevice->CreateInputLayout({});

    PipelineDesc pipelineDesc{ .ShaderProgram = m_program };
    pipelineDesc.BlendModes    = { BlendMode::None };
    pipelineDesc.RootSignature = m_rootSignature.get();
    pipelineDesc.InputLayout   = m_inputLayout.get();
    pipelineDesc.BindPoint     = BindPoint::Compute;

    m_pipeline = m_logicalDevice->CreatePipeline(pipelineDesc);
    m_fence    = m_logicalDevice->CreateFence();

    m_commandListPool = m_logicalDevice->CreateCommandListPool(CommandListPoolDesc{ .QueueType = QueueType::Compute });
    auto commandList  = m_commandListPool->GetCommandLists()[ 0 ];

    bufferDesc.Descriptor   = BitSet<ResourceDescriptor>();
    bufferDesc.HeapType     = HeapType::GPU_CPU;
    bufferDesc.InitialState = ResourceState::CopyDst;
    auto readBack           = m_logicalDevice->CreateBufferResource("readBack", bufferDesc);

    commandList->Begin();
    commandList->BindPipeline(m_pipeline.get());
    commandList->BindResourceGroup(m_resourceBindGroup.get());
    commandList->Dispatch(1024, 1, 1);

    PipelineBarrier barrier{};
    barrier.BufferBarrier(BufferBarrierDesc{ .Resource = buffer.get(), .OldState = ResourceState::UnorderedAccess, .NewState = ResourceState::CopySrc });
    commandList->SetPipelineBarrier(barrier);

    CopyBufferRegionDesc copyDesc{};
    copyDesc.DstBuffer = readBack.get();
    copyDesc.SrcBuffer = buffer.get();
    copyDesc.NumBytes  = 1024 * sizeof(float);
    commandList->CopyBufferRegion(copyDesc);

    barrier = {};
    barrier.BufferBarrier(BufferBarrierDesc{ .Resource = buffer.get(), .OldState = ResourceState::CopySrc, .NewState = ResourceState::UnorderedAccess });
    commandList->SetPipelineBarrier(barrier);

    ExecuteDesc executeDesc{};
    executeDesc.Notify = m_fence.get();
    commandList->Execute(executeDesc);

    m_fence->Wait();

    readBack->MapMemory();
    readBack->ReadData();
    float *mappedData = reinterpret_cast<float *>(readBack->ReadData());
    for ( UINT i = 0; i < 1024; i++ )
    {
//        std::cout << "Index " << i << ": " << mappedData[ i ] << std::endl;
    }
    readBack->UnmapMemory();
    return 0;
}
