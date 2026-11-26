/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <memory>
#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"

namespace DenOfIz
{
    struct DxilToMslDesc
    {
        DenOfIz_ShaderStageDescArray     Shaders;
        DenOfIz_CompiledShaderStageArray DXILShaders;
        DenOfIz_ShaderRayTracingDesc     RayTracing;
        DenOfIz_ByteArrayArray          *OutMSLShaders;
    };

    class DxilToMsl
    {
    public:
        DxilToMsl( );
        ~DxilToMsl( );
        void Convert( const DxilToMslDesc &desc ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
} // namespace DenOfIz
