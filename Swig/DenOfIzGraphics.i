%module(directors="1") DenOfIzGraphics
#pragma SWIG nowarn=320
#pragma SWIG nowarn=401
#pragma SWIG nowarn=503
#pragma SWIG nowarn=509 // overloaded method handling
#pragma SWIG nowarn=314 // class/struct redefinition


%{
#include "DenOfIzGraphics/DenOfIzGraphics.h"
%}
#define DZ_API

%feature("director") DenOfIz::EventCallback;
%feature("director") DenOfIz::KeyboardEventCallback;
%feature("director") DenOfIz::MouseMotionEventCallback;
%feature("director") DenOfIz::MouseButtonEventCallback;
%feature("director") DenOfIz::MouseWheelEventCallback;
%feature("director") DenOfIz::WindowEventCallback;
%feature("director") DenOfIz::ControllerAxisEventCallback;
%feature("director") DenOfIz::ControllerButtonEventCallback;
%feature("director") DenOfIz::QuitEventCallback;
%feature("director") DenOfIz::IContentRenderer;

%warnfilter(516) InteropArray;

// DirectX namespace is for internal use only
namespace DirectX {}
using namespace DirectX;

%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "carrays.i"
%include "cpointer.i"

#if defined(SWIGCSHARP)
%include "Targets/CSharp/DenOfIzGraphics_CSharp.i"
#endif
#if defined(SWIGJAVA)
%include "Targets/Java/DenOfIzGraphics_Java.i"
#endif

%apply void *VOID_INT_PTR { void * }
%apply unsigned char INPUT[]  {unsigned char *inputBytes}

%ignore std::chrono;
%ignore __parse_int;

%ignore IDxcBlob;
%ignore DenOfIz::RenderGraphInternal::NodeExecutionContext;
%ignore DenOfIz::RenderGraphInternal::GraphNode;
%ignore DenOfIz::RenderGraphInternal::PresentContext;
%ignore DenOfIz::RenderGraphInternal::ResourceLockedState;
%ignore DenOfIz::RenderGraphInternal::ResourceLocking;

%ignore DenOfIz::Texture::Texture( const std::string & );
%ignore DenOfIz::Texture::StreamMipData;
%ignore DenOfIz::MipStreamCallback;

%ignore DenOfIz::CompiledShaders;
%ignore DenOfIz::ShaderProgram::GetCompiledShaders;
%ignore DenOfIz::ReflectionState;
%ignore DenOfIz::BatchResourceCopy::SyncOp;
%ignore DenOfIz::GraphicsWindowHandle::CreateFromSDLWindow;
%ignore DenOfIz::GraphicsWindowHandle::CreateViaSDLWindowID;
%ignore TWindowHandle;
%ignore DenOfIz::GraphicsWindowHandle::GetNativeHandle;
%ignore DenOfIz::ResourceState;

%ignore DenOfIz::RenderingAttachmentDesc::ClearColor;
%ignore DenOfIz::RenderingAttachmentDesc::ClearDepthStencil;
// ByteVector ignores:
%ignore DenOfIz::BatchResourceCopy::CreateUniformBuffer;
%ignore DenOfIz::IBufferResource::MapMemory;
%ignore DenOfIz::IBufferResource::InitialState;
%ignore DenOfIz::IBufferResource::Data;
%ignore DenOfIz::IBufferResource::NumBytes;
%ignore DenOfIz::IResourceBindGroup::SetRootConstants;

// Font/Text renderer ignores:
%ignore DenOfIz::Font::m_ftLibrary;
%ignore DenOfIz::Font::m_face;
%ignore DenOfIz::Font::m_glyphs;
%ignore DenOfIz::Font::FTFace;
%ignore DenOfIz::FontDesc::FontAsset;
%ignore DenOfIz::TextRenderer::m_projectionMatrix;
%ignore DenOfIz::BundleHeader::BundleHeaderMagic;

