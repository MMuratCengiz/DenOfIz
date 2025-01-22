#pragma once

#include "CommonData.h"

namespace DenOfIz
{
    enum class ReflectionBindingType
    {
        Pointer,
        Struct,
        Texture,
        SamplerDesc,
    };

    enum class ReflectionFieldType
    {
        Undefined,
        Void,
        Bool,
        Int,
        Float,
        String,
        Texture,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        Sampler,
        Sampler1d,
        Sampler2d,
        Sampler3d,
        SamplerCube,
        PixelFragment,
        VertexFragment,
        Uint,
        Uint8,
        DepthStencil,
        Blend,
        Buffer,
        CBuffer,
        TBuffer,
        Texture1DArray,
        Texture2DArray,
        RenderTargetView,
        DepthStencilView,
        Texture2Dms,
        Texture2DmsArray,
        TextureCubeArray,
        InterfacePointer,
        Double,
        RWTexture1D,
        RWTexture1DArray,
        RWTexture2D,
        RWTexture2DArray,
        RWTexture3D,
        RWBuffer,
        ByteAddressBuffer,
        RWByteAddressBuffer,
        StructuredBuffer,
        RWStructuredBuffer,
        AppendStructuredBuffer,
        ConsumeStructuredBuffer,
        Min8Float,
        Min10Float,
        Min16Float,
        Min12Int,
        Min16Int,
        Min16UInt,
        Int16,
        UInt16,
        Float16,
        Int64,
        UInt64,
        PixelShader,
        VertexShader,
        GeometryShader,
        HullShader,
        DomainShader,
        ComputeShader,
    };

    struct DZ_API ReflectionResourceField
    {
        InteropString       Name;
        ReflectionFieldType Type;
        uint32_t            NumColumns  = 1;
        uint32_t            NumRows     = 0;
        uint32_t            Elements    = 0;
        uint32_t            Offset      = 0;
        uint32_t            Level       = 0;          // Nesting level in hierarchy
        uint32_t            ParentIndex = UINT32_MAX; // UINT32_MAX is root
    };
    template class DZ_API InteropArray<ReflectionResourceField>;

    enum class ReflectRootParameterType
    {
        DescriptorTable,
        RootConstant,
        RootDescriptor,
    };

    struct DZ_API ReflectionDesc
    {
        InteropString                         Name;
        ReflectionBindingType                 Type;
        InteropArray<ReflectionResourceField> Fields;
        size_t                                NumBytes = 0;
    };
} // namespace DenOfIz
