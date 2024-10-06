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
    struct ResourceBindGroupDesc
    {
        IRootSignature *RootSignature;
        uint32_t        RegisterSpace;
    };
    static ResourceBindGroupDesc RootConstantBindGroupDesc( IRootSignature *rootSignature )
    {
        return { rootSignature, 99 };
    }

    template <typename T>
    struct UpdateDescItem
    {
        ResourceBindingSlot Slot;
        T                  *Resource;
    };

    struct UpdateDesc
    {
        uint32_t                                      RegisterSpace;
        std::vector<UpdateDescItem<IBufferResource>>  Buffers;
        std::vector<UpdateDescItem<ITextureResource>> Textures;
        std::vector<UpdateDescItem<ISampler>>         Samplers;

        explicit UpdateDesc( const uint32_t registerSpace ) : RegisterSpace( registerSpace )
        {
        }

        UpdateDesc &Cbv( const uint32_t binding, IBufferResource *resource )
        {
            ResourceBindingSlot slot{ };
            slot.RegisterSpace = RegisterSpace;
            slot.Binding       = binding;
            slot.Type          = DescriptorBufferBindingType::ConstantBuffer;
            Buffers.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Srv( const uint32_t binding, IBufferResource *resource )
        {
            ResourceBindingSlot slot{ };
            slot.RegisterSpace = RegisterSpace;
            slot.Binding       = binding;
            slot.Type          = DescriptorBufferBindingType::ConstantBuffer;
            Buffers.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Srv( const uint32_t binding, ITextureResource *resource )
        {
            ResourceBindingSlot slot{ };
            slot.RegisterSpace = RegisterSpace;
            slot.Binding       = binding;
            slot.Type          = DescriptorBufferBindingType::ShaderResource;
            Textures.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Uav( const uint32_t binding, IBufferResource *resource )
        {
            ResourceBindingSlot slot{ };
            slot.RegisterSpace = RegisterSpace;
            slot.Binding       = binding;
            slot.Type          = DescriptorBufferBindingType::ShaderResource;
            Buffers.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Uav( const uint32_t binding, ITextureResource *resource )
        {
            ResourceBindingSlot slot{ };
            slot.RegisterSpace = RegisterSpace;
            slot.Binding       = binding;
            slot.Type          = DescriptorBufferBindingType::UnorderedAccess;
            Textures.push_back( { slot, resource } );
            return *this;
        }

        UpdateDesc &Sampler( const ResourceBindingSlot &slot, ISampler *sampler )
        {
            Samplers.push_back( { slot, sampler } );
            return *this;
        }

        UpdateDesc &Sampler( const uint32_t binding, ISampler *sampler )
        {
            Samplers.push_back( { ResourceBindingSlot{ .Binding = binding, .RegisterSpace = RegisterSpace, .Type = DescriptorBufferBindingType::Sampler }, sampler } );
            return *this;
        }
    };

    class IResourceBindGroup
    {
    protected:
        ResourceBindGroupDesc m_desc;
        IRootSignature       *m_rootSignature;

    public:
        explicit IResourceBindGroup( const ResourceBindGroupDesc &desc ) : m_desc( desc ), m_rootSignature( desc.RootSignature )
        {
        }

        [[nodiscard]] uint32_t RegisterSpace( ) const
        {
            return m_desc.RegisterSpace;
        }

        virtual ~    IResourceBindGroup( )                            = default;
        virtual void SetRootConstants( uint32_t binding, void *data ) = 0;
        virtual void Update( const UpdateDesc &desc );

    protected:
        virtual void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) = 0;
        virtual void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )   = 0;
        virtual void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )          = 0;
    };

    inline void IResourceBindGroup::Update( const UpdateDesc &desc )
    {
        std::vector<ResourceBindingSlot> boundBindings;

        for ( auto item : desc.Buffers )
        {
            BindBuffer( item.Slot, item.Resource );
#ifndef NDEBUG
            boundBindings.push_back( item.Slot );
#endif
        }
        for ( auto item : desc.Textures )
        {
            BindTexture( item.Slot, item.Resource );
#ifndef NDEBUG
            boundBindings.push_back( item.Slot );
#endif
        }
        for ( auto item : desc.Samplers )
        {
            BindSampler( item.Slot, item.Resource );
#ifndef NDEBUG
            boundBindings.push_back( item.Slot );
#endif
        }

#ifndef NDEBUG
        for ( const auto &binding : m_rootSignature->Bindings( ) )
        {
            if ( !ContainerUtilities::Contains( boundBindings, binding ) && binding.RegisterSpace == m_desc.RegisterSpace )
            {
                LOG( ERROR ) << "Binding slot defined in root signature " << binding.ToString( ) << " is not bound.";
            }
        }
#endif
    }
} // namespace DenOfIz