// Animation ignores:
%ignore DenOfIz::Internal::AnimationState;
%ignore DenOfIz::Internal::BlendingState;
%ignore DenOfIz::AnimationStateManager::m_skeleton;
%ignore DenOfIz::AnimationStateManager::m_animations;
%ignore DenOfIz::AnimationStateManager::m_currentAnimation;
%ignore DenOfIz::AnimationStateManager::m_blendingState;
%ignore DenOfIz::AnimationStateManager::m_localTransforms;
%ignore DenOfIz::AnimationStateManager::m_modelTransforms;
%ignore DenOfIz::AnimationStateManager::ConvertToOzzAnimation;
%ignore DenOfIz::AnimationStateManager::SampleAnimation;
%ignore DenOfIz::AnimationStateManager::UpdateBlending;
%ignore DenOfIz::AnimationStateManager::GetJointLocalTransform;
%ignore DenOfIz::AnimationStateManager::ToOzzTranslation;
%ignore DenOfIz::AnimationStateManager::ToOzzRotation;
%ignore DenOfIz::AnimationStateManager::ToOzzScale;
%ignore DenOfIz::AnimationStateManager::FromOzzTranslation;
%ignore DenOfIz::AnimationStateManager::FromOzzRotation;
%ignore DenOfIz::AnimationStateManager::FromOzzScale;

// Bundle ignores:
%ignore DenOfIz::Bundle::m_bundleFile;

%ignore DenOfIz::ThorVGPicture::Load( uint32_t *, uint32_t, uint32_t ) const;
%ignore DenOfIz::ThorVGPicture::Load( uint32_t *, uint32_t, uint32_t, bool ) const;
%ignore DenOfIz::BinaryWriter::BinaryWriter(std::ostream *);
%ignore DenOfIz::BinaryWriter::BinaryWriter(std::ostream *, const BinaryWriterDesc &);
%ignore DenOfIz::BinaryReader::BinaryReader(std::istream *);
%ignore DenOfIz::BinaryReader::BinaryReader(std::istream *, const BinaryReaderDesc &);

// Fix InteropString/InteropArray:
%ignore DenOfIz::InteropString::InteropString(const char *, const size_t);
%ignore DenOfIz::InteropString::InteropString(InteropString &&);
%ignore DenOfIz::InteropArray::InteropArray(std::initializer_list<T>);
%ignore DenOfIz::InteropString::operator DenOfIz::InteropString;
%ignore DenOfIz::InteropString::operator=;
%ignore DenOfIz::InteropArray::operator=;
%ignore DenOfIz::InteropArray::Data;
%ignore DenOfIz::InteropArray::EmplaceElement;
%ignore DenOfIz::InteropArray::MemCpy;

// Asset serde ignores:
%ignore DenOfIz::AssetHeader::operator=;
%ignore DenOfIz::AssetUri::operator=;

// BitSet ignores:
%ignore DenOfIz::BitSet::operator DenOfIz::ResourceUsage;
%ignore DenOfIz::BitSet::operator DenOfIz::BuildDesc;
%ignore DenOfIz::BitSet::operator DenOfIz::ResourceDescriptor;
%ignore DenOfIz::BitSet::operator DenOfIz::ASBuildFlags;
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

%ignore DenOfIz::Window::GetSDLWindow;
%ignore DenOfIz::InputSystem::GetSDLInputSystem;
%ignore DenOfIz::Time::OnEachSecond;
%ignore DenOfIz::StepTimer::OnEachSecond;
%ignore DenOfIz::Font::MsdfPixelRange;

// First include the math and interop utilities
%include <DenOfIzGraphics/Utilities/Interop.h>
%include <DenOfIzGraphics/Utilities/InteropMath.h>
%include <DenOfIzGraphics/Utilities/InteropUtilities.h>
%include <DenOfIzGraphics/Utilities/Common_Macro.h>
%include <DenOfIzGraphics/Assets/FileSystem/FSConfig.h>
%include <DenOfIzGraphics/Utilities/Engine.h>
%include <DenOfIzGraphics/Assets/Serde/Asset.h>
%include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
%include <DenOfIzGraphics/Utilities/BitSet.h>

