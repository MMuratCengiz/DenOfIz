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

#include <algorithm>
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "IBindGroupLayout.h"
#include "IBuffer.h"
#include "ITexture.h"
#include "RayTracing/ITopLevelAS.h"

namespace DenOfIz
{

    class IBindGroup
    {
    public:
        virtual ~IBindGroup( )                                                                               = default;
        virtual IBindGroup *BeginUpdate( )                                                                   = 0;
        virtual IBindGroup *Cbv( const uint32_t binding, IBuffer *resource )                                 = 0;
        virtual IBindGroup *Cbv( const uint32_t binding, IBuffer *resource, size_t offset )                  = 0;
        virtual IBindGroup *Srv( const uint32_t binding, IBuffer *resource )                                 = 0;
        virtual IBindGroup *Srv( const uint32_t binding, IBuffer *resource, size_t offset )                  = 0;
        virtual IBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )                = 0;
        virtual IBindGroup *Srv( const uint32_t binding, ITexture *resource )                                = 0;
        virtual IBindGroup *SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )        = 0;
        virtual IBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource ) = 0;
        virtual IBindGroup *Uav( const uint32_t binding, IBuffer *resource )                                 = 0;
        virtual IBindGroup *Uav( const uint32_t binding, IBuffer *resource, size_t offset )                  = 0;
        virtual IBindGroup *Uav( const uint32_t binding, ITexture *resource )                                = 0;
        virtual IBindGroup *Sampler( const uint32_t binding, ISampler *sampler )                             = 0;
        virtual void        EndUpdate( )                                                                     = 0;
    };
} // namespace DenOfIz
