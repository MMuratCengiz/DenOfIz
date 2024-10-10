%module DenOfIzGraphics

%{
#include "DenOfIzGraphics/DenOfIzGraphics.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_unique_ptr.i>

%template(UCommandListPool) std::unique_ptr<DenOfIz::ICommandListPool>;
%template(UPipeline) std::unique_ptr<DenOfIz::IPipeline>;
%template(USwapChain) std::unique_ptr<DenOfIz::ISwapChain>;
%template(URootSignature) std::unique_ptr<DenOfIz::IRootSignature>;
%template(UInputLayout) std::unique_ptr<DenOfIz::IInputLayout>;
%template(UResourceBindGroup) std::unique_ptr<DenOfIz::IResourceBindGroup>;
%template(UFence) std::unique_ptr<DenOfIz::IFence>;
%template(USemaphore) std::unique_ptr<DenOfIz::ISemaphore>;
%template(UBufferResource) std::unique_ptr<DenOfIz::IBufferResource>;
%template(UTextureResource) std::unique_ptr<DenOfIz::ITextureResource>;
%template(USampler) std::unique_ptr<DenOfIz::ISampler>;


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
%include <DenOfIzGraphics/Data/BatchResourceCopy.h>
%include <DenOfIzGraphics/Data/Geometry.h>
%include <DenOfIzGraphics/Data/Texture.h>
%include <DenOfIzGraphics/Renderer/Assets/AssetData.h>
%include <DenOfIzGraphics/Renderer/Assets/MaterialData.h>
%include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
%include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>


%{
    namespace DenOfIz {
        typedef std::unique_ptr<ICommandListPool> UCommandListPool;
        typedef std::unique_ptr<IPipeline> UPipeline;
        typedef std::unique_ptr<ISwapChain> USwapChain;
        typedef std::unique_ptr<IRootSignature> URootSignature;
        typedef std::unique_ptr<IInputLayout> UInputLayout;
        typedef std::unique_ptr<IResourceBindGroup> UResourceBindGroup;
        typedef std::unique_ptr<IFence> UFence;
        typedef std::unique_ptr<ISemaphore> USemaphore;
        typedef std::unique_ptr<IBufferResource> UBufferResource;
        typedef std::unique_ptr<ITextureResource> UTextureResource;
        typedef std::unique_ptr<ISampler> USampler;
    }
%}