// Then include the backend interfaces
%include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
%include <DenOfIzGraphics/Backends/Interface/CommonData.h>
%include <DenOfIzGraphics/Backends/Interface/RayTracing/RayTracingData.h>
%include <DenOfIzGraphics/Backends/Interface/ReflectionData.h>
%include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
%include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
%include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
%include <DenOfIzGraphics/Backends/Interface/RayTracing/IBottomLevelAS.h>
%include <DenOfIzGraphics/Backends/Interface/RayTracing/ITopLevelAS.h>
%include <DenOfIzGraphics/Backends/Interface/PipelineBarrierDesc.h>
%include <DenOfIzGraphics/Backends/Interface/IFence.h>
%include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>
%include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>
%include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
%include <DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h>
%include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderLocalData.h>

// Required in the middle here since ShaderProgram depends on ShaderAsset
%include <DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h>
%include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h>
%include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>

%include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
%include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
// Dependency on IPipeline requires it to be separated from other RayTracing includes
%include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h>

%newobject DenOfIz::ILogicalDevice::CreateCommandQueue;
%newobject DenOfIz::ILogicalDevice::CreateCommandListPool;
%newobject DenOfIz::ILogicalDevice::CreatePipeline;
%newobject DenOfIz::ILogicalDevice::CreateSwapChain;
%newobject DenOfIz::ILogicalDevice::CreateRootSignature;
%newobject DenOfIz::ILogicalDevice::CreateInputLayout;
%newobject DenOfIz::ILogicalDevice::CreateResourceBindGroup;
%newobject DenOfIz::ILogicalDevice::CreateFence;
%newobject DenOfIz::ILogicalDevice::CreateSemaphore;
%newobject DenOfIz::ILogicalDevice::CreateBufferResource;
%newobject DenOfIz::ILogicalDevice::CreateTextureResource;
%newobject DenOfIz::ILogicalDevice::CreateSampler;
%newobject DenOfIz::ILogicalDevice::CreateTopLevelAS;
%newobject DenOfIz::ILogicalDevice::CreateBottomLevelAS;
%newobject DenOfIz::ILogicalDevice::CreateShaderBindingTable;
%newobject DenOfIz::ILogicalDevice::CreateLocalRootSignature;
%newobject DenOfIz::ILogicalDevice::CreateShaderLocalData;

%ignore DenOfIz::RenderingDesc::RTAttachments; // Uses performant array instead
%include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
%include <DenOfIzGraphics/Backends/Interface/ICommandQueue.h>

%include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
%include <DenOfIzGraphics/Backends/Interface/ISwapChain.h>
%include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>

%newobject DenOfIz::GraphicsApi::CreateLogicalDevice;
%newobject DenOfIz::GraphicsApi::CreateAndLoadOptimalLogicalDevice;

%include <DenOfIzGraphics/Backends/GraphicsApi.h>

%include <DenOfIzGraphics/Renderer/Sync/FrameSync.h>
%include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>

// Binary Reader
%include <DenOfIzGraphics/Assets/Stream/BinaryContainer.h>
%include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
%include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>

// Texture Asset
%include <DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h>

// Material Asset
%include <DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Material/MaterialAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h>

// Physics Asset
%include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Physics/PhysicsAssetWriter.h>

// Shader Asset
%include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h>

// Mesh Asset
%include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h>

// Font system
%include <DenOfIzGraphics/Assets/Serde/Font/FontAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h>

%include <DenOfIzGraphics/Assets/Font/Font.h>
%include <DenOfIzGraphics/Assets/Font/FontLibrary.h>
%include <DenOfIzGraphics/Assets/Font/TextLayout.h>
%include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
%include <DenOfIzGraphics/Utilities/FrameDebugRenderer.h>

