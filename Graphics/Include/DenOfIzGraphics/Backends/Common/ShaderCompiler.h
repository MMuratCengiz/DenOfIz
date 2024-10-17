#pragma once

#if defined( __APPLE__ )
#include <metal_irconverter/metal_irconverter.h>
#include <simd/simd.h>
#include "Metal/Metal.h"
#include "MetalKit/MetalKit.h"
#endif

#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <DenOfIzGraphics/Utilities/Common.h>

namespace DenOfIz
{

    struct DZ_API CompileDesc
    {
        InteropString               Path;
        InteropString               EntryPoint = "main";
        ShaderStage                 Stage;
        TargetIL                    TargetIL;
        InteropArray<InteropString> Defines;
    };

    class DZ_API ShaderCompiler final
    {
        IDxcLibrary   *m_dxcLibrary  = nullptr;
        IDxcCompiler3 *m_dxcCompiler = nullptr;
        IDxcUtils     *m_dxcUtils    = nullptr;

#if defined( __APPLE__ )
        IRCompiler *m_irCompiler = nullptr;
#endif

    public:
        static constexpr uint32_t VkShiftCbv     = 1000;
        static constexpr uint32_t VkShiftSrv     = 2000;
        static constexpr uint32_t VkShiftUav     = 3000;
        static constexpr uint32_t VkShiftSampler = 4000;
        [[nodiscard]] IDxcUtils  *DxcUtils( ) const;

        ShaderCompiler( );
        ~ShaderCompiler( );
        [[nodiscard]] std::unique_ptr<CompiledShader> CompileHLSL( const CompileDesc &compileDesc ) const;
#ifdef BUILD_METAL
        [[nodiscard]] IDxcBlob            *DxilToMsl( const CompileDesc &compileOptions, IDxcBlob *code, IRRootSignature *rootSignature ) const;
        [[nodiscard]] static IRShaderStage ConvertIrShaderStage( const ShaderStage &stage );
#endif

        void CacheCompiledShader( const std::string &filename, const TargetIL &targetIL, IDxcBlob *code ) const;
    };

#ifdef BUILD_METAL
    struct MetalDxcBlob_Impl : IDxcBlob
    {
        uint16_t m_refCount = 0;
        uint8_t *m_data;
        size_t   m_size;
        // This is not a very nice solution, might need to revisit this later
        IRObject *IrObject = nullptr;

        MetalDxcBlob_Impl( uint8_t *data, const size_t size );
        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject ) override;
        ULONG STDMETHODCALLTYPE   AddRef( ) override;
        ULONG STDMETHODCALLTYPE   Release( ) override;
        LPVOID STDMETHODCALLTYPE  GetBufferPointer( ) override;
        SIZE_T STDMETHODCALLTYPE  GetBufferSize( ) override;
    };
#endif
} // namespace DenOfIz
