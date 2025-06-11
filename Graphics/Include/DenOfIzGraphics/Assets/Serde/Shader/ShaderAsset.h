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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Assets/Shaders/ShaderReflectDesc.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    struct DZ_API ShaderStageAsset
    {
        ShaderStage          Stage;
        InteropString        EntryPoint;
        ByteArray            DXIL;
        ByteArray            MSL;
        ByteArray            SPIRV;
        ByteArray            Reflection; // Generated Dxc reflection
        RayTracingShaderDesc RayTracing;

        void Dispose( ) const
        {
            DXIL.Dispose( );
            MSL.Dispose( );
            SPIRV.Dispose( );
            Reflection.Dispose( );
        }
    };

    struct DZ_API ShaderStageAssetArray
    {
        ShaderStageAsset *Elements;
        uint32_t          NumElements;

        void Dispose( ) const
        {
            for ( uint32_t i = 0; i < NumElements; ++i )
            {
                Elements[ i ].Dispose( );
            }
            delete[] Elements;
        }
    };

    struct DZ_API ShaderAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        ShaderStageAssetArray  Stages;
        ShaderReflectDesc      ReflectDesc;
        ShaderRayTracingDesc   RayTracing;
        UserPropertyArray      UserProperties;

        ShaderAsset( ) : AssetHeader( 0x44414853445A /*DZSHAD*/, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzshader";
        }

        void Dispose( ) const
        {
            Stages.Dispose( );
            delete[] UserProperties.Elements;
        }
    };
} // namespace DenOfIz