// Animation system
%include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h>

%include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
%include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>
%include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>

%include <DenOfIzGraphics/Animation/OzzAnimation.h>
%include <DenOfIzGraphics/Animation/AnimationStateManager.h>

%include <DenOfIzGraphics/Data/Geometry.h>
%include <DenOfIzGraphics/Data/AlignedDataWriter.h>
%include <DenOfIzGraphics/Data/BatchResourceCopy.h>
%include <DenOfIzGraphics/Data/Texture.h>

// Bundle system
%include <DenOfIzGraphics/Assets/Bundle/Bundle.h>
%include <DenOfIzGraphics/Assets/Bundle/BundleManager.h>

%include <DenOfIzGraphics/Utilities/Time.h>
%include <DenOfIzGraphics/Utilities/StepTimer.h>

// Vector Graphics includes
%include <DenOfIzGraphics/Assets/Vector2d/QuadRenderer.h>
%include <DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h>

%include <DenOfIzGraphics/Assets/Import/ImporterCommon.h>
%include <DenOfIzGraphics/Assets/Import/AssetScanner.h>
%include <DenOfIzGraphics/Assets/Import/AssimpImporter.h>
%include <DenOfIzGraphics/Assets/Import/FontImporter.h>
%include <DenOfIzGraphics/Assets/Import/ShaderImporter.h>
%include <DenOfIzGraphics/Assets/Import/TextureImporter.h>
%include <DenOfIzGraphics/Assets/Import/VGImporter.h>

%include "DenOfIzGraphics_Input.i"

%include <DenOfIzGraphics/UI/IClayContext.h>
%include <DenOfIzGraphics/UI/ClayData.h>
%include <DenOfIzGraphics/UI/Clay.h>
%include <DenOfIzGraphics/UI/UIShapes.h>
%include <DenOfIzGraphics/UI/IContentRenderer.h>

// UI Widgets
%include <DenOfIzGraphics/UI/Widgets/Widget.h>
%include <DenOfIzGraphics/UI/Widgets/CheckboxWidget.h>
%include <DenOfIzGraphics/UI/Widgets/SliderWidget.h>
%include <DenOfIzGraphics/UI/Widgets/TextFieldWidget.h>
%include <DenOfIzGraphics/UI/Widgets/DropdownWidget.h>
%include <DenOfIzGraphics/UI/Widgets/ColorPickerWidget.h>
%include <DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h>
%include <DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h>

// -- BitSet definitions
// Convenience typedefs
typedef DenOfIz::BitSet<DenOfIz::ResourceUsage> ResourceUsageBitSet;
typedef DenOfIz::BitSet<DenOfIz::BuildDesc> BuildDescBitSet;
typedef DenOfIz::BitSet<DenOfIz::ResourceDescriptor> ResourceDescriptorBitSet;
typedef DenOfIz::BitSet<DenOfIz::ASBuildFlags> ASBuildFlagsBitSet;
typedef DenOfIz::BitSet<DenOfIz::KeyMod> KeyModBitSet;
// Instantiations
%template(ResourceUsageBitSet) DenOfIz::BitSet<DenOfIz::ResourceUsage>;
%template(BuildDescBitSet) DenOfIz::BitSet<DenOfIz::BuildDesc>;
%template(ResourceDescriptorBitSet) DenOfIz::BitSet<DenOfIz::ResourceDescriptor>;
%template(ASBuildFlagsBitSet) DenOfIz::BitSet<DenOfIz::ASBuildFlags>;
%template(KeyModBitSet) DenOfIz::BitSet<DenOfIz::KeyMod>;
// --

