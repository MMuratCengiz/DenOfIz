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
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>

namespace DenOfIz
{
    struct DZ_API ShaderProgramAssetReaderDesc
    {
        BinaryReader *Reader;
    };

    class DZ_API ShaderAssetReader
    {
        BinaryReader *m_reader;
        ShaderAsset   m_shaderAsset;
        bool          m_assetRead         = false;
        uint64_t      m_streamStartOffset = 0;

    public:
        explicit ShaderAssetReader( const ShaderProgramAssetReaderDesc &desc );
        ~ShaderAssetReader( );

        ShaderAsset Read( );

        static CompiledShader ConvertToCompiledShader( const ShaderAsset &shaderAsset );

    private:
        void ReadHeader( );
        void ReadInputLayout( InputLayoutDesc &inputLayout ) const;
        void ReadRootSignature( RootSignatureDesc &rootSignature ) const;
        void ReadLocalRootSignature( LocalRootSignatureDesc &localDesc ) const;
        void ReadResourceBinding( ResourceBindingDesc &binding ) const;
        void ReadResourceReflection( ReflectionDesc &reflection ) const;
    };
} // namespace DenOfIz
