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

#include "DenOfIzGraphics/Result.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define LOGICAL_DEVICE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ILogicalDevice, handle )

static bool DenOfIz_ValidateBufferDesc( const DenOfIz_BufferDesc *desc )
{
    bool valid = true;

    if ( desc->NumBytes == 0 )
    {
        spdlog::warn( "BufferDesc: NumBytes is 0, buffer will be unusable" );
        valid = false;
    }

    if ( desc->StructureDesc.Stride > 0 )
    {
        if ( desc->StructureDesc.NumElements == 0 )
        {
            spdlog::warn( "BufferDesc: Structured buffer (Stride > 0) but StructureDesc.NumElements is 0" );
            valid = false;
        }
        if ( desc->StructureDesc.NumElements > 0 )
        {
            uint64_t requiredSize = desc->StructureDesc.Offset + ( desc->StructureDesc.Stride * desc->StructureDesc.NumElements );
            if ( requiredSize > desc->NumBytes )
            {
                spdlog::warn( "BufferDesc: Structured buffer requires %llu bytes (offset=%llu + stride=%llu * numElements=%llu) but NumBytes is only %zu", requiredSize,
                              desc->StructureDesc.Offset, desc->StructureDesc.Stride, desc->StructureDesc.NumElements, desc->NumBytes );
                valid = false;
            }
        }
    }

    if ( desc->HeapType != DENOFIZ_HEAP_TYPE_GPU )
    {
        if ( desc->Usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT )
        {
            spdlog::error( "BufferDesc: STORAGE_BIT requires HEAP_TYPE_GPU. CPU-mappable heaps (CPU_GPU, GPU_CPU, CPU) cannot have storage/UAV access. "
                           "Use a staging buffer with COPY_SRC and copy to a GPU-only storage buffer instead." );
            valid = false;
        }
        if ( desc->Usage & DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT )
        {
            spdlog::error( "BufferDesc: ACCELERATION_STRUCTURE_BIT requires HEAP_TYPE_GPU. CPU-mappable heaps are not supported." );
            valid = false;
        }
    }

    constexpr uint64_t maxConstantBufferViewSize = 65536;
    if ( ( desc->Usage & DENOFIZ_BUFFER_USAGE_UNIFORM_BIT ) && desc->NumBytes > maxConstantBufferViewSize )
    {
        spdlog::warn( "BufferDesc: Uniform buffer size ({} bytes) exceeds maximum constant buffer view size ({} bytes). "
                      "When binding as CBV with offset, you must explicitly specify the view size. "
                      "Consider using a structured buffer or multiple smaller uniform buffers instead.",
                      desc->NumBytes, maxConstantBufferViewSize );
    }

    return valid;
}

static bool DenOfIz_IsFloat4x4Zero( const DenOfIz_Float4x4 &m )
{
    return m._11 == 0.0f && m._12 == 0.0f && m._13 == 0.0f && m._14 == 0.0f && m._21 == 0.0f && m._22 == 0.0f && m._23 == 0.0f && m._24 == 0.0f && m._31 == 0.0f && m._32 == 0.0f &&
           m._33 == 0.0f && m._34 == 0.0f && m._41 == 0.0f && m._42 == 0.0f && m._43 == 0.0f && m._44 == 0.0f;
}

static bool DenOfIz_ValidateTopLevelASDesc( const DenOfIz_TopLevelASDesc *desc )
{
    bool valid = true;

    if ( desc->Instances.NumElements == 0 )
    {
        spdlog::warn( "TopLevelASDesc: Instances.NumElements is 0, TLAS will be empty" );
    }

    for ( uint32_t i = 0; i < desc->Instances.NumElements; ++i )
    {
        const DenOfIz_ASInstanceDesc &instance = desc->Instances.Elements[ i ];

        if ( !DENOFIZ_HANDLE_IS_VALID( instance.BLAS ) )
        {
            spdlog::warn( "TopLevelASDesc: Instance[{}].BLAS handle is invalid", i );
            valid = false;
        }

        if ( DenOfIz_IsFloat4x4Zero( instance.Transform ) )
        {
            spdlog::warn( "TopLevelASDesc: Instance[{}].Transform is all zeros (geometry will be invisible). Did you forget to set DENOFIZ_FLOAT4X4_IDENTITY?", i );
            valid = false;
        }

        if ( instance.Mask == 0 )
        {
            spdlog::warn( "TopLevelASDesc: Instance[{}].Mask is 0, this instance will never be hit by rays", i );
        }
    }

    return valid;
}