// -- Interop Array definitons
// Convenience typedefs
typedef DenOfIz::InteropArray<DenOfIz::ASGeometryDesc> ASGeometryDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ASInstanceDesc> ASInstanceDescArray;
typedef DenOfIz::InteropArray<DenOfIz::BufferBarrierDesc> BufferBarrierDescArray;
typedef DenOfIz::InteropArray<DenOfIz::GeometryVertexData> GeometryVertexDataArray;
typedef DenOfIz::InteropArray<DenOfIz::ICommandList*> ICommandListArray;
typedef DenOfIz::InteropArray<DenOfIz::InputGroupDesc> InputGroupDescArray;
typedef DenOfIz::InteropArray<DenOfIz::InputLayoutElementDesc> InputLayoutElementDescArray;
typedef DenOfIz::InteropArray<DenOfIz::InteropString> InteropStringArray;
typedef DenOfIz::InteropArray<DenOfIz::ISemaphore*> ISemaphoreArray;
typedef DenOfIz::InteropArray<DenOfIz::MemoryBarrierDesc> MemoryBarrierDescArray;
typedef DenOfIz::InteropArray<DenOfIz::PhysicalDevice> PhysicalDeviceArray;
typedef DenOfIz::InteropArray<DenOfIz::ReflectionResourceField> ReflectionResourceFieldArray;
typedef DenOfIz::InteropArray<DenOfIz::RenderingAttachmentDesc> RenderingAttachmentDescArray;
typedef DenOfIz::InteropArray<DenOfIz::RenderTargetDesc> RenderTargetDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ResourceBindingDesc> ResourceBindingDescArray;
typedef DenOfIz::InteropArray<DenOfIz::RootConstantResourceBindingDesc> RootConstantResourceBindingDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ShaderStageDesc> ShaderStageDescArray;
typedef DenOfIz::InteropArray<DenOfIz::ShaderStage> ShaderStageArray;
typedef DenOfIz::InteropArray<DenOfIz::StaticSamplerDesc> StaticSamplerDescArray;
typedef DenOfIz::InteropArray<DenOfIz::TextureBarrierDesc> TextureBarrierDescArray;
typedef DenOfIz::InteropArray<unsigned int> UnsignedIntArray;
typedef DenOfIz::InteropArray<DenOfIz::Byte> ByteArray;
typedef DenOfIz::InteropArray<DenOfIz::GlyphVertex> GlyphVertexArray;
typedef DenOfIz::InteropArray<DenOfIz::AssetEntry> AssetEntryArray;
typedef DenOfIz::InteropArray<DenOfIz::ShaderRecordDebugData> ShaderRecordDebugDataArray;
typedef DenOfIz::InteropArray<DenOfIz::ITextureResource*> ITextureResourceArray;
typedef DenOfIz::InteropArray<DenOfIz::ILocalRootSignature*> ILocalRootSignatureArray;
typedef DenOfIz::InteropArray<DenOfIz::LocalRootSignatureDesc> LocalRootSignatureDescArray;
typedef DenOfIz::InteropArray<DenOfIz::BindlessSlot> BindlessSlotArray;
typedef DenOfIz::InteropArray<DenOfIz::BindlessResourceDesc> BindlessResourceDescArray;
typedef DenOfIz::InteropArray<DenOfIz::AnimationClip> AnimationClipArray;
typedef DenOfIz::InteropArray<DenOfIz::JointAnimTrack> JointAnimTrackArray;
typedef DenOfIz::InteropArray<DenOfIz::Joint> JointArray;
typedef DenOfIz::InteropArray<DenOfIz::MorphAnimTrack> MorphAnimTrackArray;
typedef DenOfIz::InteropArray<DenOfIz::MorphKeyframe> MorphKeyframeArray;
typedef DenOfIz::InteropArray<DenOfIz::MorphTarget> MorphTargetArray;
typedef DenOfIz::InteropArray<DenOfIz::MorphTargetDelta> MorphTargetDeltaArray;
typedef DenOfIz::InteropArray<DenOfIz::PositionKey> PositionKeyArray;
typedef DenOfIz::InteropArray<DenOfIz::RotationKey> RotationKeyArray;
typedef DenOfIz::InteropArray<DenOfIz::ScaleKey> ScaleKeyArray;
typedef DenOfIz::InteropArray<DenOfIz::AssetType> AssetTypeArray;
typedef DenOfIz::InteropArray<DenOfIz::AssetUri> AssetUriArray;
typedef DenOfIz::InteropArray<DenOfIz::Bundle*> BundlePointerArray;
typedef DenOfIz::InteropArray<DenOfIz::ColorFormat> ColorFormatArray;
typedef DenOfIz::InteropArray<DenOfIz::CompiledShaderStage*> CompiledShaderStagePointerArray;
typedef DenOfIz::BitSet<DenOfIz::GeometryFlags> GeometryFlagsBitSet;
typedef DenOfIz::InteropArray<DenOfIz::BlendingJobLayerDesc> BlendingJobLayerDescArray;
typedef DenOfIz::InteropArray<DenOfIz::BoundingVolume> BoundingVolumeArray;
typedef DenOfIz::InteropArray<DenOfIz::FontGlyph> FontGlyphArray;
typedef DenOfIz::InteropArray<DenOfIz::HitGroupDesc> HitGroupDescArray;
typedef DenOfIz::InteropArray<DenOfIz::MeshVertex> MeshVertexArray;
typedef DenOfIz::InteropArray<DenOfIz::PhysicsCollider> PhysicsColliderArray;
typedef DenOfIz::InteropArray<DenOfIz::ResourceBindingSlot> ResourceBindingSlotArray;
typedef DenOfIz::InteropArray<DenOfIz::ShaderStageAsset> ShaderStageAssetArray;
typedef DenOfIz::InteropArray<DenOfIz::SubMeshData> SubMeshDataArray;
typedef DenOfIz::InteropArray<DenOfIz::TextureMip> TextureMipArray;
typedef DenOfIz::InteropArray<DenOfIz::ThreadGroupInfo> ThreadGroupInfoArray;
typedef DenOfIz::InteropArray<DenOfIz::TransitionBufferDesc> TransitionBufferDescArray;
typedef DenOfIz::InteropArray<DenOfIz::TransitionTextureDesc> TransitionTextureDescArray;
typedef DenOfIz::InteropArray<DenOfIz::UserProperty> UserPropertyArray;
typedef DenOfIz::InteropArray<DenOfIz::UVChannel> UVChannelArray;
typedef DenOfIz::InteropArray<unsigned short> UnsignedShortArray;
typedef DenOfIz::InteropArray<float> FloatArray;
typedef DenOfIz::InteropArray<DenOfIz::InteropArray<float>> FloatArrayArray;
typedef DenOfIz::InteropArray<DenOfIz::CompiledShader> CompiledShaderArray;
typedef DenOfIz::InteropArray<unsigned char> UnsignedCharArray;
typedef DenOfIz::InteropArray<unsigned char> ByteArray;
typedef DenOfIz::InteropArray<bool> BoolArray;
typedef DenOfIz::InteropArray<int> IntArray;
typedef DenOfIz::InteropArray<DenOfIz::ThorVGColorStop> ThorVGColorStopArray;

