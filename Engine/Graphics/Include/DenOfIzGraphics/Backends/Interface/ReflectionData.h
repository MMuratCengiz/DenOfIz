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

    struct ReflectionDesc
    {
        std::string                          Name;
        ReflectionBindingType                Type;
        std::vector<ReflectionResourceField> Fields;
        size_t                               NumBytes = 0;
        /**
         * Metal specific information to simulate register spaces. We use a top level argument buffers:
         * DescriptorTableIndex: The index of the descriptor table in the argument buffer.
         * DescriptorOffset: The offset of the descriptor within the descriptor table.
         */
        uint32_t DescriptorTableIndex = 0;
        uint32_t DescriptorOffset     = 0;
    };

    /**
     * These custom register spaces give hints to the binding model.
     * - RootConstantRegisterSpace: This register space is reserved for root constants/push constant.
     * - OptimizedRegisterSpace:
     *      - For (D3D12/Metal) this will use direct buffers/root buffers instead of descriptor tables.
     *      - For (Vulkan) this doesn't have any effect.
     */
    static constexpr uint32_t RootConstantRegisterSpace = 99;
    static constexpr uint32_t OptimizedRegisterSpace    = 2;
} // namespace DenOfIz
