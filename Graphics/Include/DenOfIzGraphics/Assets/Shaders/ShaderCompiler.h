#pragma once

#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <DenOfIzGraphics/Utilities/Common.h>

namespace DenOfIz
{

    struct DZ_API CompileDesc
    {
        InteropString               Path;
        InteropArray<Byte>          Data;
        InteropString               EntryPoint = "main";
        ShaderStage                 Stage;
        TargetIL                    TargetIL;
        InteropArray<InteropString> Defines;
    };

    struct DZ_API CompileResult
    {
        IDxcBlob *Code       = nullptr;
        IDxcBlob *Reflection = nullptr; // Reflection is only present if the TargetIL = DXIL.
    };

    class DZ_API ShaderCompiler final
    {
        IDxcLibrary        *m_dxcLibrary        = nullptr;
        IDxcCompiler3      *m_dxcCompiler       = nullptr;
        IDxcUtils          *m_dxcUtils          = nullptr;
        IDxcIncludeHandler *m_dxcIncludeHandler = nullptr;

    public:
        static constexpr uint32_t VkShiftCbv     = 1000;
        static constexpr uint32_t VkShiftSrv     = 2000;
        static constexpr uint32_t VkShiftUav     = 3000;
        static constexpr uint32_t VkShiftSampler = 4000;
        [[nodiscard]] IDxcUtils  *DxcUtils( ) const;

        ShaderCompiler( );
        ~ShaderCompiler( );
        [[nodiscard]] CompileResult CompileHLSL( const CompileDesc &compileDesc ) const;
        void        CacheCompiledShader( const std::string &filename, const std::string &entryPoint, const TargetIL &targetIL, IDxcBlob *code, IDxcBlob *reflection ) const;
        std::string CachedShaderFile( const std::string &filename, const std::string &entryPoint, const TargetIL &targetIL ) const;
        std::string CachedReflectionFile( const std::string &filename, const std::string &entryPoint ) const;
        [[nodiscard]] IDxcBlob *LoadCachedShader( const std::string &filename ) const;
        [[nodiscard]] IDxcBlob *LoadCachedReflection( const std::string &filename, const std::string &entryPoint ) const;
    };
} // namespace DenOfIz
