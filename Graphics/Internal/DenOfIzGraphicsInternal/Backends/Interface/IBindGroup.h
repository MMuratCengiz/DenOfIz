/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <algorithm>
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "../Common/DeferredDestroyQueue.h"
#include "IBindGroupLayout.h"
#include "IBuffer.h"
#include "ITexture.h"
#include "RayTracing/ITopLevelAS.h"

namespace DenOfIz
{
    static constexpr uint32_t DZ_ALL_MIP_LEVELS = UINT32_MAX;

    class IBindGroup : public IDeviceObject
    {
    public:
        virtual ~IBindGroup( )                                                                               = default;
        virtual IBindGroup *BeginUpdate( )                                                                   = 0;
        virtual IBindGroup *Cbv( const uint32_t binding, IBuffer *resource )                                 = 0;
        virtual IBindGroup *Cbv( const uint32_t binding, IBuffer *resource, size_t offset )                  = 0;
        virtual IBindGroup *Srv( const uint32_t binding, IBuffer *resource )                                 = 0;
        virtual IBindGroup *Srv( const uint32_t binding, IBuffer *resource, size_t offset )                  = 0;
        virtual IBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )                = 0;
        virtual IBindGroup *Srv( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) = 0;
        virtual IBindGroup *SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )        = 0;
        virtual IBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource ) = 0;
        virtual IBindGroup *Uav( const uint32_t binding, IBuffer *resource )                                 = 0;
        virtual IBindGroup *Uav( const uint32_t binding, IBuffer *resource, size_t offset )                  = 0;
        virtual IBindGroup *Uav( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) = 0;
        virtual IBindGroup *Sampler( const uint32_t binding, ISampler *sampler )                             = 0;
        virtual void        EndUpdate( )                                                                     = 0;
    };
} // namespace DenOfIz
