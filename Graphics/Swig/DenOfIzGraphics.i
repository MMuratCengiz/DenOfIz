%module DenOfIzGraphics

%{
#include "DenOfIzGraphics/DenOfIzGraphics.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_unique_ptr.i>

%unique_ptr(DenOfIz::ICommandListPool)
%unique_ptr(DenOfIz::IPipeline)
%unique_ptr(DenOfIz::ISwapChain)
%unique_ptr(DenOfIz::IRootSignature)
%unique_ptr(DenOfIz::IInputLayout)
%unique_ptr(DenOfIz::IResourceBindGroup)
%unique_ptr(DenOfIz::IFence)
%unique_ptr(DenOfIz::ISemaphore)
%unique_ptr(DenOfIz::IBufferResource)
%unique_ptr(DenOfIz::ITextureResource)
%unique_ptr(DenOfIz::ISampler)


%include <DenOfIzGraphics/Utilities/BitSet.h>
%include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
%include <DenOfIzGraphics/Backends/Interface/IShader.h>
%include <DenOfIzGraphics/Backends/Interface/CommonData.h>
%include <DenOfIzGraphics/Backends/Interface/ReflectionData.h>
%include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
%include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
%include <DenOfIzGraphics/Backends/Interface/PipelineBarrierDesc.h>
%include <DenOfIzGraphics/Backends/Interface/IFence.h>
%include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>
%include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>
%include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
%include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
%include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
%include <DenOfIzGraphics/Backends/Interface/IRayTracingAccelerationStructure.h>
%include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
%include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
%include <DenOfIzGraphics/Backends/Interface/ISwapChain.h>
%include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
%include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
%include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
%include <DenOfIzGraphics/Backends/GraphicsApi.h>
%include <DenOfIzGraphics/Data/Geometry.h>
%include <DenOfIzGraphics/Data/Texture.h>
%include <DenOfIzGraphics/Renderer/Assets/MaterialData.h>
%include <DenOfIzGraphics/Renderer/Assets/AssetData.h>
%include <DenOfIzGraphics/Data/BatchResourceCopy.h>
%include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
%include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>