static bool DenOfIz_ValidatePipelineDesc( const DenOfIz_PipelineDesc *desc )
{
    bool valid = true;

    if ( !DENOFIZ_HANDLE_IS_VALID( desc->RootSignature ) )
    {
        spdlog::warn( "PipelineDesc: RootSignature handle is invalid" );
        valid = false;
    }

    if ( !DENOFIZ_HANDLE_IS_VALID( desc->ShaderProgram ) )
    {
        spdlog::warn( "PipelineDesc: ShaderProgram handle is invalid" );
        valid = false;
    }

    if ( desc->BindPoint == DENOFIZ_BIND_POINT_RAYTRACING )
    {
        DenOfIz_CompiledShaderStageArray compiledShaders = { };
        if ( DENOFIZ_HANDLE_IS_VALID( desc->ShaderProgram ) )
        {
            DenOfIz_ShaderProgram_CompiledShaders( desc->ShaderProgram, &compiledShaders );
        }

        for ( uint32_t i = 0; i < desc->RayTracing.HitGroups.NumElements; ++i )
        {
            const DenOfIz_HitGroupDesc &hitGroup = desc->RayTracing.HitGroups.Elements[ i ];

            if ( hitGroup.Name.NumChars == 0 )
            {
                spdlog::warn( "PipelineDesc: RayTracing.HitGroups[{}].Name is empty", i );
                valid = false;
            }

            if ( hitGroup.Type == DENOFIZ_HIT_GROUP_TYPE_TRIANGLES && hitGroup.IntersectionShaderIndex != -1 )
            {
                spdlog::warn( "PipelineDesc: RayTracing.HitGroups[{}] has Type=TRIANGLES but IntersectionShaderIndex={} is set. Triangle hit groups should not have intersection "
                              "shaders (set to -1)",
                              i, hitGroup.IntersectionShaderIndex );
            }

            if ( hitGroup.Type == DENOFIZ_HIT_GROUP_TYPE_AABBS && hitGroup.IntersectionShaderIndex == -1 )
            {
                spdlog::warn( "PipelineDesc: RayTracing.HitGroups[{}] has Type=PROCEDURAL but IntersectionShaderIndex=-1. Procedural hit groups require an intersection shader",
                              i );
                valid = false;
            }

            if ( compiledShaders.NumElements > 0 )
            {
                auto validateShaderIndex = [ & ]( int32_t index, const char *name, DenOfIz_ShaderStageFlags expectedStage )
                {
                    if ( index == -1 )
                    {
                        return;
                    }
                    if ( index < 0 || static_cast<uint32_t>( index ) >= compiledShaders.NumElements )
                    {
                        spdlog::warn( "PipelineDesc: RayTracing.HitGroups[{}].{} = {} is out of bounds (ShaderProgram has {} stages)", i, name, index,
                                      compiledShaders.NumElements );
                        valid = false;
                        return;
                    }
                    DenOfIz_ShaderStageFlags actualStage = compiledShaders.Elements[ index ]->Stage;
                    if ( actualStage != expectedStage )
                    {
                        spdlog::warn( "PipelineDesc: RayTracing.HitGroups[{}].{} = {} points to shader with stage {} but expected stage {}", i, name, index,
                                      static_cast<int>( actualStage ), static_cast<int>( expectedStage ) );
                        valid = false;
                    }
                };

                validateShaderIndex( hitGroup.IntersectionShaderIndex, "IntersectionShaderIndex", DENOFIZ_SHADER_STAGE_INTERSECTION_BIT );
                validateShaderIndex( hitGroup.AnyHitShaderIndex, "AnyHitShaderIndex", DENOFIZ_SHADER_STAGE_ANY_HIT_BIT );
                validateShaderIndex( hitGroup.ClosestHitShaderIndex, "ClosestHitShaderIndex", DENOFIZ_SHADER_STAGE_CLOSEST_HIT_BIT );
            }
        }
    }

    return valid;
}

