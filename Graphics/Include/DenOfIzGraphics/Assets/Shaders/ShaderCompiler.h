#pragma once

#include <memory>
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include "DenOfIzGraphics/Utilities/Common.h"

namespace DenOfIz
{
    struct DZ_API CompileDesc
    {
        InteropString Path;
        CodePage      CodePage;
        ByteArray     Data;
        InteropString EntryPoint = "main";
        ShaderStage   Stage;
        TargetIL      TargetIL;
        StringArray   Defines;
    };

    struct DZ_API CompileResult
    {
        ByteArray Code;
        ByteArray Reflection;
    };

    class ShaderCompiler final
    {
    public:
        DZ_API static constexpr uint32_t VkShiftCbv     = 1000;
        DZ_API static constexpr uint32_t VkShiftSrv     = 2000;
        DZ_API static constexpr uint32_t VkShiftUav     = 3000;
        DZ_API static constexpr uint32_t VkShiftSampler = 4000;

        DZ_API ShaderCompiler( );
        DZ_API ~ShaderCompiler( );
        DZ_API [[nodiscard]] CompileResult CompileHLSL( const CompileDesc &compileDesc ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
} // namespace DenOfIz
