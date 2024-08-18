#pragma once

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#if defined( __APPLE__ )
#include <metal_irconverter/metal_irconverter.h>
#include <simd/simd.h>
#include "Metal/Metal.h"
#include "MetalKit/MetalKit.h"
#endif

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/IShader.h>

namespace DenOfIz
{

    enum class TargetIL
    {
        DXIL,
        MSL,
        SPIRV
    };

    struct CompileOptions
    {
        std::string EntryPoint = "main";
        ShaderStage Stage;
        TargetIL    TargetIL;

        std::vector<std::string> Defines;
    };

    class ShaderCompiler final : public NonCopyable
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
        IDxcUtils                *DxcUtils( ) const;

                                                      ShaderCompiler( );
        ~                                             ShaderCompiler( );
        void                                          InitResources( TBuiltInResource &Resources ) const;
        [[nodiscard]] EShLanguage                     FindLanguage( ShaderStage shaderType ) const;
        [[nodiscard]] std::unique_ptr<CompiledShader> CompileHLSL( const std::string &path, const CompileOptions &compileOptions ) const;
        [[nodiscard]] std::vector<uint32_t>           CompileGLSL( const std::string &filename, const CompileOptions &compileOptions ) const;
        [[nodiscard]] IDxcBlob                       *DxilToMsl( const CompileOptions &compileOptions, IDxcBlob *code ) const;
#ifdef BUILD_METAL
        [[nodiscard]] static IRShaderStage ConvertIrShaderStage( const ShaderStage &stage );
#endif

        void CacheCompiledShader( const std::string &filename, const TargetIL &targetIL, IDxcBlob *code ) const;
    };

    struct MetalDxcBlob_Impl final : IDxcBlob
    {
        uint16_t m_refCount = 0;
        uint8_t *m_data;
        size_t   m_size;
#ifdef BUILD_METAL
        // This is not a very nice solution, might need to revisit this later
        IRObject *IrObject = nullptr;
#endif

        MetalDxcBlob_Impl( uint8_t *data, const size_t size ) : m_data( data ), m_size( size )
        {
            m_refCount = 1;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject ) override
        {
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef( ) override
        {
            return ++m_refCount;
        }

        ULONG STDMETHODCALLTYPE Release( ) override
        {
            if ( --m_refCount == 0 )
            {
#ifdef BUILD_METAL
                if ( IrObject )
                {
                    IRObjectDestroy( IrObject );
                }
#endif
                delete[] m_data;
                delete this;
            }
            return m_refCount;
        }

        LPVOID STDMETHODCALLTYPE GetBufferPointer( ) override
        {
            return m_data;
        }

        SIZE_T STDMETHODCALLTYPE GetBufferSize( ) override
        {
            return m_size;
        }
    };
} // namespace DenOfIz