static bool DenOfIz_ValidateShaderBindingTableDesc( const DenOfIz_ShaderBindingTableDesc *desc )
{
    bool valid = true;

    if ( !DENOFIZ_HANDLE_IS_VALID( desc->Pipeline ) )
    {
        spdlog::warn( "ShaderBindingTableDesc: Pipeline handle is invalid" );
        valid = false;
    }

    if ( desc->SizeDesc.NumRayGenerationShaders == 0 )
    {
        spdlog::warn( "ShaderBindingTableDesc: SizeDesc.NumRayGenerationShaders is 0, at least 1 ray generation shader is required" );
        valid = false;
    }

    if ( desc->SizeDesc.NumMissShaders == 0 && desc->SizeDesc.NumHitGroups == 0 )
    {
        spdlog::warn( "ShaderBindingTableDesc: Both SizeDesc.NumMissShaders and SizeDesc.NumHitGroups are 0, at least one should be non-zero for a functional SBT" );
    }

    return valid;
}

static bool DenOfIz_ValidateTextureDesc( const DenOfIz_TextureDesc *desc )
{
    bool valid = true;

    if ( desc->Width == 0 )
    {
        spdlog::error( "TextureDesc: Width must be greater than 0" );
        valid = false;
    }

    if ( desc->ArraySize == 0 )
    {
        spdlog::warn( "TextureDesc: ArraySize is 0, must be at least 1" );
        valid = false;
    }

    if ( desc->MipLevels == 0 )
    {
        spdlog::warn( "TextureDesc: MipLevels is 0, must be at least 1" );
        valid = false;
    }

    if ( desc->Format == DENOFIZ_FORMAT_UNDEFINED )
    {
        spdlog::warn( "TextureDesc: Format is UNDEFINED" );
        valid = false;
    }

    bool isDepthFormat = DenOfIz_Format_IsDepth( desc->Format );

    if ( desc->MSAASampleCount > DENOFIZ_MSAA_SAMPLE_COUNT_1 && desc->MipLevels > 1 )
    {
        spdlog::warn( "TextureDesc: Multisampled textures cannot have more than 1 mip level (MipLevels=%u, MSAA=%d)", desc->MipLevels,
                      DenOfIz_MSAASampleCount_ToNumSamples( desc->MSAASampleCount ) );
        valid = false;
    }

    if ( desc->Usage & DENOFIZ_TEXTURE_USAGE_CUBE_BIT )
    {
        if ( desc->ArraySize == 0 || ( desc->ArraySize % 6 ) != 0 )
        {
            spdlog::warn( "TextureDesc: Cube texture ArraySize must be a multiple of 6 (ArraySize=%u)", desc->ArraySize );
            valid = false;
        }
        if ( desc->Width != desc->Height )
        {
            spdlog::warn( "TextureDesc: Cube texture must have equal Width and Height (Width=%u, Height=%u)", desc->Width, desc->Height );
            valid = false;
        }
    }

    if ( desc->Depth > 1 )
    {
        if ( desc->ArraySize > 1 )
        {
            spdlog::warn( "TextureDesc: 3D textures (Depth > 1) cannot be arrays (ArraySize=%u)", desc->ArraySize );
            valid = false;
        }
        if ( desc->MSAASampleCount > DENOFIZ_MSAA_SAMPLE_COUNT_1 )
        {
            spdlog::warn( "TextureDesc: 3D textures cannot be multisampled" );
            valid = false;
        }
    }

    if ( desc->Usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
    {
        if ( desc->MSAASampleCount > DENOFIZ_MSAA_SAMPLE_COUNT_1 )
        {
            spdlog::warn( "TextureDesc: STORAGE_BINDING (UAV) is not compatible with multisampled textures" );
            valid = false;
        }
    }

    if ( DenOfIz_Format_IsBC( desc->Format ) )
    {
        if ( ( desc->Width % 4 ) != 0 || ( desc->Height % 4 ) != 0 )
        {
            spdlog::warn( "TextureDesc: Block-compressed format requires Width and Height to be multiples of 4 (Width=%u, Height=%u)", desc->Width, desc->Height );
            valid = false;
        }
        if ( desc->Usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
        {
            spdlog::warn( "TextureDesc: Block-compressed formats do not support STORAGE_BINDING (UAV) access" );
            valid = false;
        }
    }

    return valid;
}

static bool DenOfIz_ValidateResourceBindingDesc( const DenOfIz_BindingDesc *binding, uint32_t index )
{
    bool valid = true;

    if ( binding->ArraySize == 0 )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: ArraySize must be >= 1 (got {})", index, binding->ArraySize );
        valid = false;
    }

    if ( binding->IsBindless && binding->ArraySize < 2 )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: IsBindless is true but ArraySize is {} - bindless arrays typically have large sizes", index, binding->ArraySize );
    }

    const uint32_t descriptor = binding->Descriptor;

    const bool hasRwBuffer  = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT ) != 0;
    const bool hasRwTexture = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT ) != 0;
    const bool hasSampler   = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT ) != 0;
    const bool hasUniform   = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT ) != 0;
    const bool hasBuffer    = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT ) != 0;
    const bool hasTexture   = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT ) != 0;
    const bool hasAccelStr  = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT ) != 0;

    if ( descriptor == DENOFIZ_RESOURCE_DESCRIPTOR_NONE_BIT )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: Descriptor is NONE_BIT - no resource type specified", index );
        valid = false;
    }

    if ( hasSampler && ( hasBuffer || hasTexture || hasUniform || hasRwBuffer || hasRwTexture || hasAccelStr ) )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: SAMPLER_BIT cannot be combined with other resource types", index );
        valid = false;
    }

    if ( hasUniform && ( hasRwBuffer || hasRwTexture ) )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: UNIFORM_BUFFER_BIT (CBV) cannot be combined with RW_* flags (UAV)", index );
        valid = false;
    }

    if ( hasUniform && ( hasBuffer || hasTexture ) )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: UNIFORM_BUFFER_BIT should not be combined with BUFFER_BIT or TEXTURE_BIT", index );
    }

    if ( hasRwBuffer && hasRwTexture )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: RW_BUFFER_BIT and RW_TEXTURE_BIT should not both be set for a single binding", index );
    }

    if ( hasBuffer && hasTexture )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: BUFFER_BIT and TEXTURE_BIT should not both be set for a single binding", index );
    }

    const bool hasNonBindingFlags = ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RENDER_TARGET_BIT ) || ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_DEPTH_STENCIL_BIT ) ||
                                    ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDEX_BUFFER_BIT ) || ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_VERTEX_BUFFER_BIT ) ||
                                    ( descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_INDIRECT_BUFFER_BIT );

    if ( hasNonBindingFlags )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: Contains flags not typically used for shader resource bindings (RENDER_TARGET, DEPTH_STENCIL, INDEX, VERTEX, or INDIRECT)", index );
    }

    if ( binding->Stages == DENOFIZ_SHADER_STAGE_NONE_BIT )
    {
        spdlog::warn( "ResourceBindingDesc[{}]: Stages is NONE_BIT - binding will not be visible to any shader stage", index );
        valid = false;
    }

    return valid;
}

