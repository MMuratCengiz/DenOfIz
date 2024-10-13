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

    struct ReflectionResourceField
    {
        std::string         Name;
        ReflectionFieldType Type;
        uint32_t            NumColumns = 1;
        uint32_t            NumRows    = 0;
    };

    enum class ReflectRootParameterType
    {
        DescriptorTable,
        RootConstant,
        RootDescriptor,
    };

#define DZ_MAX_FIELDS 32
    struct ResourceFields
    {
        size_t                  NumElements = 0;
        ReflectionResourceField Array[ DZ_MAX_FIELDS ];
    };

    struct ReflectionDesc
    {
        std::string           Name;
        ReflectionBindingType Type;
        ResourceFields        Fields;
        size_t                NumBytes = 0;
#ifdef BUILD_METAL
        /**
         * Metal specific information to simulate register spaces. We use a top level argument buffers:
         * TLABOffset: The index of the descriptor table in the argument buffer.
         * DescriptorTableIndex: The offset of the descriptor within the descriptor table.
         *
         * Valid when RootParameterType is DescriptorTable.
         */
        uint32_t DescriptorTableIndex = 0;
        uint32_t TLABOffset           = 0;
#endif
    };
} // namespace DenOfIz
