%module(directors="1") DenOfIzGraphics
#pragma SWIG nowarn=320

%{
#include "DenOfIzGraphics/DenOfIzGraphics.h"
%}
#define DZ_API
%warnfilter(516) InteropArray;

%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "carrays.i"
%include "cpointer.i"

%apply void *VOID_INT_PTR { void * }

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
%ignore DenOfIz::GraphicsWindowHandle::CreateViaSDLWindowID;
%ignore TWindowHandle;
%ignore DenOfIz::GraphicsWindowHandle::GetNativeHandle;

%ignore DenOfIz::RenderingAttachmentDesc::ClearColor;
%ignore DenOfIz::RenderingAttachmentDesc::ClearDepthStencil;
// ByteVector ignores:
%ignore DenOfIz::BatchResourceCopy::CreateUniformBuffer;
%ignore DenOfIz::IBufferResource::MapMemory;
%ignore DenOfIz::IBufferResource::InitialState;
%ignore DenOfIz::IBufferResource::Data;
%ignore DenOfIz::IBufferResource::NumBytes;
%ignore DenOfIz::IResourceBindGroup::SetRootConstants;

// Fix InteropString/InteropArray:
%ignore DenOfIz::InteropString::InteropString(InteropString &&);
%ignore DenOfIz::InteropString::operator DenOfIz::InteropString;
%ignore DenOfIz::InteropString::operator=;
%ignore DenOfIz::InteropArray::operator=;
%ignore DenOfIz::InteropArray::Data;
%ignore DenOfIz::InteropArray::EmplaceElement;
%ignore DenOfIz::InteropArray::MemCpy;

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

%feature("director") DenOfIz::NodeExecutionCallback;
%feature("director") DenOfIz::PresentExecutionCallback;

%include <DenOfIzGraphics/Utilities/Interop.h>
%include <DenOfIzGraphics/Utilities/Common_Macro.h>
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
//%include <DenOfIzGraphics/DenOfIzGraphics.h>

// -- BitSet definitions
// Convenience typedefs
typedef DenOfIz::BitSet<DenOfIz::ResourceState> ResourceStateBitSet;
typedef DenOfIz::BitSet<DenOfIz::BuildDesc> BuildDescBitSet;
typedef DenOfIz::BitSet<DenOfIz::ResourceDescriptor> ResourceDescriptorBitSet;
// Instantiations
%template(ResourceStateBitSet) DenOfIz::BitSet<DenOfIz::ResourceState>;
%template(BuildDescBitSet) DenOfIz::BitSet<DenOfIz::BuildDesc>;
%template(ResourceDescriptorBitSet) DenOfIz::BitSet<DenOfIz::ResourceDescriptor>;
// --