static bool DenOfIz_ValidateRootSignatureDesc( const DenOfIz_RootSignatureDesc *desc )
{
    bool valid = true;

    for ( uint32_t i = 0; i < desc->BindGroupLayouts.NumElements; ++i )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( desc->BindGroupLayouts.Elements[ i ] ) )
        {
            spdlog::warn( "RootSignatureDesc: BindGroupLayouts[{}] is an invalid handle", i );
            valid = false;
        }
    }

    for ( uint32_t i = 0; i < desc->RootConstants.NumElements; ++i )
    {
        const DenOfIz_RootConstantBindingDesc *rootConstant = &desc->RootConstants.Elements[ i ];
        if ( rootConstant->NumBytes == 0 )
        {
            spdlog::warn( "RootConstantResourceBindingDesc[{}]: NumBytes is 0", i );
            valid = false;
        }
        if ( rootConstant->Stages == DENOFIZ_SHADER_STAGE_NONE_BIT )
        {
            spdlog::warn( "RootConstantResourceBindingDesc[{}]: Stages is NONE_BIT - push constant will not be visible to any shader stage", i );
            valid = false;
        }
    }

    return valid;
}

extern "C"
{

    DenOfIz_Result DenOfIz_LogicalDevice_CreateDevice( DenOfIz_LogicalDevice device, const DenOfIz_LogicalDeviceDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        LOGICAL_DEVICE_IMPL( device )->CreateDevice( *desc );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_ListPhysicalDevices( DenOfIz_LogicalDevice device, DenOfIz_PhysicalDeviceArray *outDevices )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( outDevices == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outDevices = LOGICAL_DEVICE_IMPL( device )->ListPhysicalDevices( );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_LoadPhysicalDevice( DenOfIz_LogicalDevice device, const DenOfIz_PhysicalDevice *physicalDevice )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( physicalDevice == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        LOGICAL_DEVICE_IMPL( device )->LoadPhysicalDevice( *physicalDevice );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_DeviceInfo( DenOfIz_LogicalDevice device, DenOfIz_PhysicalDevice *outDevice )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( outDevice == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outDevice = LOGICAL_DEVICE_IMPL( device )->DeviceInfo( );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_IsDeviceLost( DenOfIz_LogicalDevice device, bool *outLost )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( outLost == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outLost = LOGICAL_DEVICE_IMPL( device )->IsDeviceLost( );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_WaitIdle( DenOfIz_LogicalDevice device )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        LOGICAL_DEVICE_IMPL( device )->WaitIdle( );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateCommandQueue( DenOfIz_LogicalDevice device, const DenOfIz_CommandQueueDesc *desc, DenOfIz_CommandQueue *outQueue )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outQueue == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outQueue = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateCommandQueue( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateCommandListPool( DenOfIz_LogicalDevice device, const DenOfIz_CommandListPoolDesc *desc, DenOfIz_CommandListPool *outPool )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outPool == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outPool = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateCommandListPool( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreatePipeline( DenOfIz_LogicalDevice device, const DenOfIz_PipelineDesc *desc, DenOfIz_Pipeline *outPipeline )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outPipeline == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DenOfIz_ValidatePipelineDesc( desc ) )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outPipeline = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreatePipeline( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreatePipelineCache( DenOfIz_LogicalDevice device, const DenOfIz_PipelineCacheDesc *desc, DenOfIz_PipelineCache *outCache )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outCache == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outCache = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreatePipelineCache( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateSwapChain( DenOfIz_LogicalDevice device, const DenOfIz_SwapChainDesc *desc, DenOfIz_SwapChain *outSwapChain )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outSwapChain == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outSwapChain = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateSwapChain( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateRootSignature( DenOfIz_LogicalDevice device, const DenOfIz_RootSignatureDesc *desc, DenOfIz_RootSignature *outSignature )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outSignature == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DenOfIz_ValidateRootSignatureDesc( desc ) )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outSignature = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateRootSignature( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateInputLayout( DenOfIz_LogicalDevice device, const DenOfIz_InputLayoutDesc *desc, DenOfIz_InputLayout *outLayout )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outLayout == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outLayout = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateInputLayout( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateBindGroupLayout( DenOfIz_LogicalDevice device, const DenOfIz_BindGroupLayoutDesc *desc, DenOfIz_BindGroupLayout *outLayout )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outLayout == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        for ( uint32_t i = 0; i < desc->Bindings.NumElements; ++i )
        {
            DenOfIz_ValidateResourceBindingDesc( &desc->Bindings.Elements[ i ], i );
        }
        *outLayout = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateBindGroupLayout( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateBindGroup( DenOfIz_LogicalDevice device, const DenOfIz_BindGroupDesc *desc, DenOfIz_BindGroup *outGroup )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outGroup == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DENOFIZ_HANDLE_IS_VALID( desc->Layout ) )
        {
            spdlog::warn( "BindGroupDesc: Layout handle is invalid" );
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outGroup = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateBindGroup( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateFence( DenOfIz_LogicalDevice device, DenOfIz_Fence *outFence )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( outFence == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outFence = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateFence( ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateSemaphore( DenOfIz_LogicalDevice device, DenOfIz_Semaphore *outSemaphore )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( outSemaphore == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outSemaphore = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateSemaphore( ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateBuffer( DenOfIz_LogicalDevice device, const DenOfIz_BufferDesc *desc, DenOfIz_Buffer *outBuffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outBuffer == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DenOfIz_ValidateBufferDesc( desc ) )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outBuffer = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateBuffer( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateTexture( DenOfIz_LogicalDevice device, const DenOfIz_TextureDesc *desc, DenOfIz_Texture *outTexture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outTexture == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DenOfIz_ValidateTextureDesc( desc ) )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outTexture = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateTexture( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateSampler( DenOfIz_LogicalDevice device, const DenOfIz_SamplerDesc *desc, DenOfIz_Sampler *outSampler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outSampler == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outSampler = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateSampler( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateQueryPool( DenOfIz_LogicalDevice device, const DenOfIz_QueryPoolDesc *desc, DenOfIz_QueryPool *outPool )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outPool == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outPool = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateQueryPool( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateTopLevelAS( DenOfIz_LogicalDevice device, const DenOfIz_TopLevelASDesc *desc, DenOfIz_TopLevelAS *outAS )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outAS == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DenOfIz_ValidateTopLevelASDesc( desc ) )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outAS = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateTopLevelAS( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateBottomLevelAS( DenOfIz_LogicalDevice device, const DenOfIz_BottomLevelASDesc *desc, DenOfIz_BottomLevelAS *outAS )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outAS == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outAS = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateBottomLevelAS( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateShaderBindingTable( DenOfIz_LogicalDevice device, const DenOfIz_ShaderBindingTableDesc *desc, DenOfIz_ShaderBindingTable *outTable )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outTable == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        if ( !DenOfIz_ValidateShaderBindingTableDesc( desc ) )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outTable = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateShaderBindingTable( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateLocalRootSignature( DenOfIz_LogicalDevice device, const DenOfIz_LocalRootSignatureDesc *desc,
                                                                   DenOfIz_LocalRootSignature *outSignature )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outSignature == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outSignature = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateLocalRootSignature( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    DenOfIz_Result DenOfIz_LogicalDevice_CreateShaderLocalData( DenOfIz_LogicalDevice device, const DenOfIz_ShaderLocalDataDesc *desc, DenOfIz_ShaderLocalData *outData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return DENOFIZ_ERROR_INVALID_HANDLE;
        }
        if ( desc == NULL || outData == NULL )
        {
            return DENOFIZ_ERROR_INVALID_ARGUMENT;
        }
        *outData = DENOFIZ_TO_HANDLE( LOGICAL_DEVICE_IMPL( device )->CreateShaderLocalData( *desc ) );
        return DENOFIZ_SUCCESS;
    }

    void DenOfIz_LogicalDevice_Destroy( DenOfIz_LogicalDevice device )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( device ) )
        {
            return;
        }
        delete LOGICAL_DEVICE_IMPL( device );
    }
}