// Instantiations:
%template(ASGeometryDescArray) DenOfIz::InteropArray<DenOfIz::ASGeometryDesc>;
%template(ASInstanceDescArray) DenOfIz::InteropArray<DenOfIz::ASInstanceDesc>;
%template(BufferBarrierDescArray) DenOfIz::InteropArray<DenOfIz::BufferBarrierDesc>;
%template(GeometryVertexDataArray) DenOfIz::InteropArray<DenOfIz::GeometryVertexData>;
%template(ICommandListArray) DenOfIz::InteropArray<DenOfIz::ICommandList*>;
%template(InputGroupDescArray) DenOfIz::InteropArray<DenOfIz::InputGroupDesc>;
%template(InputLayoutElementDescArray) DenOfIz::InteropArray<DenOfIz::InputLayoutElementDesc>;
%template(InteropStringArray) DenOfIz::InteropArray<DenOfIz::InteropString>;
%template(ISemaphoreArray) DenOfIz::InteropArray<DenOfIz::ISemaphore*>;
%template(MemoryBarrierDescArray) DenOfIz::InteropArray<DenOfIz::MemoryBarrierDesc>;
%template(PhysicalDeviceArray) DenOfIz::InteropArray<DenOfIz::PhysicalDevice>;
%template(ReflectionResourceFieldArray) DenOfIz::InteropArray<DenOfIz::ReflectionResourceField>;
%template(RenderingAttachmentDescArray) DenOfIz::InteropArray<DenOfIz::RenderingAttachmentDesc>;
%template(RenderTargetDescArray) DenOfIz::InteropArray<DenOfIz::RenderTargetDesc>;
%template(ResourceBindingDescArray) DenOfIz::InteropArray<DenOfIz::ResourceBindingDesc>;
%template(RootConstantResourceBindingDescArray) DenOfIz::InteropArray<DenOfIz::RootConstantResourceBindingDesc>;
%template(ShaderStageDescArray) DenOfIz::InteropArray<DenOfIz::ShaderStageDesc>;
%template(ShaderStageArray) DenOfIz::InteropArray<DenOfIz::ShaderStage>;
%template(StaticSamplerDescArray) DenOfIz::InteropArray<DenOfIz::StaticSamplerDesc>;
%template(TextureBarrierDescArray) DenOfIz::InteropArray<DenOfIz::TextureBarrierDesc>;
%template(UnsignedIntArray) DenOfIz::InteropArray<unsigned int>;
%template(ByteArray) DenOfIz::InteropArray<DenOfIz::Byte>;
%template(GlyphVertexArray) DenOfIz::InteropArray<DenOfIz::GlyphVertex>;
%template(AssetEntryArray) DenOfIz::InteropArray<DenOfIz::AssetEntry>;
%template(Float_2Array) DenOfIz::InteropArray<DenOfIz::Float_2>;
%template(Float_3Array) DenOfIz::InteropArray<DenOfIz::Float_3>;
%template(Float_4Array) DenOfIz::InteropArray<DenOfIz::Float_4>;
%template(Float_4x4Array) DenOfIz::InteropArray<DenOfIz::Float_4x4>;
%template(ShaderRecordDebugDataArray) DenOfIz::InteropArray<DenOfIz::ShaderRecordDebugData>;
%template(ILocalRootSignatureArray) DenOfIz::InteropArray<DenOfIz::ILocalRootSignature*>;
%template(ITextureResourceArray) DenOfIz::InteropArray<DenOfIz::ITextureResource*>;
%template(LocalRootSignatureDescArray) DenOfIz::InteropArray<DenOfIz::LocalRootSignatureDesc>;
%template(BindlessSlotArray) DenOfIz::InteropArray<DenOfIz::BindlessSlot>;
%template(BindlessResourceDescArray) DenOfIz::InteropArray<DenOfIz::BindlessResourceDesc>;

