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

    class DZ_API IResourceBindGroup
    {
    public:
        virtual ~IResourceBindGroup( )                                                                      = default;
        virtual void                SetRootConstantsData( uint32_t binding, const InteropArray<Byte> &data ) = 0;
        virtual void                SetRootConstants( uint32_t binding, void *data )                        = 0;
        virtual IResourceBindGroup *BeginUpdate( )                                                          = 0;
        virtual IResourceBindGroup *Cbv( const uint32_t binding, IBufferResource *resource )                = 0;
        virtual IResourceBindGroup *Srv( const uint32_t binding, IBufferResource *resource )                = 0;
        virtual IResourceBindGroup *Srv( const uint32_t binding, ITextureResource *resource )               = 0;
        virtual IResourceBindGroup *Uav( const uint32_t binding, IBufferResource *resource )                = 0;
        virtual IResourceBindGroup *Uav( const uint32_t binding, ITextureResource *resource )               = 0;
        virtual IResourceBindGroup *Sampler( const uint32_t binding, ISampler *sampler )                    = 0;
        virtual void                EndUpdate( )                                                            = 0;

    protected:
        virtual void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) = 0;
        virtual void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )   = 0;
        virtual void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )          = 0;
    };
} // namespace DenOfIz
