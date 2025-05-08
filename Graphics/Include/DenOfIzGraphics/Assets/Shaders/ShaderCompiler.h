#pragma once

#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <DenOfIzGraphics/Utilities/Common.h>

namespace DenOfIz
{
    struct DZ_API CompileDesc
    {
        InteropString               Path;
        CodePage                    CodePage;
        InteropArray<Byte>          Data;
        InteropString               EntryPoint = "main";
        ShaderStage                 Stage;
        TargetIL                    TargetIL;
        InteropArray<InteropString> Defines;
    };

    struct DZ_API CompileResult
    {
        InteropArray<Byte> Code;
        InteropArray<Byte> Reflection;
    };

    class ShaderCompiler final
    {
        IDxcLibrary        *m_dxcLibrary        = nullptr;
        IDxcCompiler3      *m_dxcCompiler       = nullptr;
        IDxcUtils          *m_dxcUtils          = nullptr;
        IDxcIncludeHandler *m_dxcIncludeHandler = nullptr;

    public:
        DZ_API static constexpr uint32_t VkShiftCbv     = 1000;
        DZ_API static constexpr uint32_t VkShiftSrv     = 2000;
        DZ_API static constexpr uint32_t VkShiftUav     = 3000;
        DZ_API static constexpr uint32_t VkShiftSampler = 4000;
        DZ_API [[nodiscard]] IDxcUtils  *DxcUtils( ) const;

        DZ_API ShaderCompiler( );
        DZ_API ~ShaderCompiler( );
        DZ_API [[nodiscard]] CompileResult CompileHLSL( const CompileDesc &compileDesc ) const;
    };
} // namespace DenOfIz