// Animation related types
%template(AnimationClipArray) DenOfIz::InteropArray<DenOfIz::AnimationClip>;
%template(JointAnimTrackArray) DenOfIz::InteropArray<DenOfIz::JointAnimTrack>;
%template(JointArray) DenOfIz::InteropArray<DenOfIz::Joint>;
%template(MorphAnimTrackArray) DenOfIz::InteropArray<DenOfIz::MorphAnimTrack>;
%template(MorphKeyframeArray) DenOfIz::InteropArray<DenOfIz::MorphKeyframe>;
%template(MorphTargetArray) DenOfIz::InteropArray<DenOfIz::MorphTarget>;
%template(MorphTargetDeltaArray) DenOfIz::InteropArray<DenOfIz::MorphTargetDelta>;
%template(PositionKeyArray) DenOfIz::InteropArray<DenOfIz::PositionKey>;
%template(RotationKeyArray) DenOfIz::InteropArray<DenOfIz::RotationKey>;
%template(ScaleKeyArray) DenOfIz::InteropArray<DenOfIz::ScaleKey>;

// Asset related types
%template(AssetTypeArray) DenOfIz::InteropArray<DenOfIz::AssetType>;
%template(AssetUriArray) DenOfIz::InteropArray<DenOfIz::AssetUri>;
%template(BundlePointerArray) DenOfIz::InteropArray<DenOfIz::Bundle*>;
%template(ColorFormatArray) DenOfIz::InteropArray<DenOfIz::ColorFormat>;
%template(CompiledShaderStagePointerArray) DenOfIz::InteropArray<DenOfIz::CompiledShaderStage*>;

