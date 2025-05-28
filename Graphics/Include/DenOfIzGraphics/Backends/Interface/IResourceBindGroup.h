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
#include "IBufferResource.h"
#include "IRootSignature.h"
#include "ITextureResource.h"
#include "RayTracing/ITopLevelAS.h"

namespace DenOfIz
{
    // Todo deprecate the fields, possible either use RootSignature+RegisterSpace or rely on UpdateDesc
    struct DZ_API ResourceBindGroupDesc
    {
        IRootSignature *RootSignature;
        uint32_t        RegisterSpace;
    };
    static ResourceBindGroupDesc RootConstantBindGroupDesc( IRootSignature *rootSignature )
    {
        return { rootSignature, DZConfiguration::Instance( ).RootConstantRegisterSpace };
    }

    /// Separate bindings for more complex bindings, right now only ResourceOffset exist but gives better api compatibility in general
    struct DZ_API BindBufferDesc
    {
        uint32_t         Binding;
        IBufferResource *Resource;
        uint32_t         ResourceOffset;
    };

    class DZ_API IResourceBindGroup
    {
    public:
        virtual ~IResourceBindGroup( )                                                                                                = default;
        virtual void                SetRootConstantsData( uint32_t binding, const InteropArray<Byte> &data )                          = 0;
        virtual void                SetRootConstants( uint32_t binding, void *data )                                                  = 0;
        virtual IResourceBindGroup *BeginUpdate( )                                                                                    = 0;
        virtual IResourceBindGroup *Cbv( const uint32_t binding, IBufferResource *resource )                                          = 0;
        virtual IResourceBindGroup *Cbv( const BindBufferDesc &desc )                                                                 = 0;
        virtual IResourceBindGroup *Srv( const uint32_t binding, IBufferResource *resource )                                          = 0;
        virtual IResourceBindGroup *Srv( const BindBufferDesc &desc )                                                                 = 0;
        virtual IResourceBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )                                 = 0;
        virtual IResourceBindGroup *Srv( const uint32_t binding, ITextureResource *resource )                                         = 0;
        virtual IResourceBindGroup *SrvArray( const uint32_t binding, const InteropArray<ITextureResource *> &resources )             = 0;
        virtual IResourceBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITextureResource *resource )          = 0;
        virtual IResourceBindGroup *Uav( const uint32_t binding, IBufferResource *resource )                                          = 0;
        virtual IResourceBindGroup *Uav( const BindBufferDesc &desc )                                                                 = 0;
        virtual IResourceBindGroup *Uav( const uint32_t binding, ITextureResource *resource )                                         = 0;
        virtual IResourceBindGroup *Sampler( const uint32_t binding, ISampler *sampler )                                              = 0;
        virtual void                EndUpdate( )                                                                                      = 0;
    };
} // namespace DenOfIz