// -- Interop Array definitons
// Convenience typedefs
typedef DenOfIz::InteropArray<DenOfIz::AccelerationStructureGeometryDesc> AccelerationStructureGeometryDescArray;
typedef DenOfIz::InteropArray<DenOfIz::AccelerationStructureInstanceDesc> AccelerationStructureInstanceDescArray;
typedef DenOfIz::InteropArray<DenOfIz::BufferBarrierDesc> BufferBarrierDescArray;
typedef DenOfIz::InteropArray<DenOfIz::GeometryVertexData> GeometryVertexDataArray;
typedef DenOfIz::InteropArray<DenOfIz::ICommandList*> ICommandListArray;
typedef DenOfIz::InteropArray<DenOfIz::InputGroupDesc> InputGroupDescArray;
typedef DenOfIz::InteropArray<DenOfIz::InputLayoutElementDesc> InputLayoutElementDescArray;
typedef DenOfIz::InteropArray<DenOfIz::InteropString> InteropStringArray;
typedef DenOfIz::InteropArray<DenOfIz::ISemaphore*> ISemaphoreArray;
typedef DenOfIz::InteropArray<DenOfIz::MemoryBarrierDesc> MemoryBarrierDescArray;
typedef DenOfIz::InteropArray<DenOfIz::NodeResourceUsageDesc> NodeResourceUsageDescArray;
typedef DenOfIz::InteropArray<DenOfIz::PhysicalDevice> PhysicalDeviceArray;
typedef DenOfIz::InteropArray<DenOfIz::ReflectionResourceField> ReflectionResourceFieldArray;
typedef DenOfIz::InteropArray<DenOfIz::RenderingAttachmentDesc> RenderingAttachmentDescArray;
typedef DenOfIz::InteropArray<DenOfIz::RenderTargetDesc> RenderTargetDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ResourceBindingDesc> ResourceBindingDescArray;
typedef DenOfIz::InteropArray<DenOfIz::RootConstantResourceBindingDesc> RootConstantResourceBindingDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ShaderDesc> ShaderDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ShaderStage> ShaderStageArray;
typedef DenOfIz::InteropArray<DenOfIz::StaticSamplerDesc> StaticSamplerDescArray;
typedef DenOfIz::InteropArray<DenOfIz::TextureBarrierDesc> TextureBarrierDescArray;
typedef DenOfIz::InteropArray<unsigned int> UnsignedIntArray;
typedef DenOfIz::InteropArray<unsigned char> ByteArray;
// Instantiations:
%template(AccelerationStructureGeometryDescArray) DenOfIz::InteropArray<DenOfIz::AccelerationStructureGeometryDesc>;
%template(AccelerationStructureInstanceDescArray) DenOfIz::InteropArray<DenOfIz::AccelerationStructureInstanceDesc>;
%template(BufferBarrierDescArray) DenOfIz::InteropArray<DenOfIz::BufferBarrierDesc>;
%template(GeometryVertexDataArray) DenOfIz::InteropArray<DenOfIz::GeometryVertexData>;
%template(ICommandListArray) DenOfIz::InteropArray<DenOfIz::ICommandList*>;
%template(InputGroupDescArray) DenOfIz::InteropArray<DenOfIz::InputGroupDesc>;
%template(InputLayoutElementDescArray) DenOfIz::InteropArray<DenOfIz::InputLayoutElementDesc>;
%template(InteropStringArray) DenOfIz::InteropArray<DenOfIz::InteropString>;
%template(ISemaphoreArray) DenOfIz::InteropArray<DenOfIz::ISemaphore*>;
%template(MemoryBarrierDescArray) DenOfIz::InteropArray<DenOfIz::MemoryBarrierDesc>;
%template(NodeResourceUsageDescArray) DenOfIz::InteropArray<DenOfIz::NodeResourceUsageDesc>;
%template(PhysicalDeviceArray) DenOfIz::InteropArray<DenOfIz::PhysicalDevice>;
%template(ReflectionResourceFieldArray) DenOfIz::InteropArray<DenOfIz::ReflectionResourceField>;
%template(RenderingAttachmentDescArray) DenOfIz::InteropArray<DenOfIz::RenderingAttachmentDesc>;
%template(RenderTargetDescArray) DenOfIz::InteropArray<DenOfIz::RenderTargetDesc>;
%template(ResourceBindingDescArray) DenOfIz::InteropArray<DenOfIz::ResourceBindingDesc>;
%template(RootConstantResourceBindingDescArray) DenOfIz::InteropArray<DenOfIz::RootConstantResourceBindingDesc>;
%template(ShaderDescArray) DenOfIz::InteropArray<DenOfIz::ShaderDesc>;
%template(ShaderStageArray) DenOfIz::InteropArray<DenOfIz::ShaderStage>;
%template(StaticSamplerDescArray) DenOfIz::InteropArray<DenOfIz::StaticSamplerDesc>;
%template(TextureBarrierDescArray) DenOfIz::InteropArray<DenOfIz::TextureBarrierDesc>;
%template(UnsignedIntArray) DenOfIz::InteropArray<unsigned int>;
%template(ByteArray) DenOfIz::InteropArray<unsigned char>;
// --