// BitSet template instantiations not already covered
%template(GeometryFlagsBitSet) DenOfIz::BitSet<DenOfIz::GeometryFlags>;

// Rendering and shader related types
%template(BlendingJobDescLayerArray) DenOfIz::InteropArray<DenOfIz::BlendingJobLayerDesc>;
%template(BoundingVolumeArray) DenOfIz::InteropArray<DenOfIz::BoundingVolume>;
%template(FontGlyphArray) DenOfIz::InteropArray<DenOfIz::FontGlyph>;
%template(HitGroupDescArray) DenOfIz::InteropArray<DenOfIz::HitGroupDesc>;
%template(MeshVertexArray) DenOfIz::InteropArray<DenOfIz::MeshVertex>;
%template(PhysicsColliderArray) DenOfIz::InteropArray<DenOfIz::PhysicsCollider>;
%template(ResourceBindingSlotArray) DenOfIz::InteropArray<DenOfIz::ResourceBindingSlot>;
%template(ShaderStageAssetArray) DenOfIz::InteropArray<DenOfIz::ShaderStageAsset>;
%template(SubMeshDataArray) DenOfIz::InteropArray<DenOfIz::SubMeshData>;
%template(TextureMipArray) DenOfIz::InteropArray<DenOfIz::TextureMip>;
%template(ThreadGroupInfoArray) DenOfIz::InteropArray<DenOfIz::ThreadGroupInfo>;
%template(TransitionBufferDescArray) DenOfIz::InteropArray<DenOfIz::TransitionBufferDesc>;
%template(TransitionTextureDescArray) DenOfIz::InteropArray<DenOfIz::TransitionTextureDesc>;
%template(UserPropertyArray) DenOfIz::InteropArray<DenOfIz::UserProperty>;
%template(UVChannelArray) DenOfIz::InteropArray<DenOfIz::UVChannel>;
%template(UnsignedShortArray) DenOfIz::InteropArray<unsigned short>;
%template(FloatArray) DenOfIz::InteropArray<float>;
%template(FloatArrayArray) DenOfIz::InteropArray<DenOfIz::InteropArray<float>>;
%template(CompiledShaderArray) DenOfIz::InteropArray<DenOfIz::CompiledShader>;
%template(UnsignedCharArray) DenOfIz::InteropArray<unsigned char>;
%template(ByteArray) DenOfIz::InteropArray<unsigned char>;
%template(BoolArray) DenOfIz::InteropArray<bool>;
%template(IntArray) DenOfIz::InteropArray<int>;

// Vector graphics
%template(ThorVGColorStopArray) DenOfIz::InteropArray<DenOfIz::ThorVGColorStop>;

// UI related types
typedef DenOfIz::InteropArray<DenOfIz::UIVertex> UIVertexArray;

%template(UIVertexArray) DenOfIz::InteropArray<DenOfIz::UIVertex>;