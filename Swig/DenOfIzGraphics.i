%module(directors="1") DenOfIzGraphics

%{
#include "DenOfIzGraphics/DenOfIzGraphics.h"
%}

%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "carrays.i"
%include "cpointer.i"

%ignore DenOfIz::RenderGraphInternal::NodeExecutionContext;
%ignore DenOfIz::RenderGraphInternal::GraphNode;
%ignore DenOfIz::RenderGraphInternal::PresentContext;
%ignore DenOfIz::RenderGraphInternal::ResourceLockedState;
%ignore DenOfIz::RenderGraphInternal::ResourceLocking;
%ignore DenOfIz::CompiledShader;
%ignore DenOfIz::CompiledShaders;
%ignore DenOfIz::ShaderProgram::GetCompiledShaders;
%ignore DenOfIz::BatchResourceCopy::SyncOp;
%ignore DenOfIz::GraphicsWindowHandle::CreateFromSDLWindow;
%ignore TWindowHandle;
%ignore DenOfIz::GraphicsWindowHandle::GetNativeHandle;

// Array ignores that do not translate well to C#, exposed via functions instead
%ignore DenOfIz::Semaphores::Array;
%ignore DenOfIz::ShaderStages::Array;
%ignore DenOfIz::CommandLists::Array;
%ignore DenOfIz::NodeDependencies::Array;
%ignore DenOfIz::ShaderDefines::Array;
%ignore DenOfIz::RenderingAttachmentDesc::ClearColor;
%ignore DenOfIz::RenderingAttachmentDesc::ClearDepthStencil;
// ByteVector ignores:
%ignore DenOfIz::CopyToGpuBufferDesc::Data;
%ignore DenOfIz::CopyToGpuBufferDesc::NumBytes;
%ignore DenOfIz::CopyDataToTextureDesc::Data;
%ignore DenOfIz::CopyDataToTextureDesc::NumBytes;
%ignore DenOfIz::BatchResourceCopy::CreateUniformBuffer;
%ignore DenOfIz::IBufferResource::MapMemory;
%ignore DenOfIz::IBufferResource::InitialState;
%ignore DenOfIz::IBufferResource::Data;
%ignore DenOfIz::IBufferResource::NumBytes;
%ignore DenOfIz::IResourceBindGroup::SetRootConstants;


// BitSet ignores:
%ignore DenOfIz::BitSet::operator DenOfIz::ResourceState;
%ignore DenOfIz::BitSet::operator DenOfIz::BuildDesc;
%ignore DenOfIz::BitSet::operator DenOfIz::ResourceDescriptor;
%ignore DenOfIz::BitSet::operator|;
%ignore DenOfIz::BitSet::operator|=;
%ignore DenOfIz::BitSet::operator&;
%ignore DenOfIz::BitSet::operator&=;
%ignore DenOfIz::BitSet::operator=;
%ignore DenOfIz::BitSet::operator==;
%ignore DenOfIz::BitSet::operator!=;
%ignore DenOfIz::BitSet::operator~;
%ignore DenOfIz::BitSet::None;
%ignore DenOfIz::BitSet::All;
%ignore DenOfIz::BitSet::Any;

%template(GeometryVertexDataVector) std::vector<DenOfIz::GeometryVertexData>;
%template(IndexVector) std::vector<uint32_t>;
%template(ByteVector) std::vector<unsigned char>;

%feature("director") DenOfIz::NodeExecutionCallback;
%feature("director") DenOfIz::PresentExecutionCallback;

%include <DenOfIzGraphics/Utilities/BitSet.h>
%include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
%include <DenOfIzGraphics/Backends/Interface/CommonData.h>
%include <DenOfIzGraphics/Backends/Interface/ReflectionData.h>
%include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
%include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
%include <DenOfIzGraphics/Backends/Interface/IShader.h>
%include <DenOfIzGraphics/Backends/Interface/PipelineBarrierDesc.h>
%include <DenOfIzGraphics/Backends/Interface/IFence.h>
%include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>
%include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>
%include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
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
%include <DenOfIzGraphics/Data/BatchResourceCopy.h>
%include <DenOfIzGraphics/Renderer/Assets/MaterialData.h>
%include <DenOfIzGraphics/Renderer/Assets/AssetData.h>
%include <DenOfIzGraphics/Renderer/Common/CommandListRing.h>
%include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>

typedef DenOfIz::BitSet<DenOfIz::ResourceState> ResourceStateBitSet;
%template(ResourceStateBitSet) DenOfIz::BitSet<DenOfIz::ResourceState>;

typedef DenOfIz::BitSet<DenOfIz::BuildDesc> BuildDescBitSet;
%template(BuildDescBitSet) DenOfIz::BitSet<DenOfIz::BuildDesc>;

typedef DenOfIz::BitSet<DenOfIz::ResourceDescriptor> ResourceDescriptorBitSet;
%template(ResourceDescriptorBitSet) DenOfIz::BitSet<DenOfIz::ResourceDescriptor>;
