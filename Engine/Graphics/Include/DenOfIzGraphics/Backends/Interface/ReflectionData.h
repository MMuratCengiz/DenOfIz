#pragma once

namespace DenOfIz
{
    enum class ReflectionBindingType
    {
        Pointer,
        Struct,
        SamplerDesc,
        Texture,
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
        // Metal workaround since it is a bit hard to migrate HLSL bindings to Metal.
        // This is normally set by the compiler, If this field is not set then(because you manually built the ResourceBinding)
        // This may not always be correct, this is okay to be set manually to fix such issues, using the ShaderCompiler
        // to generate the locations and overwriting other fields
        uint32_t LocationHint = 0;
    };
} // namespace DenOfIz
