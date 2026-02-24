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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ShaderBindingTable.h"

namespace DenOfIz
{

    class IShaderBindingTable
    {
    public:
        // TODO TBD if we want to keep this
        virtual void Resize( const DenOfIz_SBTSizeDesc &resizeDesc )                         = 0;
        virtual void BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc ) = 0;
        virtual void BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc )                 = 0;
        virtual void BindMissShader( const DenOfIz_MissBindingDesc &desc )                   = 0;
        virtual void Build( )                                                                = 0;
        virtual ~IShaderBindingTable( )                                                      = default;
    };
} // namespace DenOfIz
