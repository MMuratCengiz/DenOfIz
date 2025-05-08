/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>

namespace DenOfIz
{
    struct DZ_API ShaderAssetWriterDesc
    {
        BinaryWriter *Writer;
    };

    class DZ_API ShaderAssetWriter
    {
        ShaderAsset   m_shaderAsset;
        BinaryWriter *m_writer;
        uint64_t      m_streamStartOffset = 0;
        bool          m_finalized         = false;

    public:
        DZ_API explicit ShaderAssetWriter( const ShaderAssetWriterDesc &desc );
        DZ_API ~ShaderAssetWriter( );

        DZ_API void Write( const ShaderAsset &shaderAsset );
        DZ_API void End( );

        DZ_API static ShaderAsset CreateFromCompiledShader( const CompiledShader &compiledShader );

    private:
        void WriteHeader( uint32_t totalNumBytes ) const;
        void WriteInputLayout( const InputLayoutDesc &inputLayout ) const;
        void WriteRootSignature( const RootSignatureDesc &rootSignature ) const;
        void WriteLocalRootSignature( const LocalRootSignatureDesc &localDesc ) const;
        void WriteResourceBinding( const ResourceBindingDesc &resourceBinding ) const;
        void WriteResourceReflection( const ReflectionDesc &reflection ) const;
    };
} // namespace DenOfIz
